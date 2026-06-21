/*
 * main.c — milestone 1 command-line entry point (./dijkstra <input_file>).
 *
 * Parses the graph + Dijkstra query from the input file, runs Dijkstra, prints
 * the shortest path and total weight to stdout, then frees the graph.  All
 * input validation and error messages come from parseGraph(); this file only
 * checks the argument count and propagates the failure exit code.
 */

#include <stdio.h>
#include <stdlib.h>
#include "io/file_parser.h"
#include "core/dijkstra.h"
#include "core/graph.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    int src, dst;
    Graph* graph = parseGraph(argv[1], &src, &dst);
    if (graph == NULL) {
        return 1;
    }

    dijkstra(graph, src, dst);
    freeGraph(graph);

    return 0;
}
