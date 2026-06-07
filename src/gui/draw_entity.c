#include "gui/draw_entity.h"

#include <math.h>
#include <stdio.h>

void draw_entity(const Traveler* traveler, const Vector2* nodePos)
{
    if (traveler->anim.finished) return;

    Vector2 pos;
    bool is_waiting = traveler->anim.waiting_for_node
                      && traveler->anim.blocked_at_node >= 0;

    if (is_waiting) {
        Vector2 start = nodePos[traveler->anim.current_node];
        Vector2 end   = nodePos[traveler->anim.blocked_at_node];
        pos.x = start.x + 0.82f * (end.x - start.x);
        pos.y = start.y + 0.82f * (end.y - start.y);

        float pulse = (float)fabs(sin(GetTime() * 4.0f));
        Color ring  = (Color){ 255, 200, 0, (unsigned char)(160 + 95 * pulse) };

        DrawCircleV(pos, 12.0f, traveler->color);
        DrawCircleLinesV(pos, 16.0f, ring);
        DrawCircleLinesV(pos, 12.0f, DARKGRAY);
    } else {
        float progress = traveler->anim.edge_progress;
        Vector2 start  = nodePos[traveler->anim.current_node];
        Vector2 end    = nodePos[traveler->anim.next_node];
        pos.x = start.x + progress * (end.x - start.x);
        pos.y = start.y + progress * (end.y - start.y);

        DrawCircleV(pos, 12.0f, traveler->color);
        DrawCircleLinesV(pos, 12.0f, BLACK);
    }
}

void draw_all_travelers(const Traveler* travelers, int count, const Vector2* nodePos)
{
    for (int i = 0; i < count; i++) {
        if (travelers[i].anim.finished) continue;

        if (!travelers[i].anim.waiting_for_node) {
            draw_entity(&travelers[i], nodePos);
            continue;
        }

        int blocked = travelers[i].anim.blocked_at_node;

        /* count how many earlier waiters share this blocked node */
        int waiter_idx = 0;
        for (int j = 0; j < i; j++) {
            if (travelers[j].anim.waiting_for_node &&
                travelers[j].anim.blocked_at_node == blocked)
                waiter_idx++;
        }

        /* base position: 82% along edge toward blocked node */
        Vector2 start = nodePos[travelers[i].anim.current_node];
        Vector2 end   = (blocked >= 0) ? nodePos[blocked] : start;
        Vector2 pos;
        pos.x = start.x + 0.82f * (end.x - start.x);
        pos.y = start.y + 0.82f * (end.y - start.y);

        /* perpendicular offset so waiters don't stack on top of each other */
        float dx = end.x - start.x;
        float dy = end.y - start.y;
        float len = sqrtf(dx * dx + dy * dy);
        if (len > 1.0f && waiter_idx > 0) {
            float nx  = -dy / len;
            float ny  =  dx / len;
            float off = waiter_idx * 22.0f;
            pos.x += nx * off;
            pos.y += ny * off;
        }

        float pulse = (float)fabs(sin(GetTime() * 4.0f));
        Color ring  = (Color){ 255, 200, 0, (unsigned char)(160 + 95 * pulse) };

        DrawCircleV(pos, 12.0f, travelers[i].color);
        DrawCircleLinesV(pos, 16.0f, ring);
        DrawCircleLinesV(pos, 12.0f, DARKGRAY);
    }
}

void draw_locked_nodes(const Traveler* travelers, int count, const Vector2* nodePos)
{
    for (int i = 0; i < count; i++) {
        if (!travelers[i].anim.waiting_for_node) continue;

        int blocked = travelers[i].anim.blocked_at_node;
        if (blocked < 0) continue;

        /* draw once per unique blocked node */
        bool already = false;
        for (int j = 0; j < i; j++) {
            if (travelers[j].anim.waiting_for_node &&
                travelers[j].anim.blocked_at_node == blocked) {
                already = true;
                break;
            }
        }
        if (already) continue;

        float pulse = (float)fabs(sin(GetTime() * 3.0f));
        Color ring  = (Color){ 220, 50, 50, (unsigned char)(140 + 110 * pulse) };
        DrawCircleLinesV(nodePos[blocked], 22.0f, ring);
        DrawCircleLinesV(nodePos[blocked], 24.0f, ring);
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
