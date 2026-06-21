/*
 * animator.h — per-traveler animation state for GUI milestones.
 *
 * PathResult holds the computed shortest path.  AnimState drives the
 * frame-by-frame movement of one traveler along that path: it slides the
 * traveler along each edge for (weight × ANIMATOR_EDGE_STEP_TIME) seconds,
 * then pauses for ANIMATOR_NODE_WAIT_TIME seconds at every intermediate node.
 *
 * For multi-process milestones the same AnimState is updated by the parent
 * based on IPC messages instead of animator_tick().
 */

#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <stdbool.h>
#include <sys/time.h>

/* Seconds per unit of edge weight while sliding between nodes. */
#define ANIMATOR_EDGE_STEP_TIME 0.3f
/* Seconds a traveler pauses at each intermediate node. */
#define ANIMATOR_NODE_WAIT_TIME 1.0f

/* Shortest-path result returned by dijkstra_compute_path(). */
typedef struct {
    int* path;          /* node IDs in traversal order, length path_len */
    int path_len;
    int* edge_weights;  /* edge_weights[i] = weight of edge path[i]→path[i+1] */
} PathResult;

/*
 * Runtime animation state for one traveler.
 *
 * edge_progress ∈ [0,1] linearly interpolates the traveler's screen position
 * between current_node and next_node.  The GUI reads it every frame.
 *
 * wait_start_time is set (via gettimeofday) when a traveler first sends a
 * waiting_for_node IPC message; the parent uses it to compute waited seconds.
 */
typedef struct {
    bool is_playing;
    bool waiting;         /* pausing at an intermediate node between edges */
    bool finished;
    bool waiting_for_node; /* blocked outside a locked node (M6+) */

    int current_edge_index;
    int blocked_at_node;  /* node this traveler is waiting to enter; -1 if none */
    int current_node;
    int next_node;        /* -1 when at destination */

    float edge_progress;  /* 0.0 (at current_node) → 1.0 (at next_node) */
    float edge_timer;     /* seconds spent on the current edge */
    float wait_timer;     /* seconds spent at the current node pause */
    struct timeval wait_start_time; /* wall-clock time when waiting began */
} AnimState;

/* Initialise state to the first node of result, not playing. */
void animator_init(AnimState* state, PathResult* result);

/* Advance the animation by dt seconds; call once per frame while playing. */
void animator_tick(AnimState* state, PathResult* result, float dt);

/* Reset to the beginning of the path (same as init). */
void animator_reset(AnimState* state, PathResult* result);

#endif
