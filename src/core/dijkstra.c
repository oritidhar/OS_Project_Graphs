#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "core/dijkstra.h"
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

    MinHeap* minHeap = createMinHeap(V);

    for (int v = 0; v < V; v++) {
        distance[v] = INT_MAX;
        prev[v] = -1;

        minHeap->array[v] = (MinHeapNode*)malloc(sizeof(MinHeapNode));
        if (minHeap->array[v] == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            freeMinHeapInternal(minHeap);
            return;
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

    if (distance[end] == INT_MAX) {
        printf("No path found\n");
        freeMinHeapInternal(minHeap);
        return;
    }

    buildPath(end, prev, path, &pathLen);
    printPathFormatted(path, pathLen);
    printf("%d\n", distance[end]);

    freeMinHeapInternal(minHeap);
}