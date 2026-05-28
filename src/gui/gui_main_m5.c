#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "raylib.h"
#include "io/file_parser.h"
#include "core/graph.h"
#include "core/traveler.h"
#include "core/process_mgr.h"
#include "core/ipc.h"
#include "gui/renderer.h"
#include "gui/layout.h"
#include "gui/draw_entity.h"

#define SCREEN_WIDTH        1100
#define SCREEN_HEIGHT       800
#define MAX_TRAVELERS       32
#define IPC_EDGE_ANIMATION_TIME 1.4f

typedef enum {
    SIM_IDLE,     /* waiting for user — button shows "Start"   */
    SIM_RUNNING,  /* children alive  — button shows "Stop"     */
    SIM_STOPPED   /* done or killed  — button shows "Restart"  */
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
    traveler->anim.current_node  = msg->current_node;
    traveler->anim.edge_progress = 0.0f;
    traveler->anim.waiting       = false;
    traveler->anim.is_playing    = true;

    if (msg->finished) {
        traveler->anim.next_node  = msg->current_node;
        traveler->anim.finished   = true;
        traveler->anim.is_playing = false;
        return;
    }

    traveler->anim.finished  = false;
    traveler->anim.next_node = (msg->next_node == -1)
                               ? msg->current_node
                               : msg->next_node;
}

static void poll_ipc_messages(Traveler* travelers, int count, int (*pipe_fds)[2]) {
    for (int i = 0; i < count; i++) {
        IPCMessage msg;
        while (ipc_recv(pipe_fds[i][0], &msg) == 1) {
            int idx = find_traveler_by_pid(travelers, count, msg.pid);
            if (idx >= 0) apply_ipc_message(&travelers[idx], &msg);
            print_ipc_log(&msg);
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
    }
}

/* ── state transitions ──────────────────────────────────────────────────── */

static SimState do_start(Traveler* travelers, int count,
                         int (*pipe_fds)[2], Graph* graph) {
    reset_travelers_anim(travelers, count);
    if (ipc_open_pipes(pipe_fds, count) != 0) return SIM_IDLE;
    spawn_travelers_ipc(travelers, count, pipe_fds, graph);
    return SIM_RUNNING;
}

static SimState do_stop(Traveler* travelers, int count, int (*pipe_fds)[2]) {
    for (int i = 0; i < count; i++)
        if (travelers[i].pid > 0) kill(travelers[i].pid, SIGTERM);
    wait_for_children(travelers, count);
    close_pipe_read_ends(pipe_fds, count);
    return SIM_STOPPED;
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

/* ── main ───────────────────────────────────────────────────────────────── */

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    Traveler* travelers    = NULL;
    int       traveler_count = 0;

    Graph* graph = parseGraphWithTravelers(argv[1], &travelers, &traveler_count);
    if (!graph) return 1;

    if (traveler_count <= 0 || traveler_count > MAX_TRAVELERS) {
        fprintf(stderr, "Error: traveler count must be between 1 and %d\n", MAX_TRAVELERS);
        free(travelers); freeGraph(graph); return 1;
    }

    Color colors[] = { BLUE, RED, GREEN, PURPLE, ORANGE, MAROON, DARKGREEN, SKYBLUE };
    int   colors_count = (int)(sizeof(colors) / sizeof(colors[0]));

    for (int i = 0; i < traveler_count; i++) {
        travelers[i].color       = colors[i % colors_count];
        travelers[i].path_result = NULL;
    }

    int pipe_fds[MAX_TRAVELERS][2];
    for (int i = 0; i < MAX_TRAVELERS; i++) {
        pipe_fds[i][0] = -1;
        pipe_fds[i][1] = -1;
    }

    reset_travelers_anim(travelers, traveler_count);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OS Project - Milestone 5 IPC");
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
                sim_state = SIM_STOPPED;
            }
        }

        /* ── draw + button logic ── */
        BeginDrawing();
        ClearBackground((Color){ 248, 250, 252, 255 });

        draw_static_graph(graph, &layout, argv[1], -1, -1);
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
                if (draw_button(btn, "Stop", RED))
                    sim_state = do_stop(travelers, traveler_count, pipe_fds);
                break;

            case SIM_STOPPED:
                DrawText("All travelers finished", 30, 115, 28, DARKGREEN);
                if (draw_button(btn, "Restart", DARKBLUE))
                    sim_state = do_start(travelers, traveler_count, pipe_fds, graph);
                break;
        }

        EndDrawing();
    }

    /* cleanup */
    if (sim_state == SIM_RUNNING)
        do_stop(travelers, traveler_count, pipe_fds);

    close_pipe_read_ends(pipe_fds, traveler_count);
    freeNodeLayout(&layout);
    CloseWindow();
    free(travelers);
    freeGraph(graph);
    return 0;
}
