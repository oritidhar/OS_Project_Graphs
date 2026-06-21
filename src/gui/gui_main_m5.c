/*
 * gui_main_m5.c — shared parent/GUI driver for milestones 5, 6 and 7 (./sim).
 *
 * Children are autonomous (see process_mgr.c): each computes its own path and
 * streams IPCMessages — waiting / entered / released / finished — back over a
 * private pipe.  The parent loop here:
 *   • polls every pipe with ipc_recv() and updates each traveler's animation,
 *   • M6: enforces one-traveler-per-node by granting node entry via SIGUSR1
 *     only when the node is free, and renders waiting travelers distinctly,
 *   • M7: when several travelers contend for a node, picks who to grant next
 *     through the selected scheduler (-schd fcfs|sjf) and shows its name on
 *     screen (scheduler_get_name()).
 * The CLI accepts "-schd fcfs|sjf <input_file>"; an unknown scheduler, missing
 * value, or missing/!openable file is reported to stderr before the window opens.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <string.h>

#include "raylib.h"
#include "io/file_parser.h"
#include "core/dijkstra.h"
#include "core/graph.h"
#include "core/traveler.h"
#include "core/process_mgr.h"
#include "core/ipc.h"
#include "gui/renderer.h"
#include "gui/layout.h"
#include "gui/draw_entity.h"
#include "core/scheduler.h"

#define SCREEN_WIDTH        1100
#define SCREEN_HEIGHT       800
#define MAX_TRAVELERS       32
#define MAX_TRACKED_NODES   1024
#define IPC_EDGE_ANIMATION_TIME 1.4f

static bool node_occupied[MAX_TRACKED_NODES];

typedef enum {
    SIM_IDLE,     /* button: "Start"   */
    SIM_RUNNING,  /* button: "Pause"   */
    SIM_PAUSED,   /* button: "Start"   (resume from where stopped) */
    SIM_FINISHED  /* button: "Restart" (all done, re-fork fresh)   */
} SimState;

/* ── helpers ────────────────────────────────────────────────────────────── */

static int find_traveler_by_pid(Traveler* travelers, int count, pid_t pid) {
    for (int i = 0; i < count; i++)
        if (travelers[i].pid == pid) return i;
    return -1;
}

static bool all_travelers_finished(Traveler* travelers, int count) {
    for (int i = 0; i < count; i++)
        if (!travelers[i].anim.finished) return false;
    return true;
}

static void print_ipc_log(const IPCMessage* msg) {
    if (msg->finished) {
        printf("[PID=%d] finished\n", msg->pid);
    } else if (msg->next_node == -1) {
        printf("[PID=%d] arrived at node %d | DESTINATION\n",
               msg->pid, msg->current_node);
    } else {
        printf("[PID=%d] arrived at node %d | next node: %d\n",
               msg->pid, msg->current_node, msg->next_node);
    }
    fflush(stdout);
}

