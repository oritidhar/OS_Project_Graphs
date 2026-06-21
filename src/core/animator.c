/*
 * animator.c — frame-by-frame movement of a traveler along a PathResult.
 *
 * The state machine has two phases that alternate until the path is exhausted:
 *   SLIDE  — edge_timer runs up to (weight × ANIMATOR_EDGE_STEP_TIME) seconds,
 *             edge_progress is linearly interpolated and read by the renderer.
 *   WAIT   — wait_timer runs up to ANIMATOR_NODE_WAIT_TIME seconds while the
 *             traveler sits at the newly reached intermediate node.
 *
 * animator_tick() is called once per frame with the elapsed delta time.
 * For multi-process milestones (M5+) the parent updates AnimState directly
 * from IPC messages and does NOT call animator_tick().
 */

#include "core/animator.h"

static bool animator_has_valid_path(PathResult* result) {
    return result != 0 && result->path != 0 && result->path_len > 0;
}

/* Return the edge weight at edge_index, falling back to 1 on bad input. */
static int animator_get_current_edge_weight(PathResult* result, int edge_index) {
    if (result == 0 || result->edge_weights == 0) {
        return 1;
    }

    if (edge_index < 0 || edge_index >= result->path_len - 1) {
        return 1;
    }

    if (result->edge_weights[edge_index] <= 0) {
        return 1;
    }

    return result->edge_weights[edge_index];
}

void animator_init(AnimState* state, PathResult* result) {
    if (state == 0) {
        return;
    }

    state->is_playing = false;
    state->waiting = false;
    state->finished = false;

    state->current_edge_index = 0;

    state->edge_progress = 0.0f;
    state->edge_timer = 0.0f;
    state->wait_timer = 0.0f;

    if (!animator_has_valid_path(result)) {
        state->current_node = -1;
        state->next_node = -1;
        state->finished = true;
        return;
    }

    state->current_node = result->path[0];

    /* Single-node path (src == dst): mark finished immediately. */
    if (result->path_len == 1) {
        state->next_node = -1;
        state->finished = true;
        return;
    }

    state->next_node = result->path[1];
}

void animator_reset(AnimState* state, PathResult* result) {
    animator_init(state, result);
}

void animator_tick(AnimState* state, PathResult* result, float dt) {
    if (state == 0 || !animator_has_valid_path(result)) {
        return;
    }

    if (!state->is_playing || state->finished) {
        return;
    }

    if (dt <= 0.0f) {
        return;
    }

    /* ── WAIT phase ── */
    if (state->waiting) {
        state->wait_timer += dt;

        if (state->wait_timer >= ANIMATOR_NODE_WAIT_TIME) {
            state->waiting = false;
            state->wait_timer = 0.0f;

            state->edge_timer = 0.0f;
            state->edge_progress = 0.0f;

            if (state->current_edge_index < result->path_len - 1) {
                state->current_node = result->path[state->current_edge_index];
                state->next_node = result->path[state->current_edge_index + 1];
            }
        }

        return;
    }

    /* ── SLIDE phase ── */
    if (state->current_edge_index >= result->path_len - 1) {
        state->finished = true;
        state->is_playing = false;
        state->edge_progress = 1.0f;
        return;
    }

    int weight = animator_get_current_edge_weight(result, state->current_edge_index);
    float edge_duration = weight * ANIMATOR_EDGE_STEP_TIME;

    state->edge_timer += dt;
    state->edge_progress = state->edge_timer / edge_duration;

    if (state->edge_progress >= 1.0f) {
        state->edge_progress = 1.0f;

        /* Arrived at the next node — advance the edge index. */
        state->current_node = result->path[state->current_edge_index + 1];
        state->current_edge_index++;

        if (state->current_edge_index >= result->path_len - 1) {
            /* Reached the destination: stop. */
            state->next_node = -1;
            state->finished = true;
            state->is_playing = false;
            return;
        }

        state->next_node = result->path[state->current_edge_index + 1];

        /* Enter the WAIT phase at this intermediate node. */
        state->waiting = true;
        state->wait_timer = 0.0f;
        state->edge_timer = 0.0f;
    }
}
