/*
 * sync.c — per-node mutual exclusion via POSIX named semaphores.
 *
 * sync_init() creates one named semaphore per graph node, each with an
 * initial value of 1 (binary semaphore = mutex).  Names are built as
 * "/osproj_node_<PID>_<i>" so concurrent simulations on the same machine
 * cannot interfere with each other.
 *
 * Children call node_try_lock() first.  If the node is free they enter
 * immediately; otherwise they send a "waiting" IPC message and call
 * node_lock() to block until the current occupant calls node_unlock().
 *
 * sync_cleanup() is called by the parent after all children have exited to
 * close and unlink every semaphore and release kernel resources.
 */

#include "core/sync.h"

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SEM_NAME_MAX_LEN 64

static sem_t** node_sems      = NULL;
static char**  node_sem_names = NULL;
static int     node_sem_count = 0;

static void fail_and_exit(const char* message) {
    perror(message);
    sync_cleanup();
    exit(EXIT_FAILURE);
}

int sync_init(int node_count) {
    if (node_count <= 0) {
        fprintf(stderr, "sync_init: node_count must be positive\n");
        return -1;
    }

    node_sems      = calloc((size_t)node_count, sizeof(sem_t*));
    node_sem_names = calloc((size_t)node_count, sizeof(char*));

    if (!node_sems || !node_sem_names) {
        perror("calloc");
        sync_cleanup();
        return -1;
    }

    node_sem_count = node_count;

    for (int i = 0; i < node_count; i++) {
        node_sem_names[i] = malloc(SEM_NAME_MAX_LEN);

        if (!node_sem_names[i]) {
            perror("malloc");
            sync_cleanup();
            return -1;
        }

        snprintf(node_sem_names[i], SEM_NAME_MAX_LEN,
                 "/osproj_node_%ld_%d", (long)getpid(), i);

        /* Unlink any leftover semaphore from a previous crashed run. */
        sem_unlink(node_sem_names[i]);

        node_sems[i] = sem_open(node_sem_names[i], O_CREAT | O_EXCL, 0600, 1);

        if (node_sems[i] == SEM_FAILED) {
            perror("sem_open");
            node_sems[i] = NULL;
            sync_cleanup();
            return -1;
        }
    }

    return 0;
}

void sync_cleanup(void) {
    if (node_sems) {
        for (int i = 0; i < node_sem_count; i++) {
            if (node_sems[i]) {
                sem_close(node_sems[i]);
            }
        }
    }

    if (node_sem_names) {
        for (int i = 0; i < node_sem_count; i++) {
            if (node_sem_names[i]) {
                sem_unlink(node_sem_names[i]);
                free(node_sem_names[i]);
            }
        }
    }

    free(node_sems);
    free(node_sem_names);

    node_sems      = NULL;
    node_sem_names = NULL;
    node_sem_count = 0;
}

static void validate_node_id(int node_id) {
    if (node_id < 0 || node_id >= node_sem_count || !node_sems || !node_sems[node_id]) {
        fprintf(stderr, "Invalid node id for semaphore: %d\n", node_id);
        exit(EXIT_FAILURE);
    }
}

/* Attempt to decrement the semaphore without blocking.
 * Returns true if the lock was acquired, false if the node is occupied. */
bool node_try_lock(int node_id) {
    validate_node_id(node_id);

    if (sem_trywait(node_sems[node_id]) == 0) {
        return true;
    }

    if (errno == EAGAIN) {
        return false;
    }

    fail_and_exit("sem_trywait");
    return false;
}

/* Block until the semaphore can be decremented (node becomes free).
 * Retries automatically on EINTR so signals do not cause a spurious failure. */
void node_lock(int node_id) {
    validate_node_id(node_id);

    while (sem_wait(node_sems[node_id]) == -1) {
        if (errno != EINTR) {
            fail_and_exit("sem_wait");
        }
    }
}

/* Increment the semaphore, waking one waiting traveler if any. */
void node_unlock(int node_id) {
    validate_node_id(node_id);

    if (sem_post(node_sems[node_id]) == -1) {
        fail_and_exit("sem_post");
    }
}
