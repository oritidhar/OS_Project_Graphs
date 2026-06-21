/*
 * gui_main_static.c — milestone 2 entry point (./sim_static): static viewer.
 *
 * Loads the graph and draws it once per frame with no animation: nodes on a
 * circular layout, directed weighted edges, and the Dijkstra query highlighted.
 * The window stays open until ESC/close — this milestone is display-only.
 */

#include <stdio.h>
#include "raylib.h"
#include "io/file_parser.h"
#include "core/graph.h"
#include "gui/renderer.h"
#include "gui/layout.h"

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

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OS Project - Graph GUI");
    SetTargetFPS(60);

    NodeLayout layout = createCircularLayout(graph->numVertices, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!layout.positions) {
        fprintf(stderr, "Error: failed to allocate GUI layout\n");
        CloseWindow();
        freeGraph(graph);
        return 1;
    }

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground((Color){ 248, 250, 252, 255 });
        draw_static_graph(graph, &layout, argv[1], src, dst);
        EndDrawing();
    }

    freeNodeLayout(&layout);
    CloseWindow();
    freeGraph(graph);

    return 0;
}
