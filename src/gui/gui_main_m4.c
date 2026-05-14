#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#include "raylib.h"
#include "io/file_parser.h"
#include "core/graph.h"
#include "core/dijkstra.h"
#include "core/animator.h"

#include "gui/renderer.h"
#include "gui/layout.h"
#include "gui/draw_entity.h"
#include "gui/ui_controls.h"

#include "core/process_mgr.h"
#include "gui/renderer.h"
#include "gui/layout.h"
#include "gui/draw_entity.h"


#define SCREEN_WIDTH  1100
#define SCREEN_HEIGHT 800


static bool all_travelers_finished(Traveler* travelers, int count) {
    for (int i = 0; i < count; i++) {
        if (!travelers[i].anim.finished) {
            return false;
        }
    }

    return true;
}

static void start_all_travelers(Traveler* travelers, int count) {
    for (int i = 0; i < count; i++) {
        travelers[i].anim.is_playing = true;
    }
}

static void reset_all_travelers(Traveler* travelers, int count) {
    for (int i = 0; i < count; i++) {
        animator_init(&travelers[i].anim, travelers[i].path_result);
        travelers[i].anim.is_playing = true;
    }
}

static void cleanup(
    Graph* graph,
    Traveler* travelers,
    int traveler_count,
    NodeLayout* layout
) {
    if (layout && layout->positions) {
        freeNodeLayout(layout);
    }

    if (travelers) {
        for (int i = 0; i < traveler_count; i++) {
            if (travelers[i].path_result) {
                free_path_result(travelers[i].path_result);
            }
        }

        free(travelers);
    }

    if (graph) {
        freeGraph(graph);
    }
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }


    int src, dst;
    Graph* graph = parseGraph(argv[1], &src, &dst);
    if (!graph) return 1;

    /* Compute the shortest path once; the animator reads it every tick. */
    PathResult* result = dijkstra_compute_path(graph, src, dst);

    AnimState state;
    animator_init(&state, result);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OS Project - Graph Simulation");
    SetTargetFPS(60);

    NodeLayout layout = createCircularLayout(graph->numVertices, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!layout.positions) {
        fprintf(stderr, "Error: failed to allocate GUI layout\n");
        CloseWindow();
        free_path_result(result);
        freeGraph(graph);
        return 1;
    }

    Rectangle buttonBounds = { 30.0f, (float)SCREEN_HEIGHT - 90.0f, 100.0f, 40.0f };

    Traveler* travelers = NULL;
    int traveler_count = 0;

    Graph* graph = parseGraphWithTravelers(argv[1], &travelers, &traveler_count);
    if (!graph) {
        return 1;
    }

    Color colors[] = {
        BLUE,
        RED,
        GREEN,
        PURPLE,
        ORANGE,
        MAROON,
        DARKGREEN,
        SKYBLUE
    };

    int colors_count = (int)(sizeof(colors) / sizeof(colors[0]));

    for (int i = 0; i < traveler_count; i++) {
        travelers[i].path_result = dijkstra_compute_path(
            graph,
            travelers[i].src,
            travelers[i].dst
        );

        if (!travelers[i].path_result) {
            fprintf(
                stderr,
                "Error: no path found for traveler %d (%d -> %d)\n",
                i + 1,
                travelers[i].src,
                travelers[i].dst
            );

            cleanup(graph, travelers, traveler_count, NULL);
            return 1;
        }

        travelers[i].color = colors[i % colors_count];

        animator_init(&travelers[i].anim, travelers[i].path_result);
    }

    spawn_travelers(travelers, traveler_count);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OS Project - Milestone 4");
    SetTargetFPS(60);

    NodeLayout layout = createCircularLayout(
        graph->numVertices,
        SCREEN_WIDTH,
        SCREEN_HEIGHT
    );

    if (!layout.positions) {
        fprintf(stderr, "Error: failed to allocate GUI layout\n");
        wait_for_all_travelers(travelers, traveler_count);
        CloseWindow();
        cleanup(graph, travelers, traveler_count, NULL);
        return 1;
    }

    start_all_travelers(travelers, traveler_count);

    bool children_terminated = false;


    while (!WindowShouldClose()) {
        float dt = GetFrameTime();


        /* Snapshot state before the draw call (where the button may fire). */
        bool finished_at_frame_start = state.finished;
        bool playing_at_frame_start  = state.is_playing;

        if (state.is_playing && !state.finished) {
            animator_tick(&state, result, dt);

        for (int i = 0; i < traveler_count; i++) {
            if (travelers[i].anim.is_playing && !travelers[i].anim.finished) {
                animator_tick(&travelers[i].anim, travelers[i].path_result, dt);
            }
        }

        if (!children_terminated && all_travelers_finished(travelers, traveler_count)) {
            wait_for_all_travelers(travelers, traveler_count);
            children_terminated = true;
        }

        Rectangle restart_button = { 30, 150, 130, 40 };
        Vector2 mouse = GetMousePosition();

        if (
            CheckCollisionPointRec(mouse, restart_button) &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)
        ) {
            if (children_terminated) {
                spawn_travelers(travelers, traveler_count);
                children_terminated = false;
            }

            reset_all_travelers(travelers, traveler_count);

        }

        BeginDrawing();
        ClearBackground((Color){ 248, 250, 252, 255 });


        draw_static_graph(graph, &layout, argv[1], src, dst);

        if (result) {
            draw_path_highlight(result->path, result->path_len, graph, &layout);
        }

        draw_entity(&state, layout.positions,RED);
        draw_ready_indicator(&state, &layout);
        draw_play_stop_button(&state, buttonBounds);
        draw_arrival_message(&state);

        EndDrawing();

        /* If play was just pressed while the animation had already finished,
         * restart from the beginning. */
        if (state.is_playing && !playing_at_frame_start && finished_at_frame_start) {
            animator_reset(&state, result);
            state.is_playing = true;
        }
    }

    freeNodeLayout(&layout);
    CloseWindow();
    free_path_result(result);
    freeGraph(graph);

    return 0;
}

        draw_static_graph(graph, &layout, argv[1], -1, -1);

        for (int i = 0; i < traveler_count; i++) {
            draw_path_highlight(
                travelers[i].path_result->path,
                travelers[i].path_result->path_len,
                graph,
                &layout
            );
        }

        for (int i = 0; i < traveler_count; i++) {
            draw_entity(&travelers[i].anim, layout.positions);
        }

        if (all_travelers_finished(travelers, traveler_count)) {
            DrawText("All travelers arrived", 30, 115, 28, DARKGREEN);
        } else {
            DrawText("Travelers are moving...", 30, 115, 24, DARKGRAY);
        }

        DrawRectangleRec(restart_button, LIGHTGRAY);
        DrawRectangleLinesEx(restart_button, 2, DARKGRAY);
        DrawText("Restart", 55, 160, 20, DARKGRAY);

        EndDrawing();
    }

    if (!children_terminated) {
        wait_for_all_travelers(travelers, traveler_count);
    }

    CloseWindow();

    cleanup(graph, travelers, traveler_count, &layout);

    return 0;
}


