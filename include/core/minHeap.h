#ifndef MINHEAP_H
#define MINHEAP_H

// Structure to represent a node in the min heap
typedef struct MinHeapNode{
    int vertex; // The vertex number
    int distance; // The distance from the start vertex
} MinHeapNode;  

// Structure to represent the min heap
typedef struct MinHeap{
    int size; // Current size of the min heap
    int capacity; // Maximum capacity of the min heap
    int* pos; // Position of each vertex in the min heap
    MinHeapNode** array; // Array of pointers to min heap nodes
} MinHeap;

MinHeap* createMinHeap(int capacity);
void swapMinHeapNode(MinHeapNode** a, MinHeapNode** b);
void minHeapify(MinHeap* minHeap, int idx);
MinHeapNode* extractMin(MinHeap* minHeap);
void decreaseKey(MinHeap* minHeap, int vertex, int distance);
int isEmpty(MinHeap* minHeap);

#endif // MINHEAP_H