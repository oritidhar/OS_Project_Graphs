
#include   <stdlib.h>
#include   <stdio.h>
#include "core/minHeap.h"

// Function to create a min heap with a given capacity
MinHeap* createMinHeap(int capacity) {
    MinHeap* minHeap = (MinHeap*)malloc(sizeof(MinHeap)); // Allocate memory for the min heap structure
    if (minHeap == NULL) {
        fprintf(stderr, "Memory allocation failed for min heap\n");
        exit(1);
    }
    minHeap->size = 0; // Initialize the size of the min heap to 0
    minHeap->capacity = capacity; // Set the capacity of the min heap
    minHeap->pos = (int*)malloc(capacity * sizeof(int)); // Allocate memory for the position array
    if (minHeap->pos == NULL) {
        fprintf(stderr, "Memory allocation failed for position array\n");
        free(minHeap); // Free the previously allocated min heap structure
        exit(1);
    }
    minHeap->array = (MinHeapNode**)malloc(capacity * sizeof(MinHeapNode*)); // Allocate memory for the array of pointers to min heap nodes
    if (minHeap->array == NULL) {
        fprintf(stderr, "Memory allocation failed for min heap node array\n");
        free(minHeap->pos); // Free the previously allocated position array
        free(minHeap); // Free the previously allocated min heap structure
        exit(1);
    }

    return minHeap; // Return the created min heap
}

// Function to swap two min heap nodes
void swapMinHeapNode(MinHeapNode** a, MinHeapNode** b) {
    MinHeapNode* temp = *a;
    *a = *b;
    *b = temp;          
}
// Function to maintain the min heap property by heapifying a subtree rooted with node at index idx
void minHeapify(MinHeap* minHeap, int idx) {
    int smallest = idx; // Initialize smallest as the current index
    int left = 2 * idx + 1; // Calculate the index of the left child
    int right = 2 * idx + 2; // Calculate the index of the right child

    // Check if the left child exists and is smaller than the current smallest
    if (left < minHeap->size && minHeap->array[left]->distance < minHeap->array[smallest]->distance) {
        smallest = left;
    }

    // Check if the right child exists and is smaller than the current smallest
    if (right < minHeap->size && minHeap->array[right]->distance < minHeap->array[smallest]->distance) {
        smallest = right;
    }

    // If the smallest is not the current index, swap and continue heapifying
    if (smallest != idx) {
        MinHeapNode* smallestNode = minHeap->array[smallest];
        MinHeapNode* idxNode = minHeap->array[idx];

        // Swap positions in the position array
        minHeap->pos[smallestNode->vertex] = idx;
        minHeap->pos[idxNode->vertex] = smallest;

        // Swap nodes in the array
        swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]);

        // Recursively heapify the affected subtree
        minHeapify(minHeap, smallest);
    }
}

// Function to extract the minimum node from the min heap
MinHeapNode* extractMin(MinHeap* minHeap) {
    if (minHeap->size == 0) {
        return NULL; // If the min heap is empty, return NULL
    }

    MinHeapNode* root = minHeap->array[0]; // Store the minimum node (root of the min heap)

    MinHeapNode* lastNode = minHeap->array[minHeap->size - 1]; // Get the last node in the min heap
    minHeap->array[0] = lastNode; // Move the last node to the root position

    // Update the position of the last node in the position array
    minHeap->pos[root->vertex] = minHeap->size - 1;
    minHeap->pos[lastNode->vertex] = 0;

    --minHeap->size; // Decrease the size of the min heap

    minHeapify(minHeap, 0); // Heapify the root to maintain the min heap property

    return root; // Return the extracted minimum node
}

// Function to decrease the distance value of a given vertex in the min heap
void decreaseKey(MinHeap* minHeap, int vertex, int distance) {
    int i = minHeap->pos[vertex]; // Get the index of the vertex in the min heap
    minHeap->array[i]->distance = distance; // Update the distance value of the vertex 

    // Move the updated vertex up the min heap to maintain the min heap property
    while (i > 0 && minHeap->array[i]->distance < minHeap->array[(i - 1) / 2]->distance) {
        // Swap the current node with its parent
        minHeap->pos[minHeap->array[i]->vertex] = (i - 1) / 2; // Update the position of the current node in the position array
        minHeap->pos[minHeap->array[(i - 1) / 2]->vertex] = i; // Update the position of the parent node in the position array
        swapMinHeapNode(&minHeap->array[i], &minHeap->array[(i - 1) / 2]); // Swap the current node with its parent in the array
        i = (i - 1) / 2; // Move up to the parent index
    }
}    

// Function to check if the min heap is empty
int isEmpty(MinHeap* minHeap) {
    return minHeap->size == 0; // Return true if the size of the min heap is 0, otherwise return false
}   