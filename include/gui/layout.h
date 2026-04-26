#ifndef GUI_LAYOUT_H
#define GUI_LAYOUT_H

#include "raylib.h"

/*
 * Stores the screen position of each graph vertex.
 * positions[i] is the position of vertex i.
 */
typedef struct NodeLayout {
    int count;
    Vector2* positions;
} NodeLayout;

/*
 * Creates a simple readable circular layout for up to 15 vertices.
 * The caller must free it using freeNodeLayout().
 */
NodeLayout createCircularLayout(int vertexCount, int screenWidth, int screenHeight);

void freeNodeLayout(NodeLayout* layout);

#endif