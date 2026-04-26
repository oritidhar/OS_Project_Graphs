#ifndef GUI_ARROW_UTILS_H
#define GUI_ARROW_UTILS_H

#include "raylib.h"

/* Draws a directed edge from start to end, stopping before the target node circle. */
void drawArrow(Vector2 start, Vector2 end, float targetRadius, Color color);

/* Returns a point slightly offset from the edge midpoint, useful for weight labels. */
Vector2 getEdgeLabelPosition(Vector2 start, Vector2 end, float offset);

#endif