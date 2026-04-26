#include "core/graph.h"
#include <stdlib.h> 
#include <stdio.h>     

// Function to create a graph with a given number of vertices
Graph* createGraph(int vertices) {
    Graph* graph = (Graph*)malloc(sizeof(Graph)); // Allocate memory for the graph structure    
    if(graph == NULL) {
        fprintf(stderr, "Memory allocation failed for graph\n");
        exit(1);
    }
    graph->numVertices = vertices; //reset the number of vertices in the graph
    graph->adjList = (Edge**)malloc(vertices * sizeof(Edge*));
    if (graph->adjList == NULL) {
        fprintf(stderr, "Memory allocation failed for adjacency list\n");
        free(graph); // Free the previously allocated graph structure
        exit(1);
    }
    
    for (int i = 0; i < vertices; i++) {
        graph->adjList[i] = NULL; // Initialize adjacency list for each vertex
    }
    
    return graph;
}
// Function to add an edge to the graph
void addEdge(Graph* graph, int src, int dest, int weight) {
    if(weight < 0){// Check for negative weight (dijkstra's algorithm does not support negative weights)
        fprintf(stderr, "Invalid edge weight. Please enter a non-negative value.\n");
        exit(1);
    }

    Edge* newEdge = (Edge*)malloc(sizeof(Edge)); // Allocate memory for the new edge
    if (newEdge == NULL) {
        fprintf(stderr, "Memory allocation failed for new edge\n");
        exit(1);
    }
    
    newEdge->target = dest; // Set the target vertex
    newEdge->weight = weight; // Set the weight of the edge
    newEdge->next = graph->adjList[src]; // Point to the current head of the adjacency list
    graph->adjList[src] = newEdge; // Update the head of the adjacency list to the new edge
}

// Function to free the memory allocated for the graph
void freeGraph(Graph* graph) {
    if (graph == NULL) {
        return; // If the graph is NULL, there's nothing to free
    }
    
    for (int i = 0; i < graph->numVertices; i++) {
        Edge* current = graph->adjList[i];
        while (current != NULL) {
            Edge* temp = current; // Store the current edge
            current = current->next; // Move to the next edge
            free(temp); // Free the current edge
        }
    }
    
    free(graph->adjList); // Free the adjacency list array
    free(graph); // Free the graph structure itself
}