/*
 * dijkstra.h — shortest-path computation over a directed weighted Graph.
 *
 * Two entry points share the same core algorithm:
 *   dijkstra()              prints the path + total weight (milestone 1 CLI).
 *   dijkstra_compute_path() returns a PathResult the GUI animates (milestone 3+).
 * Both treat an unreachable destination and the start == end case explicitly.
 */

#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "graph.h"
#include "animator.h"

/*
 * Run Dijkstra's algorithm on the given graph from start to end.
 * The function prints:
 * - the shortest path and total weight
 * - or "No path found" if the destination is unreachable
 * - or the special case output when start == end
 */
void dijkstra(Graph* graph, int start, int end);

/*
 * Run Dijkstra and return the path as a PathResult for GUI animation.
 * Returns NULL if graph is NULL or no path exists.
 * Caller must free the result with free_path_result().
 */
PathResult* dijkstra_compute_path(Graph* graph, int start, int end);

void free_path_result(PathResult* result);

#endif