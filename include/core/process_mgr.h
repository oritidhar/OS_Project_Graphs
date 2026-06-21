/*
 * process_mgr.h — fork/wait management of traveler child processes.
 *
 * Milestone 4: the parent computes every path and the children only sleep.
 * Milestone 5+: children compute their own paths and stream position updates
 * back to the parent over per-child pipes (see ipc.h).
 */

#ifndef PROCESS_MGR_H
#define PROCESS_MGR_H

#include "core/traveler.h"
#include "core/graph.h"

/* M4 parent computes the paths, children just sleep */
void spawn_travelers(Traveler* travelers, int n);
void wait_for_all_travelers(Traveler* travelers, int n);

/* M5 children compute their own paths and send IPC updates */
void spawn_travelers_ipc(Traveler* travelers, int n, int (*pipe_fds)[2], Graph* graph);

#endif // PROCESS_MGR_H