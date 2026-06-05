#ifndef SYNC_H
#define SYNC_H

#include <stdbool.h>

int  sync_init(int node_count);
void sync_cleanup(void);

bool node_try_lock(int node_id);
void node_lock(int node_id);
void node_unlock(int node_id);

#endif