#ifndef IPC_H
#define IPC_H

#include <sys/types.h>
#include <stdbool.h>

typedef struct {
    pid_t pid;
    double arrival_time;
    int   path_remaining;
    int   current_node;
    int   next_node;    /* -1 when traveler just reached destination */
    bool  finished;     /* true on the final "I'm done" message */
    bool  waiting_for_node;
    int   blocked_at_node;
} IPCMessage;

/* Create one pipe per traveler. pipe_fds[i][0]=read, [i][1]=write */
int  ipc_open_pipes(int (*pipe_fds)[2], int n);

/* Write one IPCMessage into the write end of a pipe */
void ipc_send(int write_fd, IPCMessage* msg);

/*
 * Non-blocking read of one IPCMessage.
 * Returns: 1 = got a message, 0 = no data yet, -1 = error/EOF
 */
int  ipc_recv(int read_fd, IPCMessage* msg);

#endif
