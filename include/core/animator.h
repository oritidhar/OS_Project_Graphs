#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <stdbool.h>

#define ANIMATOR_EDGE_STEP_TIME 0.3f
#define ANIMATOR_NODE_WAIT_TIME 1.0f

typedef struct {
    int* path;          // Array of node ids in the shortest path
    int path_len;       // Number of nodes in path
    int* edge_weights;  // edge_weights[i] = weight between path[i] and path[i + 1]
} PathResult;

typedef struct {
    bool is_playing;
    bool waiting;
    bool finished;

    int current_edge_index;

    int current_node;
    int next_node;

    float edge_progress; // 0.0 to 1.0
    float edge_timer;
    float wait_timer;
} AnimState;

void animator_init(AnimState* state, PathResult* result);
void animator_tick(AnimState* state, PathResult* result, float dt);
void animator_reset(AnimState* state, PathResult* result);

#endif