#include <stdio.h>
#include <stdlib.h>
#include "io/file_parser.h"

/*
 * Opens the file and reads:
 *   1. N (vertices) and M (edges) from the first line
 *   2. M directed edges: src dst weight
 *   3. A final query line: src dst
 *
 * Validates that all node indices are in range [0, N-1] and all
 * weights are non-negative. Negative values are treated as invalid input.
 */
Graph* parseGraph(const char* filename, int* src_out, int* dst_out) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Error: cannot open file '%s'\n", filename);
        return NULL;
    }

    int n, m;
    // Read number of vertices and edges
    if (fscanf(f, "%d %d", &n, &m) != 2 || n <= 0 || m < 0) {
        fprintf(stderr, "Error: invalid graph header\n");
        fclose(f);
        return NULL;
    }

    Graph* g = createGraph(n);

    // Read each edge
    for (int i = 0; i < m; i++) {
        int src, dst, weight;
        if (fscanf(f, "%d %d %d", &src, &dst, &weight) != 3) {
            fprintf(stderr, "Error: invalid edge on line %d\n", i + 2);
            freeGraph(g);
            fclose(f);
            return NULL;
        }
        // Validate node indices
        if (src < 0 || dst < 0 || src >= n || dst >= n) {
            fprintf(stderr, "Error: node index out of range on line %d\n", i + 2);
            freeGraph(g);
            fclose(f);
            return NULL;
        }
        // Negative weight check is handled inside addEdge, but we catch it here
        // to avoid calling exit() and allow the caller to clean up
        if (weight < 0) {
            fprintf(stderr, "Error: negative weight is not allowed\n");
            freeGraph(g);
            fclose(f);
            return NULL;
        }
        addEdge(g, src, dst, weight);
    }

    // Read Dijkstra query (source and destination)
    int src, dst;
    if (fscanf(f, "%d %d", &src, &dst) != 2) {
        fprintf(stderr, "Error: missing query line (src dst)\n");
        freeGraph(g);
        fclose(f);
        return NULL;
    }
    if (src < 0 || dst < 0 || src >= n || dst >= n) {
        fprintf(stderr, "Error: query node index out of range\n");
        freeGraph(g);
        fclose(f);
        return NULL;
    }

    *src_out = src;
    *dst_out = dst;

    fclose(f);
    return g;
}
