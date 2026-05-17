#include "gui/draw_entity.h"

#include <math.h>
#include <stdio.h>

void draw_entity(AnimState* state, Vector2* nodePos, Color color)
{
    if (state->finished) {
        return;
    }

    Vector2 currPos;
    float radius = 12.0f;

    if (state->waiting) {
        currPos = nodePos[state->current_node];
        radius += sinf(GetTime() * 10.0f) * 3.0f;
    } else {
        Vector2 start = nodePos[state->current_node];
        Vector2 end = nodePos[state->next_node];

        currPos.x = start.x + state->edge_progress * (end.x - start.x);
        currPos.y = start.y + state->edge_progress * (end.y - start.y);
    }

    DrawCircleV(currPos, radius, color);
    DrawCircleLinesV(currPos, radius, BLACK);
}

void draw_all_travelers(Traveler* travelers, int count, Vector2* nodePos)
{
    for (int i = 0; i < count; i++) {
        draw_entity(&travelers[i].anim, nodePos, travelers[i].color);
    }
}

void draw_travelers_legend(Traveler* travelers, int count)
{
    int x = GetScreenWidth() - 190;
    int y = 80;

    DrawRectangle(x - 12, y - 12, 175, 35 + count * 25, Fade(RAYWHITE, 0.85f));
    DrawRectangleLines(x - 12, y - 12, 175, 35 + count * 25, LIGHTGRAY);

    DrawText("Travelers:", x, y, 20, BLACK);

    for (int i = 0; i < count; i++) {
        int rowY = y + 30 + i * 25;

        DrawCircle(x + 10, rowY + 9, 8, travelers[i].color);

        char label[64];
        snprintf(label, sizeof(label), "Traveler %d", i + 1);

        DrawText(label, x + 30, rowY, 18, BLACK);
    }
}