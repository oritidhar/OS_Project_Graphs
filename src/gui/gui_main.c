#include <stdio.h>
#include "io/file_parser.h"
#include "core/graph.h"
#include "gui/renderer.h"

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

    runGraphGui(graph, argv[1], src, dst);

    freeGraph(graph);

    return 0;
}