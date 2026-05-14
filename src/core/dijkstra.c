#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "core/dijkstra.h"
#include "core/animator.h"
#include "core/minHeap.h"
#include "core/graph.h"

/*
 * Builds the path by following the predecessor array backward from the
 * destination to the source. The resulting path is stored in reverse order.
 */
static void buildPath(int end, int prev[], int path[], int* pathLen) {
    int current = end;
    *pathLen = 0;
    while (current != -1) {
        path[(*pathLen)++] = current;
        current = prev[current];
    }
}

/*
 * Frees all memory allocated for the MinHeap internal structures.
 */
static void freeMinHeapInternal(MinHeap* minHeap) {
    if (minHeap == NULL) return;
    for (int i = 0; i < minHeap->size; i++) {
        if (minHeap->array[i]) free(minHeap->array[i]);
    }
    free(minHeap->pos);
    free(minHeap->array);
    free(minHeap);
}

/*
 * Core Dijkstra algorithm: calculates distances and predecessors for the graph.
 */
static bool run_dijkstra_core(Graph* graph, int start, int end, int* distance, int* prev) {
    int V = graph->numVertices;
    MinHeap* minHeap = createMinHeap(V);
    if (minHeap == NULL) return false;

    for (int v = 0; v < V; v++) {
        distance[v] = INT_MAX;
        prev[v] = -1;
        minHeap->array[v] = (MinHeapNode*)malloc(sizeof(MinHeapNode));
        if (minHeap->array[v] == NULL) {
            freeMinHeapInternal(minHeap);
            return false;
        }
        minHeap->array[v]->vertex = v;
        minHeap->array[v]->distance = INT_MAX;
        minHeap->pos[v] = v;
    }

    minHeap->size = V;
    distance[start] = 0;
    decreaseKey(minHeap, start, 0);

    while (!isEmpty(minHeap)) {
        MinHeapNode* minNode = extractMin(minHeap);
        if (minNode == NULL) break;

        int u = minNode->vertex;
        free(minNode);

        if (distance[u] == INT_MAX || u == end) break;

        Edge* current = graph->adjList[u];
        while (current != NULL) {
            int v = current->target;
            if (minHeap->pos[v] < minHeap->size &&
                distance[u] != INT_MAX &&
                distance[v] > distance[u] + current->weight) {
                distance[v] = distance[u] + current->weight;
                prev[v] = u;
                decreaseKey(minHeap, v, distance[v]);
            }
            current = current->next;
        }
    }
    freeMinHeapInternal(minHeap);
    return true;
}

/*
 * Main Dijkstra function: computes path, reverses it for the GUI animator, 
 * and allocates memory for the path and edge weights.
 */
PathResult* dijkstra(Graph* graph, int start, int end) {
    if (graph == NULL) return NULL;

    PathResult* res = malloc(sizeof(PathResult));
    if (res == NULL) return NULL;

    // Handle case where source and destination are the same
    if (start == end) {
        res->path_len = 1;
        res->path = malloc(sizeof(int));
        if (res->path) res->path[0] = start;
        res->edge_weights = NULL;
        return res;
    }

    int V = graph->numVertices;
    int distance[V];
    int prev[V];
    int path_buf[V];
    int pathLen = 0;

    if (!run_dijkstra_core(graph, start, end, distance, prev) || distance[end] == INT_MAX) {
        free(res);
        return NULL;
    }

    buildPath(end, prev, path_buf, &pathLen);
    res->path_len = pathLen;
    
    // Allocate memory for path and weights
    res->path = malloc(sizeof(int) * pathLen);
    res->edge_weights = (pathLen > 1) ? malloc(sizeof(int) * (pathLen - 1)) : NULL;

    if (res->path == NULL) {
        free(res);
        return NULL;
    }

    // Reverse path: from (end -> start) to (start -> end)
    for (int i = 0; i < pathLen; i++) {
        res->path[i] = path_buf[pathLen - 1 - i];
    }

    // Fill edge weights (critical for animation speed)
    for (int i = 0; i < pathLen - 1; i++) {
        int u = res->path[i];
        int v = res->path[i + 1];
        res->edge_weights[i] = 1; // Default fallback weight
        Edge* e = graph->adjList[u];
        while (e) {
            if (e->target == v) {
                res->edge_weights[i] = e->weight;
                break;
            }
            e = e->next;
        }
    }

    return res;
}

/*
 * Alias for dijkstra() to maintain compatibility with GUI calls.
 */
PathResult* dijkstra_compute_path(Graph* graph, int start, int end) {
    return dijkstra(graph, start, end);
}

/*
 * Properly frees a PathResult structure and its allocated arrays.
 */
void free_path_result(PathResult* result) {
    if (!result) return;
    if (result->path) free(result->path);
    if (result->edge_weights) free(result->edge_weights);
    free(result);
}