/*
 * gui_main_m4.c — milestone 4 entry point (./sim): multiple travelers.
 *
 * The parent reads the traveler list, computes every traveler's path with
 * Dijkstra, fork()s one child per traveler (children just print "started" and
 * pause()), and animates all travelers concurrently, each in its own colour.
 * When a traveler reaches its destination the parent SIGTERMs that child; it
 * waits for all children before exiting.  Paths still come from the parent —
 * children become autonomous only at M5.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "raylib.h"
#include "io/file_parser.h"
#include "core/graph.h"
#include "core/dijkstra.h"
#include "core/animator.h"
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

        draw_static_graph(graph, &layout, argv[1], -1, -1);

        for (int i = 0; i < traveler_count; i++) {
            draw_path_highlight(
                travelers[i].path_result->path,
                travelers[i].path_result->path_len,
                graph,
                &layout
            );
        }

        draw_all_travelers(travelers, traveler_count, layout.positions);
        draw_travelers_legend(travelers, traveler_count);

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