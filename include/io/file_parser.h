#ifndef FILE_PARSER_H
#define FILE_PARSER_H

#include "core/graph.h"

/*
 * Parses a graph input file and returns a Graph pointer.
 *
 * File format:
 *   Line 1:      N M          (number of vertices, number of edges)
 *   Lines 2..M+1: src dst weight  (directed edge from src to dst)
 *   Last line:   src dst      (Dijkstra query: source and destination)
 *
 * On success: returns the graph and writes the query into *src_out and *dst_out.
 * On failure (bad file, invalid input, negative weights): prints an error
 *             message to stderr and returns NULL.
 */
Graph* parseGraph(const char* filename, int* src_out, int* dst_out);

#endif // FILE_PARSER_H
