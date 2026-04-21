#include  "graph.h"
#include  "minHeap.h"
#include  "dijkstra.h"
#include  <stdio.h>
#include  <stdlib.h> 
#include <limits.h>   

// Helper function to print the shortest path recursively
static void printPath(int currVer, int prev[]){
    if(currVer == -1){
        return;
    }
    printPath(prev[currVer],prev);

    printf("%d ", currVer);
}
// Function to perform Dijkstra's algorithm on the given graph starting from the specified vertex
void dijkstra(Graph* graph, int start, int end){
    int V = graph->numVertices; // Get total number of vertices
    int distance[V];            // Array to store shortest distances
    int prev[V];                // Array to store shortest path tree

    MinHeap* minHeap = createMinHeap(V); // Create a min-heap for priority queue

    // Initialize min priority heap and arrays
    for(int v = 0; v < V; ++v){
        distance[v] = INT_MAX; // Initialize all distances to INFINITY
        prev[v] = -1;          // No parent initially
        
        // Allocate and initialize heap nodes
        minHeap->array[v] = (MinHeapNode*)malloc(sizeof(MinHeapNode));
        minHeap->array[v]->vertex = v;
        minHeap->array[v]->distance = distance[v];
        minHeap->pos[v] = v;  // Set initial position mapping
    }

    distance[start] = 0; // Distance from start node to itself is 0
    decreaseKey(minHeap, start, distance[start]); // Update start node distance in heap
    minHeap->size = V; // Set initial heap size to total vertices

    // Main Dijkstra loop
    while(!isEmpty(minHeap)){
        MinHeapNode* minHeapNode = extractMin(minHeap); // Get node with minimum distance
        int u = minHeapNode->vertex; // Store vertex number
        free(minHeapNode); // Free the extracted node memory

        // If the smallest distance is infinity, remaining nodes are unreachable
        if(distance[u] == INT_MAX ) break; 

        Edge* temp = graph->adjList[u]; // Traverse all adjacent vertices of u
        while(temp != NULL){
            int v = temp->target; // Get target vertex

            // Relaxation check: if v is in heap & path through u is shorter
            if(minHeap->pos[v] < minHeap->size && distance[u] != INT_MAX && distance[v] > distance[u] + temp->weight){
                distance[v] = distance[u] + temp->weight; // Update shortest distance to v
                prev[v] = u;                              // Set u as parent of v
                decreaseKey(minHeap, v, distance[v]);     // Update distance in the heap
            }
            temp = temp->next; // Move to the next adjacent node
        }
    }

    // Print the final result
    if(distance[end] == INT_MAX){
        printf("No path found from %d to %d\n", start, end); // Unreachable destination
    }else{
        printf("The shortest distance is: %d\n", distance[end]); // Print shortest distance
        printf("Path: ");
        printPath(end, prev);
        printf("\n");
    }

    freeMinHeap(minHeap); // Clean up allocated memory (ensure this is implemented in minHeap.c)
}

// Function to free the memory allocated for the min heap
void freeMinHeap(MinHeap* minHeap){
    if(minHeap == NULL) return;
    //Free the other vertex if Dijkstra stopped early
    for(int i = 0; i < minHeap->size; i++){
        free(minHeap->array[i]);
    }
    free(minHeap->pos);
    free(minHeap->array);
    free(minHeap);
}