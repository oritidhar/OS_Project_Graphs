#ifndef GUI_RENDERER_H
#define GUI_RENDERER_H

#include "core/graph.h"
#include "gui/layout.h"

/* Draw the static graph (header, edges, nodes) inside an active drawing frame. */
void draw_static_graph(Graph* graph, NodeLayout* layout,
                       const char* sourceFileName, int querySrc, int queryDst);

/* Re-draw the edges that belong to the shortest path in a highlight colour. */
void draw_path_highlight(int* path, int path_len, Graph* graph, NodeLayout* layout);

#endif
