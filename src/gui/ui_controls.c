/*
 * ui_controls.c — standalone GUI widgets: play/stop button, arrival banner,
 *                 source-node "READY" pulse indicator.
 *
 * All three functions are stateless except through the AnimState pointer they
 * receive; no global GUI state is held here.
 */

#include "gui/ui_controls.h"
#include "raylib.h"
#include <math.h>

/* Draw a right-pointing triangle (play icon) inside bounds. */
static void draw_play_icon(Rectangle bounds) {
    Vector2 triangle[3] = {
        { bounds.x + bounds.width * 0.34f, bounds.y + bounds.height * 0.2f },
        { bounds.x + bounds.width * 0.34f, bounds.y + bounds.height * 0.8f },
        { bounds.x + bounds.width * 0.78f, bounds.y + bounds.height * 0.5f }
    };
    DrawTriangle(triangle[0], triangle[1], triangle[2], DARKBLUE);
}

/* Draw a filled square (stop icon) inside bounds. */
static void draw_stop_icon(Rectangle bounds) {
    Rectangle stopRect = {
        bounds.x + bounds.width  * 0.32f,
        bounds.y + bounds.height * 0.24f,
        bounds.width  * 0.36f,
        bounds.height * 0.52f
    };
    DrawRectangleRounded(stopRect, 0.2f, 4, DARKBLUE);
}

/* Toggle play/stop on click; show whichever icon matches the current state. */
void draw_play_stop_button(AnimState* state, Rectangle bounds) {
    if (CheckCollisionPointRec(GetMousePosition(), bounds)) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            state->is_playing = !state->is_playing;

            if (state->is_playing) {
                state->finished = false;
            }
        }
    }

    Color fillColor = WHITE;
    if (CheckCollisionPointRec(GetMousePosition(), bounds)) {
        fillColor = (Color){ 240, 240, 240, 255 };
    }

    DrawRectangleRounded(bounds, 0.25f, 8, fillColor);
    DrawRectangleRoundedLines(bounds, 0.35f, 8, LIGHTGRAY);
    if (state->is_playing) {
        draw_stop_icon(bounds);
    } else {
        draw_play_icon(bounds);
    }
}

/* Show a banner when the animation finishes: green on success, red if no path. */
void draw_arrival_message(AnimState* state) {
    if (state->finished && state->current_node != -1) {
        state->is_playing = false;

        int screenWidth = GetScreenWidth();
        const char* text = "Congratulations! Destination Reached";
        int fontSize  = 25;
        int textWidth = MeasureText(text, fontSize);
        int posX = (screenWidth - textWidth) / 2;
        int posY = 40;

        DrawRectangle(posX - 15, posY - 10, textWidth + 30, fontSize + 20, Fade(SKYBLUE, 0.9f));
        DrawRectangleLines(posX - 15, posY - 10, textWidth + 30, fontSize + 20, BLUE);
        DrawText(text, posX, posY, fontSize, DARKBLUE);
    }

    if (state->finished && state->current_node == -1) {
        int screenWidth = GetScreenWidth();
        const char* text = "No path found";
        int fontSize  = 25;
        int textWidth = MeasureText(text, fontSize);
        int posX = (screenWidth - textWidth) / 2;
        int posY = 40;

        DrawRectangle(posX - 15, posY - 10, textWidth + 30, fontSize + 20, Fade(RED, 0.2f));
        DrawRectangleLines(posX - 15, posY - 10, textWidth + 30, fontSize + 20, RED);
        DrawText(text, posX, posY, fontSize, RED);
    }
}

/* Pulsing green ring + "READY" label at the source node while paused. */
void draw_ready_indicator(AnimState* state, NodeLayout* layout) {
    if (!state->is_playing && !state->finished) {
        Vector2 sourcePos = layout->positions[state->current_node];

        float pulse = (sinf(GetTime() * 4.0f) + 1.0f) * 5.0f;
        DrawCircleLinesV(sourcePos, 24.0f + 5.0f + pulse, Fade(GREEN, 0.6f));
        DrawText("READY", (int)sourcePos.x - 20, (int)sourcePos.y - 45, 12, DARKGREEN);
    }
}