static void apply_ipc_message(Traveler* traveler, const IPCMessage* msg) {
    //save the prev state of the traveler to see time of changing
    bool was_waiting = traveler->anim.waiting_for_node; 
    int prev_blocked_node = traveler->anim.blocked_at_node;

    if (msg->waiting_for_node) {
        int node = msg->blocked_at_node;

        if (node >= 0 && node < MAX_TRACKED_NODES && !node_occupied[node]) {
            node_occupied[node] = true;
            printf("[SCHED] next node=%d selected pid=%d waiting_count=%d\n",
                   node, (int)msg->pid, scheduler_waiting_count(node));
            fflush(stdout);
            kill(msg->pid, SIGUSR1);
            return;
        }

        gettimeofday(&traveler->anim.wait_start_time, NULL);
        printf("[PID=%d] waiting for node %d\n", msg->pid, node);
        fflush(stdout);
        scheduler_enqueue_with_remaining(node, *traveler, msg->path_remaining);
        traveler->anim.waiting_for_node = true;
        traveler->anim.blocked_at_node = node;
        return;
    }

    if (!msg->finished && msg->blocked_at_node >= 0) {
        int node = msg->blocked_at_node;
        if (node < 0 || node >= MAX_TRACKED_NODES) {
            return;
        }

        node_occupied[node] = false;
        pid_t selected = scheduler_next(node);
        if (selected > 0) {
            node_occupied[node] = true;
            kill(selected, SIGUSR1);
        }
        return;
    }

    if (msg->finished) {
        traveler->anim.next_node = msg->current_node;
        traveler->anim.finished = true;
        traveler->anim.is_playing = false;
        printf("[PID=%d] finished\n", msg->pid);
        fflush(stdout);
        return;
    }

    //the traveler was blocked on this node and now enters it
    if(was_waiting && prev_blocked_node == msg->current_node){
        struct timeval end_time;
        gettimeofday(&end_time, NULL);

        double waiting_time = (end_time.tv_sec - traveler->anim.wait_start_time.tv_sec) + (end_time.tv_usec - traveler->anim.wait_start_time.tv_usec) / 1000000.0;    
        
        printf("[PID=%d] entered node %d | waited %.6f seconds\n", msg->pid, msg->current_node, waiting_time);
        fflush(stdout);
        gettimeofday(&end_time, NULL);
    }
    //normal entry: traveler was not waiting, and this is not the finished sentinel
    if(!was_waiting && !msg->finished){
        printf("[PID=%d] entered node %d\n", msg->pid, msg->current_node);
        fflush(stdout);
    }

    //the traveler enter the node
    traveler->anim.waiting_for_node = false;
    traveler->anim.blocked_at_node = -1;

    //update animation location
    traveler->anim.current_node = msg->current_node;
    traveler->anim.edge_progress = 0.0f;
    traveler->anim.waiting       = false;
    traveler->anim.is_playing    = true;

    traveler->anim.finished = false;
    traveler->anim.next_node = (msg->next_node == -1) ? msg->current_node : msg->next_node;


}

static void poll_ipc_messages(Traveler* travelers, int count, int (*pipe_fds)[2]) {
    for (int i = 0; i < count; i++) {
        IPCMessage msg;
        while (ipc_recv(pipe_fds[i][0], &msg) == 1) {
            int idx = find_traveler_by_pid(travelers, count, msg.pid);
            if (idx >= 0) apply_ipc_message(&travelers[idx], &msg);
        }
    }
}

static void tick_ipc_animation(Traveler* travelers, int count, float dt) {
    for (int i = 0; i < count; i++) {
        AnimState* anim = &travelers[i].anim;
        if (anim->is_playing && !anim->finished &&
            anim->current_node != anim->next_node) {
            anim->edge_progress += dt / IPC_EDGE_ANIMATION_TIME;
            if (anim->edge_progress > 1.0f) anim->edge_progress = 1.0f;
        }
    }
}

static void wait_for_children(Traveler* travelers, int count) {
    for (int i = 0; i < count; i++) {
        if (travelers[i].pid > 0) {
            waitpid(travelers[i].pid, NULL, 0);
            travelers[i].pid = -1;
        }
    }
}

static void close_pipe_read_ends(int (*pipe_fds)[2], int count) {
    for (int i = 0; i < count; i++) {
        if (pipe_fds[i][0] >= 0) {
            close(pipe_fds[i][0]);
            pipe_fds[i][0] = -1;
        }
    }
}

static void reset_travelers_anim(Traveler* travelers, int count) {
    for (int i = 0; i < count; i++) {
        travelers[i].pid                  = -1;
        travelers[i].anim.is_playing      = false;
        travelers[i].anim.waiting         = false;
        travelers[i].anim.finished        = false;
        travelers[i].anim.current_edge_index = 0;
        travelers[i].anim.current_node    = travelers[i].src;
        travelers[i].anim.next_node       = travelers[i].src;
        travelers[i].anim.edge_progress   = 0.0f;
        travelers[i].anim.edge_timer      = 0.0f;
        travelers[i].anim.wait_timer      = 0.0f;
        travelers[i].anim.waiting_for_node    = false;
        travelers[i].anim.blocked_at_node     = -1;
    }
}

/* ── state transitions ──────────────────────────────────────────────────── */

