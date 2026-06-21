/*
 * ui_controls.h — standalone GUI widgets that do not depend on the graph.
 */

#ifndef UI_CONTROLS_H
#define UI_CONTROLS_H

#include "raylib.h"
#include "core/animator.h"
#include "gui/layout.h"

/* Draw a play/stop toggle button inside bounds.
 * Clicking it flips state->is_playing; the icon changes to match. */
void draw_play_stop_button(AnimState* state, Rectangle bounds);

/* Draw a congratulation or "No path found" banner when the animation ends. */
void draw_arrival_message(AnimState* state);

/* Draw a pulsing "READY" indicator at the source node when idle. */
void draw_ready_indicator(AnimState* state, NodeLayout* layout);

#endif // UI_CONTROLS_H
