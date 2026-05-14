#ifndef TRAVELER_H
#define TRAVELER_H

#include <sys/types.h>
#include "raylib.h"
#include "core/animator.h"

typedef struct {
    int src;
    int dst;

    pid_t pid;

    PathResult* path_result;
    AnimState anim;

    Color color;
} Traveler;

#endif // TRAVELER_H