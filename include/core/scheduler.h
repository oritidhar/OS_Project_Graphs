/*
 * scheduler.h — pluggable node-entry scheduler for milestone 7.
 *
 * When multiple travelers wait to enter the same node, the parent process
 * uses this interface to decide who goes next.  Two algorithms are provided:
 *   FCFS — First Come First Served: travelers enter in arrival-time order.
 *   SJF  — Shortest Job First: the traveler with the fewest remaining hops
 *           enters first; ties are broken by arrival order.
 *
 * Call scheduler_init() once with the chosen type, then for each node:
 *   scheduler_enqueue_with_remaining() when a traveler starts waiting,
 *   scheduler_next()                   when the node becomes free.
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <sys/types.h>
#include "core/traveler.h"

/* The scheduler stores Traveler structs; PID is the only field it returns. */
typedef Traveler TravelerInfo;

typedef enum { FCFS, SJF } SchedulerType;

/* Initialise the chosen algorithm and clear all per-node queues. */
void scheduler_init(SchedulerType type);

/* Clear all queues without changing the algorithm (used on simulation restart). */
void scheduler_reset(void);

/* Enqueue a traveler; path_remaining is computed from the traveler's path. */
void scheduler_enqueue(int node_id, TravelerInfo t);

/* Enqueue with an explicit path_remaining value (from the IPC message). */
void scheduler_enqueue_with_remaining(int node_id, TravelerInfo t, int path_remaining);

/* Dequeue and return the PID of the next traveler for node_id.
 * Returns -1 if the queue is empty. */
pid_t scheduler_next(int node_id);

/* Return "FCFS" or "SJF" for GUI display. */
const char* scheduler_get_name(void);

/* Return the number of travelers currently queued for node_id. */
int scheduler_waiting_count(int node_id);

/* ── algorithm-specific functions (called through the dispatcher above) ── */

void  fcfs_init(void);
void  fcfs_enqueue(int node_id, TravelerInfo t);
pid_t fcfs_next(int node_id);
int   fcfs_waiting_count(int node_id);

void  sjf_init(void);
void  sjf_enqueue(int node_id, TravelerInfo t, int path_remaining);
pid_t sjf_next(int node_id);
int   sjf_waiting_count(int node_id);

#endif // SCHEDULER_H
