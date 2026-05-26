#ifndef PROCESS_MGR_H
#define PROCESS_MGR_H

#include "core/traveler.h"

/*M4 parent computes the paths, children just sleep*/
void spawn_travelers(Traveler* travelers, int n);
void wait_for_all_travelers(Traveler* travelers, int n);

/* M5 children compute their own paths and send IPC updates*/
void spawn_travelers_ipc(Traveler* travelers, int n, int (*pipe_fds)[2], struct Graph* graph);

#endif // PROCESS_MGR_H