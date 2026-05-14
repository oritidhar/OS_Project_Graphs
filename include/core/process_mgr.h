#ifndef PROCESS_MGR_H
#define PROCESS_MGR_H

#include <sys/types.h>  //for pid_t
#include "raylib.h" // for Color
#include "core/animator.h" // for AnimState

typedef struct {
    int src, dst;  //start and end nodes for this traveler
    pid_t pid;     //pid of the process that is traveling
    int* path;      //shortest path array (filled before fork)
    int path_len;   //length of the path
    AnimState anim; //animation state for this traveler
    Color color;   //color of the traveler
} Traveler;


void spawn_travelers(Traveler* travelers, int n);
void wait_for_all_travelers(Traveler* travelers, int n);

#endif // PROCESS_MGR_H