/*
 * gui_main.c — milestone 3 entry point (./sim): single animated traveler.
 *
 * Loads the graph, computes one Dijkstra path, and runs the raylib loop:
 * draws the static graph + highlighted path, animates one traveler along it
 * (300 ms per edge hop, 1 s pause per intermediate node), and exposes a
 * play/stop button.  Single-process — no fork/IPC yet (that starts at M4).
 */

#include <stdio.h>
#include "raylib.h"
#include "io/file_parser.h"
#include "core/graph.h"
#include "core/dijkstra.h"
#include "core/animator.h"
#include "core/traveler.h"
#include "gui/renderer.h"
#include "gui/layout.h"
#include "gui/draw_entity.h"
#include "gui/ui_controls.h"

#define SCREEN_WIDTH  1100
#define SCREEN_HEIGHT 800

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

    Traveler traveler = {0};
    traveler.src   = src;
    traveler.dst   = dst;
    traveler.pid   = -1;
    traveler.color = BLUE;
    traveler.path_result = result;
    animator_init(&traveler.anim, result);

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

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        /* Snapshot state before the draw call (where the button may fire). */
        bool finished_at_frame_start = traveler.anim.finished;
        bool playing_at_frame_start  = traveler.anim.is_playing;

        if (traveler.anim.is_playing && !traveler.anim.finished) {
            animator_tick(&traveler.anim, result, dt);
        }

        BeginDrawing();
        ClearBackground((Color){ 248, 250, 252, 255 });

        draw_static_graph(graph, &layout, argv[1], src, dst);

        if (result) {
            draw_path_highlight(result->path, result->path_len, graph, &layout);
        }

        draw_entity(&traveler, layout.positions);
        draw_ready_indicator(&traveler.anim, &layout);
        draw_play_stop_button(&traveler.anim, buttonBounds);
        draw_arrival_message(&traveler.anim);

        EndDrawing();

        /* If play was just pressed while the animation had already finished,
         * restart from the beginning. */
        if (traveler.anim.is_playing && !playing_at_frame_start && finished_at_frame_start) {
            animator_reset(&traveler.anim, result);
            traveler.anim.is_playing = true;
        }
    }

    freeNodeLayout(&layout);
    CloseWindow();
    free_path_result(result);
    freeGraph(graph);

    return 0;
}
