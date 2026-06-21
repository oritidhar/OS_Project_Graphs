/*
 * draw_entity.c — per-frame rendering of traveler circles and status overlays.
 *
 * A traveler can be in one of three visual states:
 *   MOVING   — linearly interpolated between current_node and next_node using
 *               edge_progress ∈ [0,1].
 *   WAITING  — blocked outside a locked node; drawn at 82% along the approach
 *               edge with a pulsing yellow ring to signal contention.
 *   FINISHED — not drawn (already at destination).
 *
 * When several travelers wait for the same node, draw_all_travelers() spreads
 * them out perpendicularly (22 px per waiter) so they do not overlap.
 */

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
        /* Place the circle 82% of the way toward the blocked node so it is
         * visually "just outside" the node without overlapping its circle. */
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
        /* Linearly interpolate between current and next node. */
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

        /* Count how many earlier travelers are already waiting at the same node
         * so we can offset this one perpendicularly to avoid overlap. */
        int waiter_idx = 0;
        for (int j = 0; j < i; j++) {
            if (travelers[j].anim.waiting_for_node &&
                travelers[j].anim.blocked_at_node == blocked)
                waiter_idx++;
        }

        /* Base position: 82% along the approach edge. */
        Vector2 start = nodePos[travelers[i].anim.current_node];
        Vector2 end   = (blocked >= 0) ? nodePos[blocked] : start;
        Vector2 pos;
        pos.x = start.x + 0.82f * (end.x - start.x);
        pos.y = start.y + 0.82f * (end.y - start.y);

        /* Perpendicular offset so waiters fan out side-by-side. */
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

        /* Only draw once per unique blocked node even if multiple travelers
         * are waiting for it. */
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