static SimState do_start(Traveler* travelers, int count,
                         int (*pipe_fds)[2], Graph* graph) {
    reset_travelers_anim(travelers, count);
    scheduler_reset();
    memset(node_occupied, 0, sizeof(node_occupied));
    if (ipc_open_pipes(pipe_fds, count) != 0) return SIM_IDLE;
    spawn_travelers_ipc(travelers, count, pipe_fds, graph);
    return SIM_RUNNING;
}

static SimState do_pause(Traveler* travelers, int count) {
    for (int i = 0; i < count; i++)
        if (travelers[i].pid > 0) kill(travelers[i].pid, SIGSTOP);
    return SIM_PAUSED;
}

static SimState do_resume(Traveler* travelers, int count) {
    for (int i = 0; i < count; i++)
        if (travelers[i].pid > 0) kill(travelers[i].pid, SIGCONT);
    return SIM_RUNNING;
}

static void kill_all_children(Traveler* travelers, int count) {
    /* SIGCONT first — SIGSTOP'd processes can't handle SIGTERM until resumed */
    for (int i = 0; i < count; i++)
        if (travelers[i].pid > 0) kill(travelers[i].pid, SIGCONT);
    for (int i = 0; i < count; i++)
        if (travelers[i].pid > 0) kill(travelers[i].pid, SIGTERM);
    wait_for_children(travelers, count);
}

static SimState do_restart(Traveler* travelers, int count,
                           int (*pipe_fds)[2], Graph* graph) {
    /* children already dead when FINISHED — just re-open pipes and re-fork */
    close_pipe_read_ends(pipe_fds, count);
    return do_start(travelers, count, pipe_fds, graph);
}

/* ── button drawing ─────────────────────────────────────────────────────── */

static bool draw_button(Rectangle b, const char* label, Color bg) {
    Vector2 mouse   = GetMousePosition();
    bool    hovered = CheckCollisionPointRec(mouse, b);
    bool    clicked = hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    Color fill = hovered
                 ? (Color){ bg.r, bg.g, bg.b, 200 }
                 : bg;

    DrawRectangleRec(b, fill);
    DrawRectangleLinesEx(b, 1.5f, DARKGRAY);

    int  fs = 18;
    int  tw = MeasureText(label, fs);
    DrawText(label,
             (int)(b.x + (b.width  - tw) / 2),
             (int)(b.y + (b.height - fs) / 2),
             fs, WHITE);
    return clicked;
}

static void draw_scheduler_status(void) {
    char label[64];
    snprintf(label, sizeof(label), "Scheduler: %s", scheduler_get_name());
    DrawText(label, 30, 88, 20, DARKBLUE);
}

static void draw_node_waiting_counts(Graph* graph, const NodeLayout* layout) {
    for (int i = 0; i < graph->numVertices; i++) {
        int node_id = i; /* graph/layout indices are the scheduler's 0-based node IDs */
        int waiting = scheduler_waiting_count(node_id);
        char label[32];
        snprintf(label, sizeof(label), "Waiting: x%d", waiting);
        DrawText(label,
                 (int)layout->positions[node_id].x + 30,
                 (int)layout->positions[node_id].y + 18,
                 14,
                 DARKGRAY);
    }
}

