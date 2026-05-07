#include "gui/ui_controls.h"
#include "raylib.h"
#include <math.h>

static void draw_play_icon(Rectangle bounds) {
    Vector2 triangle[3] = {
        { bounds.x + bounds.width * 0.34f, bounds.y + bounds.height * 0.2f },
        { bounds.x + bounds.width * 0.34f, bounds.y + bounds.height * 0.8f },
        { bounds.x + bounds.width * 0.78f, bounds.y + bounds.height * 0.5f }
    };
    DrawTriangle(triangle[0], triangle[1], triangle[2], DARKBLUE);
}

static void draw_stop_icon(Rectangle bounds) {
    Rectangle stopRect = {
        bounds.x + bounds.width * 0.32f,
        bounds.y + bounds.height * 0.24f,
        bounds.width * 0.36f,
        bounds.height * 0.52f
    };
    DrawRectangleRounded(stopRect, 0.2f, 4, DARKBLUE);
}

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
    DrawRectangleRoundedLines(bounds, 0.25f, 8, 1.0f, GRAY);
    if (state->is_playing) {
        draw_stop_icon(bounds);
    } else {
        draw_play_icon(bounds);
    }
}

void draw_arrival_message(AnimState *state) {
    if (state->finished) {
        state->is_playing = false; // עצירה בסיום

        int screenWidth = GetScreenWidth();
        const char* text = "Congratulations! Destination Reached";
        int fontSize = 25;
        int textWidth = MeasureText(text, fontSize);
        int posX = (screenWidth - textWidth) / 2;
        int posY = 40; 

        DrawRectangle(posX - 15, posY - 10, textWidth + 30, fontSize + 20, Fade(SKYBLUE, 0.9f));
        DrawRectangleLines(posX - 15, posY - 10, textWidth + 30, fontSize + 20, BLUE);
        DrawText(text, posX, posY, fontSize, DARKBLUE);
    }
}
void draw_ready_indicator(AnimState* state, NodeLayout* layout) {
    // תנאי המשימה: לא מנגן ועדיין לא סיים
    if (!state->is_playing && !state->finished) {
        
        // מקבלים את המיקום של צומת המקור מתוך ה-Layout
        Vector2 sourcePos = layout->positions[state->current_node];
        
        // ציור פעימה עדינה (Pulse) מסביב לצומת
        float pulse = (sinf(GetTime() * 4.0f) + 1.0f) * 5.0f; 
        
        DrawCircleLinesV(sourcePos, 24.0f + 5.0f + pulse, Fade(GREEN, 0.6f));
        
        // כיתוב קטן מעל הצומת
        DrawText("READY", (int)sourcePos.x - 20, (int)sourcePos.y - 45, 12, DARKGREEN);
    }
}