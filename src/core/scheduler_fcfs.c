#include <stdlib.h>
#include "core/scheduler.h"

#define FCFS_MAX_NODES 1024

typedef struct FCFSQueueNode {
    TravelerInfo traveler;
    struct FCFSQueueNode* next;
} FCFSQueueNode;

static FCFSQueueNode* queue_heads[FCFS_MAX_NODES];
static FCFSQueueNode* queue_tails[FCFS_MAX_NODES];
static SchedulerType current_scheduler = FCFS;

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

int fcfs_waiting_count(int node_id) {
    if (!is_valid_node(node_id)) {
        return 0;
    }

    int count = 0;
    for (FCFSQueueNode* node = queue_heads[node_id]; node != NULL; node = node->next) {
        count++;
    }
    return count;
}

static int path_remaining_for_node(const TravelerInfo* traveler, int node_id) {
    if (traveler == NULL || traveler->path_result == NULL || traveler->path_result->path == NULL) {
        return 0;
    }

    for (int i = 0; i < traveler->path_result->path_len; i++) {
        if (traveler->path_result->path[i] == node_id) {
            return traveler->path_result->path_len - i - 1;
        }
    }

    return traveler->path_result->path_len;
}

void scheduler_init(SchedulerType type) {
    current_scheduler = type;
    fcfs_init();
    sjf_init();
}

void scheduler_enqueue(int node_id, TravelerInfo t) {
    if (current_scheduler == SJF) {
        sjf_enqueue(node_id, t, path_remaining_for_node(&t, node_id));
        return;
    }

    fcfs_enqueue(node_id, t);
}

pid_t scheduler_next(int node_id) {
    if (current_scheduler == SJF) {
        return sjf_next(node_id);
    }

    return fcfs_next(node_id);
}

const char* scheduler_get_name(void) {
    return current_scheduler == SJF ? "SJF" : "FCFS";
}

int scheduler_waiting_count(int node_id) {
    if (current_scheduler == SJF) {
        return sjf_waiting_count(node_id);
    }

    return fcfs_waiting_count(node_id);
}
