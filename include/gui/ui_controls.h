#ifndef UI_CONTROLS_H
#define UI_CONTROLS_H

#include "raylib.h"
#include "core/animator.h"
#include "gui/layout.h"

void draw_play_stop_button(AnimState* state, Rectangle bounds);
void draw_arrival_message(AnimState *state);

#endif // UI_CONTROLS_H