#include <stdio.h>
#include "raylib.h"
#include "io/file_parser.h"
#include "core/graph.h"
#include "core/dijkstra.h"
#include "core/animator.h"
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

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        /* Snapshot state before the draw call (where the button may fire). */
        bool finished_at_frame_start = state.finished;
        bool playing_at_frame_start  = state.is_playing;

        if (state.is_playing && !state.finished) {
            animator_tick(&state, result, dt);
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
