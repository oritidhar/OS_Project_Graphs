#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "io/file_parser.h"

#define LINE_SIZE 256

/*
 * Return non-zero if the line is empty or begins with a comment marker '#'
 * after skipping leading whitespace.
 */
static int isCommentOrEmpty(const char* line) {
    while (*line == ' ' || *line == '\t' || *line == '\n' || *line == '\r') {
        line++;
    }

    return *line == '#' || *line == '\0';
}

/*
 * Read the next meaningful line from the file.
 * Comment lines and empty lines are skipped.
 * Returns 1 on success and 0 on end-of-file.
 */
static int readNextDataLine(FILE* f, char* buffer, size_t size) {
    while (fgets(buffer, (int)size, f) != NULL) {
        if (!isCommentOrEmpty(buffer)) {
            return 1;
        }
    }
    return 0;
}

/*
 * Parse the graph and the Dijkstra query from a text file.
 * This function validates:
 * - positive number of vertices
 * - non-negative number of edges
 * - vertex indices within range
 * - non-negative edge weights
 * - existence of the final query line
 */
Graph* parseGraph(const char* filename, int* src_out, int* dst_out) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Error: cannot open file '%s'\n", filename);
        return NULL;
    }

    char line[LINE_SIZE];
    int n, m;

    if (!readNextDataLine(f, line, sizeof(line)) ||
        sscanf(line, "%d %d", &n, &m) != 2 ||
        n <= 0 || m < 0) {
        fprintf(stderr, "Error: invalid graph header\n");
        fclose(f);
        return NULL;
    }

    Graph* g = createGraph(n);

    for (int i = 0; i < m; i++) {
        int src, dst, weight;

        if (!readNextDataLine(f, line, sizeof(line)) ||
            sscanf(line, "%d %d %d", &src, &dst, &weight) != 3) {
            fprintf(stderr, "Error: invalid edge on line %d\n", i + 2);
            freeGraph(g);
            fclose(f);
            return NULL;
        }

        if (src < 0 || dst < 0 || src >= n || dst >= n) {
            fprintf(stderr, "Error: node index out of range on line %d\n", i + 2);
            freeGraph(g);
            fclose(f);
            return NULL;
        }

        if (weight < 0) {
            fprintf(stderr, "Error: negative weight is not allowed\n");
            freeGraph(g);
            fclose(f);
            return NULL;
        }

        addEdge(g, src, dst, weight);
    }

    int src, dst;
    if (!readNextDataLine(f, line, sizeof(line)) ||
        sscanf(line, "%d %d", &src, &dst) != 2) {
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

Graph* parseGraphWithTravelers(
    const char* filename,
    Traveler** travelers_out,
    int* traveler_count_out
) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Error: cannot open file '%s'\n", filename);
        return NULL;
    }

    char line[LINE_SIZE];
    int n, m;

    if (!readNextDataLine(f, line, sizeof(line)) ||
        sscanf(line, "%d %d", &n, &m) != 2 ||
        n <= 0 || m < 0) {
        fprintf(stderr, "Error: invalid graph header\n");
        fclose(f);
        return NULL;
    }

    Graph* g = createGraph(n);
    if (!g) {
        fprintf(stderr, "Error: failed to create graph\n");
        fclose(f);
        return NULL;
    }

    for (int i = 0; i < m; i++) {
        int src, dst, weight;

        if (!readNextDataLine(f, line, sizeof(line)) ||
            sscanf(line, "%d %d %d", &src, &dst, &weight) != 3) {
            fprintf(stderr, "Error: invalid edge on line %d\n", i + 2);
            freeGraph(g);
            fclose(f);
            return NULL;
        }

        if (src < 0 || dst < 0 || src >= n || dst >= n) {
            fprintf(stderr, "Error: node index out of range on line %d\n", i + 2);
            freeGraph(g);
            fclose(f);
            return NULL;
        }

        if (weight < 0) {
            fprintf(stderr, "Error: negative weight is not allowed\n");
            freeGraph(g);
            fclose(f);
            return NULL;
        }

        addEdge(g, src, dst, weight);
    }

    int traveler_count;

    if (!readNextDataLine(f, line, sizeof(line)) ||
        sscanf(line, "%d", &traveler_count) != 1 ||
        traveler_count <= 0) {
        fprintf(stderr, "Error: invalid travelers count\n");
        freeGraph(g);
        fclose(f);
        return NULL;
    }

    Traveler* travelers = calloc((size_t)traveler_count, sizeof(Traveler));
    if (!travelers) {
        fprintf(stderr, "Error: failed to allocate travelers\n");
        freeGraph(g);
        fclose(f);
        return NULL;
    }

    for (int i = 0; i < traveler_count; i++) {
        int src, dst;

        if (!readNextDataLine(f, line, sizeof(line)) ||
            sscanf(line, "%d %d", &src, &dst) != 2) {
            fprintf(stderr, "Error: invalid traveler line\n");
            free(travelers);
            freeGraph(g);
            fclose(f);
            return NULL;
        }

        if (src < 0 || dst < 0 || src >= n || dst >= n) {
            fprintf(stderr, "Error: traveler node index out of range\n");
            free(travelers);
            freeGraph(g);
            fclose(f);
            return NULL;
        }

        travelers[i].src = src;
        travelers[i].dst = dst;
        travelers[i].pid = -1;
        travelers[i].path_result = NULL;
        travelers[i].color = WHITE;
    }

    *travelers_out = travelers;
    *traveler_count_out = traveler_count;

    fclose(f);
    return g;
}