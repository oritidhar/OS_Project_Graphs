#ifndef DRAW_ENTITY_H
#define DRAW_ENTITY_H

#include "raylib.h"
#include "../core/animator.h"
#include "core/process_mgr.h"

void intialize_travelers_colors(Traveler travel[], int n);
void draw_entity(AnimState* state, Vector2* nodePos, Color color);
void draw_all_travelers(Traveler[], int n,Vector2* nodePos);

#endif