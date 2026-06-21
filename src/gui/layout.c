/*
 * layout.c — circular screen-position assignment for graph nodes.
 *
 * All nodes are placed on a circle centred slightly below the screen centre
 * (to leave room for the header text).  The first node is placed at the top
 * (angle = -π/2) and the rest follow clock-wise at equal angular intervals.
 *
 * The circle radius is 34% of the smaller screen dimension, which keeps nodes
 * inside the window for any aspect ratio and leaves space for the legend and
 * control buttons.
 */

#include <stdlib.h>
#include <math.h>
#include "gui/layout.h"

#ifndef PI
#define PI 3.14159265358979323846f
#endif

NodeLayout createCircularLayout(int vertexCount, int screenWidth, int screenHeight) {
    NodeLayout layout;
    layout.count     = vertexCount;
    layout.positions = NULL;

    if (vertexCount <= 0) {
        return layout;
    }

    layout.positions = (Vector2*)malloc(sizeof(Vector2) * vertexCount);
    if (layout.positions == NULL) {
        layout.count = 0;
        return layout;
    }

    float centerX = screenWidth  / 2.0f;
    float centerY = screenHeight / 2.0f + 20.0f;  /* shift down for header */

    float minDimension = (screenWidth < screenHeight)
                         ? (float)screenWidth : (float)screenHeight;
    float radius = minDimension * 0.34f;

    if (vertexCount == 1) {
        layout.positions[0] = (Vector2){ centerX, centerY };
        return layout;
    }

    for (int i = 0; i < vertexCount; i++) {
        /* Start at 12 o'clock (-π/2) so node 0 is at the top. */
        float angle = (-PI / 2.0f) + (2.0f * PI * i / vertexCount);
        layout.positions[i].x = centerX + radius * cosf(angle);
        layout.positions[i].y = centerY + radius * sinf(angle);
    }

    return layout;
}

void freeNodeLayout(NodeLayout* layout) {
    if (layout == NULL) {
        return;
    }

    free(layout->positions);
    layout->positions = NULL;
    layout->count     = 0;
}
