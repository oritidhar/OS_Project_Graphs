#include "draw_entity.h"
#include <math.h>

void draw_entity(AnimState* state , Vector2* nodePos){
    if(state->finished)return; //we finished -> no printing

    Vector2 currPos; //(x,y) current position
    float radius = 12.0f; //basis radius

    if(state->waiting){
        currPos = nodePos[state->current_node]; //we are on the curr node location
        radius += sinf(GetTime() * 10.0f) * 3.0f; //create a cycle effect
    }else{//didnt finished/waiting
        //calculate our location on the edge
        Vector2 start = nodePos[state->current_node];
        Vector2 end = nodePos[state->next_node];

        //calculate our location between 0.0 - 1.0 in Linear Interpolation technique
        currPos.x = start.x + state->edge_progress * (end.x - start.x);
        currPos.y = start.y + state->edge_progress * (end.y - start.y);
    }
    Color entityColor = state->is_playing? RED : MAROON; // if move red if wait maroon

    DrawCircleV(currPos,radius, RED); //drew in color at th location we calculate
    DrawCircleLinesV(currPos, radius, MAROON);

}