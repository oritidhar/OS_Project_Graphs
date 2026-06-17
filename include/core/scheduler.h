#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <sys/types.h>
#include "core/traveler.h"

typedef Traveler TravelerInfo;

typedef enum { FCFS, SJF } SchedulerType;

void scheduler_init(SchedulerType type);
void scheduler_enqueue(int node_id, TravelerInfo t);
pid_t scheduler_next(int node_id);

void fcfs_init(void);
void fcfs_enqueue(int node_id, TravelerInfo t);
pid_t fcfs_next(int node_id);

void sjf_init(void);
void sjf_enqueue(int node_id, TravelerInfo t, int path_remaining);
pid_t sjf_next(int node_id);

#endif // SCHEDULER_H
