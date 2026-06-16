#include <stdlib.h>
#include "core/scheduler.h"

#define FCFS_MAX_NODES 1024

typedef struct FCFSQueueNode {
    TravelerInfo traveler;
    struct FCFSQueueNode* next;
} FCFSQueueNode;

static FCFSQueueNode* queue_heads[FCFS_MAX_NODES];
static FCFSQueueNode* queue_tails[FCFS_MAX_NODES];

static int is_valid_node(int node_id) {
    return node_id >= 0 && node_id < FCFS_MAX_NODES;
}

void fcfs_init(void) {
    for (int i = 0; i < FCFS_MAX_NODES; i++) {
        FCFSQueueNode* current = queue_heads[i];
        while (current != NULL) {
            FCFSQueueNode* next = current->next;
            free(current);
            current = next;
        }

        queue_heads[i] = NULL;
        queue_tails[i] = NULL;
    }
}

void fcfs_enqueue(int node_id, TravelerInfo t) {
    if (!is_valid_node(node_id)) {
        return;
    }

    FCFSQueueNode* node = malloc(sizeof(*node));
    if (node == NULL) {
        return;
    }

    node->traveler = t;
    node->next = NULL;

    if (queue_tails[node_id] == NULL) {
        queue_heads[node_id] = node;
        queue_tails[node_id] = node;
        return;
    }

    queue_tails[node_id]->next = node;
    queue_tails[node_id] = node;
}

pid_t fcfs_next(int node_id) {
    if (!is_valid_node(node_id) || queue_heads[node_id] == NULL) {
        return -1;
    }

    FCFSQueueNode* node = queue_heads[node_id];
    pid_t pid = node->traveler.pid;

    queue_heads[node_id] = node->next;
    if (queue_heads[node_id] == NULL) {
        queue_tails[node_id] = NULL;
    }

    free(node);
    return pid;
}
