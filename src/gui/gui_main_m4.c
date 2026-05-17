#include "raylib.h"
#include "core/graph.h"
#include "core/dijkstra.h"
#include "io/file_parser.h"
#include "gui/layout.h"
#include "gui/renderer.h"
#include "gui/draw_entity.h"
#include "core/process_mgr.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 800

// Helper function to check if all travelers have reached their destination
bool all_travelers_finished(Traveler* travelers, int count) {
    if (count <= 0) return false;
    for (int i = 0; i < count; i++) {
        if (!travelers[i].anim.finished) return false;
    }
    return true;
}

// Memory management to free graph, layout, and traveler data
void cleanup(Graph* graph, Traveler* travelers, int count, NodeLayout* layout) {
    if (graph) freeGraph(graph);
    if (layout) freeNodeLayout(layout);
    if (travelers) {
        for (int i = 0; i < count; i++) {
            if (travelers[i].path_result) {
                free_path_result(travelers[i].path_result); 
            }
        }
        free(travelers);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    // Initialize Raylib window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OS Project - Milestone 4 Simulation");
    SetTargetFPS(60);

    // UI elements and colors
    Rectangle restart_button = { 30, 150, 120, 40 };
    Color traveler_colors[] = { RED, BLUE, GREEN, ORANGE, PURPLE, GOLD, MAROON, LIME };

    // Simulation state
    bool simulation_running = false; 
    Traveler* travelers = NULL;
    int traveler_count = 0;

    // Load graph and traveler data from file
    Graph* graph = parseGraphWithTravelers(argv[1], &travelers, &traveler_count);
    if (!graph) {
        CloseWindow();
        return 1;
    }

    // Pre-calculate paths and initialize animators
    for (int i = 0; i < traveler_count; i++) {
        PathResult* res = dijkstra(graph, travelers[i].src, travelers[i].dst);
        travelers[i].path_result = res;
        animator_init(&travelers[i].anim, res);
        travelers[i].color = traveler_colors[i % 8];
    }

    // Prepare graph layout and spawn child processes (OS Logic)
    NodeLayout layout = createCircularLayout(graph->numVertices, SCREEN_WIDTH, SCREEN_HEIGHT);
    spawn_travelers(travelers, traveler_count);

    // Main Game Loop
    while (!WindowShouldClose()) {
        // --- 1. State Check & Input Handling ---
        Vector2 mouse_pos = GetMousePosition();
        bool finished = all_travelers_finished(travelers, traveler_count);

        if (CheckCollisionPointRec(mouse_pos, restart_button)) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (finished) {
                    // RESTART LOGIC: Reset all travelers to the start
                    for (int i = 0; i < traveler_count; i++) {
                        animator_reset(&travelers[i].anim, travelers[i].path_result);
                        travelers[i].anim.is_playing = true;
                    }
                    simulation_running = true;
                } else if (!simulation_running) {
                    // INITIAL START LOGIC
                    simulation_running = true;
                    for (int i = 0; i < traveler_count; i++) {
                        travelers[i].anim.is_playing = true;
                    }
                }
            }
        }

        float dt = GetFrameTime();

        // --- 2. Update Logic ---
        if (simulation_running && !finished) {
            for (int i = 0; i < traveler_count; i++) {
                if (travelers[i].path_result != NULL) {
                    animator_tick(&travelers[i].anim, travelers[i].path_result, dt);
                }
            }
        }

        // --- 3. Rendering ---
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw graph connections and nodes
        draw_static_graph(graph, &layout, argv[1], -1, -1);

        // Draw paths and travelers
        for (int i = 0; i < traveler_count; i++) {
            if (travelers[i].path_result != NULL) {
                draw_path_highlight(
                    travelers[i].path_result->path,
                    travelers[i].path_result->path_len,
                    graph,
                    &layout
                );
                draw_entity(&travelers[i].anim, layout.positions, travelers[i].color);
            }
        }

<<<<<<< Updated upstream
        // Render UI Status Text
        if (finished) {
            DrawText("Status: All travelers arrived", 30, 115, 20, DARKGREEN);
        } else if (simulation_running) {
            DrawText("Status: Simulation running", 30, 115, 20, DARKGRAY);
=======
        for (int i = 0; i < traveler_count; i++) {
            draw_entity(&travelers[i].anim, layout.positions,travelers[i].color );
        }

        if (all_travelers_finished(travelers, traveler_count)) {
            DrawText("All travelers arrived", 30, 115, 28, DARKGREEN);
>>>>>>> Stashed changes
        } else {
            DrawText("Status: Waiting for Play", 30, 115, 20, MAROON);
        }

        // Render Dynamic Button Label
        const char* button_text = "Play";
        if (finished) button_text = "Restart";
        else if (simulation_running) button_text = "Running";

        DrawRectangleRec(restart_button, LIGHTGRAY);
        DrawRectangleLinesEx(restart_button, 2, DARKGRAY);
        
        // Adjust text position slightly if label is "Restart"
        int x_offset = (finished) ? 25 : 35; 
        DrawText(button_text, restart_button.x + x_offset, restart_button.y + 10, 20, BLACK);

        EndDrawing();
    } 

    // --- 4. Cleanup and Exit ---
    wait_for_all_travelers(travelers, traveler_count);
    CloseWindow();
    cleanup(graph, travelers, traveler_count, &layout);
    
    return 0;
}