#include "gui/draw_entity.h"

#include <math.h>
#include <stdio.h>

void draw_entity(const Traveler* traveler, const Vector2* nodePos)
{
    if (traveler->anim.finished) {
        return;
    }

    int current_node = traveler->anim.current_node;
    int next_node    = traveler->anim.next_node;
    float progress   = traveler->anim.edge_progress;

    Vector2 start = nodePos[current_node];
    Vector2 end   = nodePos[next_node];

    Vector2 pos;
    pos.x = start.x + progress * (end.x - start.x);
    pos.y = start.y + progress * (end.y - start.y);

    DrawCircleV(pos, 12.0f, traveler->color);
    DrawCircleLinesV(pos, 12.0f, BLACK);
}

void draw_all_travelers(const Traveler* travelers, int count, const Vector2* nodePos)
{
    for (int i = 0; i < count; i++) {
        draw_entity(&travelers[i], nodePos);
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