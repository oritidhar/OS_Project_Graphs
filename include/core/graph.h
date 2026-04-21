#ifndef GRAPH_H
#define GRAPH_H

//Represent an edge in the graph as a linked list
typedef struct Edge {
    int target; // vertex to which the edge is connected
    int weight; // weight of the edge
    struct Edge* next; // pointer to the next edge in the adjacency list
    
} Edge;

//represent the graph 
typedef struct Graph {
    int numVertices; // number of vertices in the graph
    Edge** adjList; // adjacency list representation of the graph
} Graph;

Graph* createGraph(int vertices);
void addEdge(Graph* graph, int src, int dest, int weight);
void freeGraph(Graph* graph);

#endif // GRAPH_H