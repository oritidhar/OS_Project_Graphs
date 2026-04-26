#include <math.h>
#include "gui/arrow_utils.h"

static Vector2 vectorAdd(Vector2 a, Vector2 b) {
    return (Vector2){ a.x + b.x, a.y + b.y };
}

static Vector2 vectorSubtract(Vector2 a, Vector2 b) {
    return (Vector2){ a.x - b.x, a.y - b.y };
}

static Vector2 vectorScale(Vector2 v, float scale) {
    return (Vector2){ v.x * scale, v.y * scale };
}

static float vectorLength(Vector2 v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}

static Vector2 vectorNormalize(Vector2 v) {
    float length = vectorLength(v);

    if (length == 0.0f) {
        return (Vector2){ 0.0f, 0.0f };
    }

    return (Vector2){ v.x / length, v.y / length };
}

void drawArrow(Vector2 start, Vector2 end, float targetRadius, Color color) {
    Vector2 direction = vectorSubtract(end, start);
    float length = vectorLength(direction);

    if (length < 1.0f) {
        return;
    }

    Vector2 unit = vectorNormalize(direction);

    Vector2 lineStart = vectorAdd(start, vectorScale(unit, targetRadius + 6.0f));
    Vector2 lineEnd = vectorSubtract(end, vectorScale(unit, targetRadius + 12.0f));

    DrawLineEx(lineStart, lineEnd, 2.0f, color);

    float arrowLength = 22.0f;
    float arrowWidth = 12.0f;

    Vector2 perpendicular = (Vector2){ -unit.y, unit.x };

    Vector2 arrowLeft = vectorAdd(
        vectorSubtract(lineEnd, vectorScale(unit, arrowLength)),
        vectorScale(perpendicular, arrowWidth)
    );

    Vector2 arrowRight = vectorSubtract(
        vectorSubtract(lineEnd, vectorScale(unit, arrowLength)),
        vectorScale(perpendicular, arrowWidth)
    );

    DrawLineEx(lineEnd, arrowLeft, 3.0f, color);
    DrawLineEx(lineEnd, arrowRight, 3.0f, color);
}

Vector2 getEdgeLabelPosition(Vector2 start, Vector2 end, float offset) {
    Vector2 mid = (Vector2){
        (start.x + end.x) / 2.0f,
        (start.y + end.y) / 2.0f
    };

    Vector2 direction = vectorSubtract(end, start);
    Vector2 unit = vectorNormalize(direction);
    Vector2 perpendicular = (Vector2){ -unit.y, unit.x };

    return vectorAdd(mid, vectorScale(perpendicular, offset));
}