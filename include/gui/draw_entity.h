/*
 * draw_entity.h — per-frame rendering of traveler circles and status overlays.
 */

#ifndef DRAW_ENTITY_H
#define DRAW_ENTITY_H

#include "raylib.h"
#include "core/animator.h"
#include "core/traveler.h"

/* Draw a single traveler interpolated between its current and next node.
 * If the traveler is waiting outside a locked node it is drawn at 82% along
 * the approach edge with a pulsing yellow ring. */
void draw_entity(const Traveler* traveler, const Vector2* nodePos);

/* Draw all travelers.  Waiters sharing the same blocked node are spread out
 * perpendicularly so they do not overlap. */
void draw_all_travelers(const Traveler* travelers, int count, const Vector2* nodePos);

/* Draw a pulsing red ring around every node that has at least one waiter. */
void draw_locked_nodes(const Traveler* travelers, int count, const Vector2* nodePos);

/* Draw a colour-keyed legend listing all travelers in the top-right corner. */
void draw_travelers_legend(Traveler* travelers, int count);

#endif
