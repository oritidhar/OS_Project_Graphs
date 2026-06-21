/*
 * sync.h — per-node mutual exclusion using POSIX named semaphores.
 *
 * Each graph node has a semaphore initialised to 1.  Only one traveler
 * process may occupy a node at a time.  The semaphore names are prefixed
 * with the parent PID so concurrent simulations on the same machine do not
 * collide.
 *
 * Usage pattern (child process):
 *   if (!node_try_lock(node)) {
 *       // tell parent we are waiting, then block
 *       node_lock(node);
 *   }
 *   // ... occupy node ...
 *   node_unlock(node);
 */

#ifndef SYNC_H
#define SYNC_H

#include <stdbool.h>

/* Create one named semaphore per node, each initialised to 1. */
int  sync_init(int node_count);

/* Close and unlink all semaphores created by sync_init(). */
void sync_cleanup(void);

/* Non-blocking acquire: returns true if the lock was taken, false if busy. */
bool node_try_lock(int node_id);

/* Blocking acquire: sleeps until the semaphore becomes available. */
void node_lock(int node_id);

/* Release the lock so the next waiting traveler can enter. */
void node_unlock(int node_id);

#endif
