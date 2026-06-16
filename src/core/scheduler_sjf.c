/*
 * scheduler_sjf.c — Shortest Job First scheduler
 *
 * Each node has an independent sorted linked list of waiting travelers.
 * Travelers are ordered by path_remaining ascending, so the one with the
 * fewest hops left is always at the head and gets the node first.
 * Ties are broken by arrival order (FIFO) because equal-priority nodes
 * are inserted after existing ones (<=, not <, in the walk condition).
 */

#include <stdlib.h>
#include "core/scheduler.h"

#define SJF_MAX_NODES 1024

/* One entry in the per-node sorted wait list. */
typedef struct SJFQueueNode {
    TravelerInfo traveler;
    int path_remaining;       /* hops left at the time of enqueue; sort key */
    struct SJFQueueNode* next;
} SJFQueueNode;

/* One sorted list per node; NULL means no one is waiting. */
static SJFQueueNode* queue_heads[SJF_MAX_NODES];

static int is_valid_node(int node_id) {
    return node_id >= 0 && node_id < SJF_MAX_NODES;
}

/* Free all queued nodes and reset every list to empty. */
void sjf_init(void) {
    for (int i = 0; i < SJF_MAX_NODES; i++) {
        SJFQueueNode* current = queue_heads[i];
        while (current != NULL) {
            SJFQueueNode* next = current->next;
            free(current);
            current = next;
        }

        queue_heads[i] = NULL;
    }
}

/*
 * Insert traveler t into node_id's wait list, sorted by path_remaining.
 * The list stays sorted after every insertion so sjf_next() is O(1).
 */
void sjf_enqueue(int node_id, TravelerInfo t, int path_remaining) {
    if (!is_valid_node(node_id))
    {
        return;
    }

    SJFQueueNode* node = malloc(sizeof(*node));
    if (node == NULL)
    {
        return;
    }

    node->traveler = t;
    node->path_remaining = path_remaining;
    node->next = NULL;

    /* Fast path: insert at head when list is empty or new node is shortest. */
    if (queue_heads[node_id] == NULL || node->path_remaining < queue_heads[node_id]->path_remaining) {
        node->next = queue_heads[node_id];
        queue_heads[node_id] = node;
        return;
    }

    /* Walk until the next node is strictly longer — insert here to preserve
       arrival order among travelers with equal path_remaining. */
    SJFQueueNode* current = queue_heads[node_id];
    while (current->next != NULL && current->next->path_remaining <= node->path_remaining) {
        current = current->next;
    }

    node->next = current->next;
    current->next = node;
}

/*
 * Remove and return the PID of the shortest-job traveler waiting at node_id.
 * Returns -1 if no traveler is queued.
 */
pid_t sjf_next(int node_id) {
    if (!is_valid_node(node_id) || queue_heads[node_id] == NULL) {
        return -1;
    }

    SJFQueueNode* node = queue_heads[node_id];
    pid_t pid = node->traveler.pid;
    queue_heads[node_id] = node->next;
    free(node);
    return pid;
}