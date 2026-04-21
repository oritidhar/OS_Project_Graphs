#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "minHeap.h"

void dijkstra(Graph* graph, int start, int);
void freeMinHeap(MinHeap* minHeap);

#endif // DIJKSTRA_H