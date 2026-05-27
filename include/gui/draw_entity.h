#ifndef DRAW_ENTITY_H
#define DRAW_ENTITY_H

#include "raylib.h"
#include "core/animator.h"
#include "core/traveler.h"

void draw_entity(const Traveler* traveler, const Vector2* nodePos);
void draw_all_travelers(const Traveler* travelers, int count,const Vector2* nodePos);
void draw_travelers_legend(Traveler* travelers, int count);

#endif