/* ── main ───────────────────────────────────────────────────────────────── */

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s -schd fcfs|sjf <input_file>\n", argv[0]);
        return 1;
    }

    char* input_file = NULL;
    SchedulerType selected_scheduler = FCFS; 

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-schd") == 0) {
            if (i + 1 < argc) {
                if (strcmp(argv[i + 1], "fcfs") == 0) {
                    selected_scheduler = FCFS;
                } else if (strcmp(argv[i + 1], "sjf") == 0) {
                    selected_scheduler = SJF;
                } else {
                    fprintf(stderr, "Error: Unknown scheduler '%s'. Use fcfs or sjf.\n", argv[i + 1]);
                    return 1;
                }
                i++; 
            } else {
                fprintf(stderr, "Error: Missing value for -schd\n");
                return 1;
            }
        } else {
            input_file = argv[i]; 
        }
    }

    if (input_file == NULL) {
        fprintf(stderr, "Error: Missing input file path.\n");
        return 1;
    }

    scheduler_init(selected_scheduler);

    Traveler* travelers    = NULL;
    int       traveler_count = 0;

    Graph* graph = parseGraphWithTravelers(input_file, &travelers, &traveler_count);
    if (!graph) return 1;

    if (traveler_count <= 0 || traveler_count > MAX_TRAVELERS) {
        fprintf(stderr, "Error: traveler count must be between 1 and %d\n", MAX_TRAVELERS);
        free(travelers); freeGraph(graph); return 1;
    }

    Color colors[] = { BLUE, RED, GREEN, PURPLE, ORANGE, MAROON, DARKGREEN, SKYBLUE };
    int   colors_count = (int)(sizeof(colors) / sizeof(colors[0]));

    for (int i = 0; i < traveler_count; i++) {
        travelers[i].color       = colors[i % colors_count];
        travelers[i].path_result = dijkstra_compute_path(graph, travelers[i].src, travelers[i].dst);
    }

    int pipe_fds[MAX_TRAVELERS][2];
    for (int i = 0; i < MAX_TRAVELERS; i++) {
        pipe_fds[i][0] = -1;
        pipe_fds[i][1] = -1;
    }

    reset_travelers_anim(travelers, traveler_count);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OS Project - Milestone 7 Scheduling");
    SetTargetFPS(60);

    NodeLayout layout = createCircularLayout(graph->numVertices,
                                             SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!layout.positions) {
        fprintf(stderr, "Error: failed to allocate GUI layout\n");
        CloseWindow(); free(travelers); freeGraph(graph); return 1;
    }

    SimState sim_state = SIM_IDLE;

    /* single cycling button */
    Rectangle btn = { 30, 150, 120, 36 };

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        /* ── update ── */
        if (sim_state == SIM_RUNNING) {
            poll_ipc_messages(travelers, traveler_count, pipe_fds);
            tick_ipc_animation(travelers, traveler_count, dt);

            if (all_travelers_finished(travelers, traveler_count)) {
                wait_for_children(travelers, traveler_count);
                close_pipe_read_ends(pipe_fds, traveler_count);
                sim_state = SIM_FINISHED;
            }
        }

        /* ── draw + button logic ── */
        BeginDrawing();
        ClearBackground((Color){ 248, 250, 252, 255 });

        draw_static_graph(graph, &layout, input_file, -1, -1);
        draw_scheduler_status();
        draw_node_waiting_counts(graph, &layout);
        draw_locked_nodes(travelers, traveler_count, layout.positions);
        draw_all_travelers(travelers, traveler_count, layout.positions);
        draw_travelers_legend(travelers, traveler_count);

        switch (sim_state) {
            case SIM_IDLE:
                DrawText("Press Start to begin", 30, 115, 22, DARKGRAY);
                if (draw_button(btn, "Start", DARKBLUE))
                    sim_state = do_start(travelers, traveler_count, pipe_fds, graph);
                break;

            case SIM_RUNNING:
                DrawText("Travelers moving...", 30, 115, 22, DARKGRAY);
                if (draw_button(btn, "Pause", ORANGE))
                    sim_state = do_pause(travelers, traveler_count);
                break;

            case SIM_PAUSED:
                DrawText("Paused", 30, 115, 22, ORANGE);
                if (draw_button(btn, "Start", DARKBLUE))
                    sim_state = do_resume(travelers, traveler_count);
                break;

            case SIM_FINISHED:
                DrawText("All travelers finished", 30, 115, 28, DARKGREEN);
                if (draw_button(btn, "Restart", DARKBLUE))
                    sim_state = do_restart(travelers, traveler_count, pipe_fds, graph);
                break;
        }

        EndDrawing();
    }

    /* cleanup */
    if (sim_state == SIM_RUNNING || sim_state == SIM_PAUSED) {
        kill_all_children(travelers, traveler_count);
        close_pipe_read_ends(pipe_fds, traveler_count);
    }

    close_pipe_read_ends(pipe_fds, traveler_count);
    freeNodeLayout(&layout);
    CloseWindow();
    for (int i = 0; i < traveler_count; i++) {
        free_path_result(travelers[i].path_result);
    }
    free(travelers);
    freeGraph(graph);
    return 0;
}
