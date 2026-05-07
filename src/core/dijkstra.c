#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "core/dijkstra.h"
#include "core/animator.h"
#include "core/minHeap.h"
#include "core/graph.h"

/*
 * Build the path by following the predecessor array backward from the
 * destination to the source. The path is stored in reverse order.
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
 * Print the path in the required format:
 * v0 -> v1 -> v2
 */
static void printPathFormatted(int path[], int pathLen) {
    for (int i = pathLen - 1; i >= 0; i--) {
        printf("%d", path[i]);
        if (i > 0) {
            printf(" -> ");
        }
    }
    printf("\n");
}

/*
 * Free all memory owned by the heap.
 * Only active heap entries up to minHeap->size are freed here.
 */
static void freeMinHeapInternal(MinHeap* minHeap) {
    if (minHeap == NULL) {
        return;
    }

    for (int i = 0; i < minHeap->size; i++) {
        free(minHeap->array[i]);
    }

    free(minHeap->pos);
    free(minHeap->array);
    free(minHeap);
}

/*
 * Core Dijkstra algorithm: fills distance[] and prev[] for all vertices.
 * Stops early when the target vertex is extracted from the heap.
 * Returns false if memory allocation fails.
 */
static bool run_dijkstra_core(Graph* graph, int start, int end,
                               int* distance, int* prev) {
    int V = graph->numVertices;
    MinHeap* minHeap = createMinHeap(V);
    if (minHeap == NULL) {
        return false;
    }

    for (int v = 0; v < V; v++) {
        distance[v] = INT_MAX;
        prev[v] = -1;

        minHeap->array[v] = (MinHeapNode*)malloc(sizeof(MinHeapNode));
        if (minHeap->array[v] == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
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
        if (minNode == NULL) {
            break;
        }

        int u = minNode->vertex;
        free(minNode);

        if (distance[u] == INT_MAX) {
            break;
        }

        if (u == end) {
            break;
        }

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
 * Standard Dijkstra implementation using a min-heap priority queue.
 * The function computes shortest distances from 'start' and reconstructs
 * the path to 'end' using the predecessor array.
 */
void dijkstra(Graph* graph, int start, int end) {
    if (graph == NULL) {
        return;
    }

    if (start == end) {
        printf("%d\n", start);
        printf("0\n");
        return;
    }

    int V = graph->numVertices;
    int distance[V];
    int prev[V];
    int path[V];
    int pathLen = 0;

    if (!run_dijkstra_core(graph, start, end, distance, prev)) {
        return;
    }

    if (distance[end] == INT_MAX) {
        printf("No path found\n");
        return;
    }

    buildPath(end, prev, path, &pathLen);
    printPathFormatted(path, pathLen);
    printf("%d\n", distance[end]);
}

/*
 * Run Dijkstra and return the result as a PathResult for GUI animation.
 * Reuses run_dijkstra_core() and buildPath() from the print version.
 */
PathResult* dijkstra_compute_path(Graph* graph, int start, int end) {
    if (graph == NULL) return NULL;

    PathResult* result = malloc(sizeof(PathResult));
    if (!result) return NULL;

    if (start == end) {
        result->path = malloc(sizeof(int));
        if (!result->path) { free(result); return NULL; }
        result->path[0] = start;
        result->path_len = 1;
        result->edge_weights = NULL;
        return result;
    }

    int V = graph->numVertices;
    int distance[V];
    int prev[V];
    int path_buf[V];
    int path_len = 0;

    if (!run_dijkstra_core(graph, start, end, distance, prev)) {
        free(result);
        return NULL;
    }

    if (distance[end] == INT_MAX) {
        free(result);
        return NULL;
    }

    buildPath(end, prev, path_buf, &path_len);

    result->path_len = path_len;
    result->path = malloc(path_len * sizeof(int));
    result->edge_weights = (path_len > 1) ? malloc((path_len - 1) * sizeof(int)) : NULL;

    if (!result->path || (path_len > 1 && !result->edge_weights)) {
        free(result->path);
        free(result->edge_weights);
        free(result);
        return NULL;
    }

    /* buildPath fills path_buf end→start; reverse it to start→end */
    for (int i = 0; i < path_len; i++) {
        result->path[i] = path_buf[path_len - 1 - i];
    }

    /* look up the weight of each edge from the adjacency list */
    for (int i = 0; i < path_len - 1; i++) {
        int from = result->path[i];
        int to   = result->path[i + 1];
        result->edge_weights[i] = 1;
        Edge* e = graph->adjList[from];
        while (e) {
            if (e->target == to) { result->edge_weights[i] = e->weight; break; }
            e = e->next;
        }
    }

    return result;
}

void free_path_result(PathResult* result) {
    if (!result) return;
    free(result->path);
    free(result->edge_weights);
    free(result);
}
