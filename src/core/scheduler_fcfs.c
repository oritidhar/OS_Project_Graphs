/*
 * scheduler_fcfs.c — First Come First Served scheduler + dispatcher.
 *
 * FCFS per-node queue:
 *   Each node has a singly-linked FIFO list.  fcfs_enqueue() appends to the
 *   tail in O(1); fcfs_next() removes the head in O(1).  Arrival order is
 *   preserved by construction — no comparison needed.
 *
 * Dispatcher (scheduler_* functions):
 *   scheduler_init() selects FCFS or SJF at runtime.  All other scheduler_*
 *   calls delegate to either fcfs_* or sjf_* based on current_scheduler.
 *   The dispatcher also prints scheduler decision lines for runtime tracing.
 */

#include <stdio.h>
#include <stdlib.h>
#include "core/scheduler.h"

#define FCFS_MAX_NODES 1024

/* One entry in the per-node FIFO wait list. */
typedef struct FCFSQueueNode {
    TravelerInfo traveler;
    struct FCFSQueueNode* next;
} FCFSQueueNode;

/* head/tail pointers give O(1) enqueue and dequeue. */
static FCFSQueueNode* queue_heads[FCFS_MAX_NODES];
static FCFSQueueNode* queue_tails[FCFS_MAX_NODES];
static SchedulerType current_scheduler = FCFS;

static int is_valid_node(int node_id) {
    return node_id >= 0 && node_id < FCFS_MAX_NODES;
}

/* Free all nodes and reset both pointers to NULL. */
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

/* Append traveler t to the tail of node_id's queue. */
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

/* Remove the head and return its PID; returns -1 if queue is empty. */
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

/* Count the number of travelers queued for node_id. */
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


void fcfs_describe_waiting(int node_id, char* buffer, int buffer_size) {
    if (buffer == NULL || buffer_size <= 0) {
        return;
    }

    if (!is_valid_node(node_id) || queue_heads[node_id] == NULL) {
        snprintf(buffer, buffer_size, "none");
        return;
    }

    int written = 0;
    for (FCFSQueueNode* node = queue_heads[node_id]; node != NULL && written < buffer_size; node = node->next) {
        int remaining = buffer_size - written;
        int n = snprintf(buffer + written, remaining, "%s%d", written == 0 ? "" : ", ", (int)node->traveler.pid);
        if (n < 0) {
            break;
        }
        if (n >= remaining) {
            buffer[buffer_size - 1] = '\0';
            break;
        }
        written += n;
    }
}
/* Walk the traveler's stored path to find how many hops remain from node_id.
 * Used by scheduler_enqueue() when path_remaining is not in the IPC message. */
static int path_remaining_for_node(const TravelerInfo* traveler, int node_id) {
    if (traveler == NULL || traveler->path_result == NULL || traveler->path_result->path == NULL) {
        return 0;
    }

    for (int i = 0; i < traveler->path_result->path_len; i++) {
        if (traveler->path_result->path[i] == node_id) {
            return traveler->path_result->path_len - i;
        }
    }

    return traveler->path_result->path_len;
}

/* -- dispatcher ---------------------------------------------------------- */

static void describe_current_waiting(int node_id, char* buffer, int buffer_size) {
    if (current_scheduler == SJF) {
        sjf_describe_waiting(node_id, buffer, buffer_size);
    } else {
        fcfs_describe_waiting(node_id, buffer, buffer_size);
    }
}

void scheduler_init(SchedulerType type) {
    current_scheduler = type;
    scheduler_reset();
}

/* Clear all queues (both algorithms) so a restarted simulation starts fresh. */
void scheduler_reset(void) {
    fcfs_init();
    sjf_init();
}

void scheduler_enqueue(int node_id, TravelerInfo t) {
    scheduler_enqueue_with_remaining(node_id, t, path_remaining_for_node(&t, node_id));
}

void scheduler_enqueue_with_remaining(int node_id, TravelerInfo t, int path_remaining) {
    char waiting_after[256];

    printf("[SCHEDULER][%s] Traveler %d is waiting at node %d (remaining path=%d)\n",
           scheduler_get_name(), (int)t.pid, node_id, path_remaining);

    if (current_scheduler == SJF) {
        sjf_enqueue(node_id, t, path_remaining);
    } else {
        fcfs_enqueue(node_id, t);
    }

    describe_current_waiting(node_id, waiting_after, sizeof(waiting_after));
    printf("[SCHEDULER][%s] Traveler %d was added to node %d queue; waiting now: [%s]\n",
           scheduler_get_name(), (int)t.pid, node_id, waiting_after);
    fflush(stdout);
}

pid_t scheduler_next(int node_id) {
    char waiting_before[256];
    char waiting_after[256];
    const char* reason = current_scheduler == SJF
        ? "it has the shortest remaining path"
        : "it arrived first";
    pid_t selected;

    describe_current_waiting(node_id, waiting_before, sizeof(waiting_before));

    if (current_scheduler == SJF) {
        selected = sjf_next(node_id);
    } else {
        selected = fcfs_next(node_id);
    }

    describe_current_waiting(node_id, waiting_after, sizeof(waiting_after));
    if (selected == -1) {
        printf("[SCHEDULER][%s] No traveler selected at node %d; waiting queue was: [%s]\n",
               scheduler_get_name(), node_id, waiting_before);
    } else {
        printf("[SCHEDULER][%s] Selected traveler %d at node %d because %s; waiting before: [%s]; still waiting: [%s]\n",
               scheduler_get_name(), (int)selected, node_id, reason, waiting_before, waiting_after);
    }
    fflush(stdout);
    return selected;
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
