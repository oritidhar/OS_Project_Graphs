#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <stdbool.h>

typedef struct {
    int* path;
    int  path_len;
    int* edge_weights;
} PathResult;

typedef struct {
    bool  is_playing;
    int   current_node;
    float edge_progress;  // 0.0 to 1.0 along current edge
    bool  waiting;        // true while paused at a node (1s wait)
    bool  finished;
} AnimState;

void      animator_init(AnimState* state, PathResult* result);
void      animator_tick(AnimState* state, PathResult* result, float dt);
void      animator_reset(AnimState* state, PathResult* result);

#endif
