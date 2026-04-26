#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "graph.h"

/*
 * Run Dijkstra's algorithm on the given graph from start to end.
 * The function prints:
 * - the shortest path and total weight
 * - or "No path found" if the destination is unreachable
 * - or the special case output when start == end
 */
void dijkstra(Graph* graph, int start, int end);

#endif