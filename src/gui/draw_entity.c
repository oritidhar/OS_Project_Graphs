#include "gui/draw_entity.h"
#include <math.h>

static Color colors[] = {RED, BLUE, GREEN, ORANGE, VIOLET, GOLD, LIME, SKYBLUE};
static int numOfColors = 8;

//Intialized the travelers color in main
void intialize_travelers_colors(Traveler travel[], int n){
    for(int i = 0; i < n; i++){
        travel[i].color = colors[i % numOfColors];
    }

}

void draw_entity(AnimState* state , Vector2* nodePos, Color baseColor){
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
    
    Color entityColor = state->is_playing? baseColor : ColorBrightness(baseColor, -0.4f); // if move red if wait maroon//if moving use base color if eaiting darken it
    DrawCircleV(currPos,radius, entityColor); 
    DrawCircleLinesV(currPos, radius, BLACK);

}
//Drew each traveler in different color
void draw_all_travelers(Traveler travel[], int n,Vector2* nodePos){
    for(int i = 0; i < n; i++){
        draw_entity(&travel[i].anim,nodePos,travel[i].color);
    }

}