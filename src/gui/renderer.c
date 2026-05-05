#include <stdio.h>
#include "raylib.h"
#include "core/graph.h"
#include "gui/layout.h"
#include "gui/arrow_utils.h"
#include "gui/renderer.h"
#include "gui/ui_controls.h"

#define SCREEN_WIDTH 1100
#define SCREEN_HEIGHT 800
#define NODE_RADIUS 24.0f
#define MAX_GUI_VERTICES 15

static void drawCenteredText(const char* text, int centerX, int centerY, int fontSize, Color color) {
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, centerX - textWidth / 2, centerY - fontSize / 2, fontSize, color);
}

static void drawWeightLabel(int weight, Vector2 position) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%d", weight);

    int fontSize = 18;
    int width = MeasureText(buffer, fontSize);
    int padding = 5;

    Rectangle labelBox = {
        position.x - width / 2.0f - padding,
        position.y - fontSize / 2.0f - padding,
        width + padding * 2.0f,
        fontSize + padding * 2.0f
    };

    DrawRectangleRounded(labelBox, 0.35f, 8, RAYWHITE);
    DrawRectangleRoundedLines(labelBox, 0.35f, 8, 1.0f, LIGHTGRAY);
    DrawText(
        buffer,
        (int)(position.x - width / 2.0f),
        (int)(position.y - fontSize / 2.0f),
        fontSize,
        DARKGRAY
    );
}

static void drawSelfLoop(Vector2 center, int weight) {
    Rectangle loopRect = {
        center.x - NODE_RADIUS,
        center.y - NODE_RADIUS - 48.0f,
        NODE_RADIUS * 2.0f,
        NODE_RADIUS * 2.0f
    };

    DrawEllipseLines(
        (int)(loopRect.x + loopRect.width / 2.0f),
        (int)(loopRect.y + loopRect.height / 2.0f),
        loopRect.width / 2.0f,
        loopRect.height / 2.0f,
        DARKGRAY
    );

    Vector2 arrowTip = {
        center.x + NODE_RADIUS * 0.75f,
        center.y - NODE_RADIUS - 18.0f
    };

    DrawTriangle(
        arrowTip,
        (Vector2){ arrowTip.x - 12.0f, arrowTip.y - 5.0f },
        (Vector2){ arrowTip.x - 4.0f, arrowTip.y + 11.0f },
        DARKGRAY
    );

    drawWeightLabel(weight, (Vector2){ center.x, center.y - NODE_RADIUS - 62.0f });
}

static void drawEdges(Graph* graph, NodeLayout* layout) {
    for (int src = 0; src < graph->numVertices; src++) {
        Edge* edge = graph->adjList[src];

        while (edge != NULL) {
            int dst = edge->target;

            if (src == dst) {
                drawSelfLoop(layout->positions[src], edge->weight);
                edge = edge->next;
                continue;
            }

            Vector2 from = layout->positions[src];
            Vector2 to = layout->positions[dst];

            drawArrow(from, to, NODE_RADIUS, DARKGRAY);

            Vector2 labelPosition = getEdgeLabelPosition(from, to, 18.0f);
            drawWeightLabel(edge->weight, labelPosition);

            edge = edge->next;
        }
    }
}

static void drawNodes(Graph* graph, NodeLayout* layout) {
    for (int i = 0; i < graph->numVertices; i++) {
        Vector2 position = layout->positions[i];

        char label[16];
        snprintf(label, sizeof(label), "%d", i);

        DrawCircleV(position, NODE_RADIUS + 3.0f, Fade(SKYBLUE, 0.45f));
        DrawCircleV(position, NODE_RADIUS, RAYWHITE);
        DrawCircleLines((int)position.x, (int)position.y, NODE_RADIUS, BLUE);

        drawCenteredText(label, (int)position.x, (int)position.y, 22, DARKBLUE);
    }
}

static void drawHeader(const char* sourceFileName, int querySrc, int queryDst, int vertexCount) {
    DrawText("Directed Weighted Graph", 30, 24, 28, DARKBLUE);

    char info[512];
    snprintf(
        info,
        sizeof(info),
        "File: %s | Vertices: %d | Dijkstra query: %d -> %d",
        sourceFileName,
        vertexCount,
        querySrc,
        queryDst
    );

    DrawText(info, 30, 62, 18, DARKGRAY);
    DrawText("ESC / close window to exit", 30, SCREEN_HEIGHT - 34, 18, GRAY);
}

void runGraphGui(Graph* graph, const char* sourceFileName, int querySrc, int queryDst) {
    if (graph == NULL) {
        return;
    }

    if (graph->numVertices > MAX_GUI_VERTICES) {
        fprintf(stderr, "Error: GUI supports up to %d vertices\n", MAX_GUI_VERTICES);
        return;
    }

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OS Project - Graph GUI");
    SetTargetFPS(60);

    NodeLayout layout = createCircularLayout(graph->numVertices, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (layout.positions == NULL) {
        fprintf(stderr, "Error: failed to allocate GUI layout\n");
        CloseWindow();
        return;
    }
    AnimState playStopState = {
        .is_playing = false,
        .waiting = false,
        .finished = false,
        .current_edge_index = 0,
        .current_node = querySrc, // מתחילים בצומת המקור כדי שהסימון יופיע עליו
        .next_node = -1,
        .edge_progress = 0.0f,
        .edge_timer = 0.0f,
        .wait_timer = 0.0f
    };
    Rectangle buttonBounds = { 30.0f, (float)SCREEN_HEIGHT - 90.0f, 100.0f, 40.0f };

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground((Color){ 248, 250, 252, 255 });

        drawHeader(sourceFileName, querySrc, queryDst, graph->numVertices);
        drawEdges(graph, &layout);
        drawNodes(graph, &layout);
        draw_ready_indicator(&playStopState, &layout);
        draw_play_stop_button(&playStopState, buttonBounds);
        draw_arrival_message(&playStopState);

        EndDrawing();
    }

    freeNodeLayout(&layout);
    CloseWindow();
}