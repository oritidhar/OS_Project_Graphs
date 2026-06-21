/*
 * graph.h — directed weighted graph as an array of adjacency lists.
 *
 * adjList[v] is a singly linked list of outgoing Edges from vertex v.  The
 * graph is built once from the input file and shared read-only by Dijkstra and
 * the renderer.  Release it with freeGraph().
 */

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