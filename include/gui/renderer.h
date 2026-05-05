#ifndef GUI_RENDERER_H
#define GUI_RENDERER_H

#include "raylib.h"
#include "core/graph.h"

extern Vector2 nodePositions[];

/* Opens a raylib window and displays the directed weighted graph. */
void runGraphGui(Graph* graph, const char* sourceFileName, int querySrc, int queryDst);

#endif
