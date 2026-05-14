#ifndef UI_CONTROLS_H
#define UI_CONTROLS_H

#include "raylib.h"
#include "core/animator.h"
#include "gui/layout.h"
#include "core/process_mgr.h"

void draw_play_stop_button(AnimState* state, Rectangle bounds);
void draw_arrival_message(AnimState* state);
void draw_ready_indicator(AnimState* state, NodeLayout* layout);
void draw_traveler_legend(Traveler travel[], int n);

#endif // UI_CONTROLS_H