/*
 * traveler.h — data for one traveler process.
 *
 * One Traveler is created per entry in the input file's "travelers" section.
 * The parent process owns the array; each entry is updated from IPC messages
 * sent by the corresponding child process.
 */

#ifndef TRAVELER_H
#define TRAVELER_H

#include <sys/types.h>
#include "raylib.h"
#include "core/animator.h"

typedef struct {
    int src;              /* source node index */
    int dst;              /* destination node index */
    pid_t pid;            /* child PID (-1 before fork) */
    PathResult* path_result; /* computed shortest path; NULL until child sends it */
    AnimState anim;       /* GUI animation state, driven by IPC messages */
    Color color;          /* unique colour assigned before forking */
} Traveler;

#endif // TRAVELER_H
