/*
 * ipc.h — inter-process communication between traveler children and the parent.
 *
 * Each child has one pipe.  The child writes IPCMessage structs into the write
 * end; the parent polls the read ends with ipc_recv() from inside the GUI loop.
 * The read end is set to O_NONBLOCK so the parent never stalls waiting for a
 * single traveler.
 */

#ifndef IPC_H
#define IPC_H

#include <sys/types.h>
#include <stdbool.h>

typedef struct {
    pid_t  pid;
    double arrival_time;   /* gettimeofday timestamp when node was requested */
    int    path_remaining; /* hops left at time of request; used by SJF */
    int    current_node;
    int    next_node;      /* -1 when the traveler has just reached its destination */
    bool   finished;       /* true on the final "I am done" message */
    bool   waiting_for_node; /* true while blocked outside a locked node */
    int    blocked_at_node;  /* the node being waited on; -1 if not waiting */
} IPCMessage;

/* Open one pipe per traveler and set each read end to O_NONBLOCK.
 * pipe_fds[i][0] = read end, pipe_fds[i][1] = write end. */
int  ipc_open_pipes(int (*pipe_fds)[2], int n);

/* Write one complete IPCMessage into write_fd (blocking write). */
void ipc_send(int write_fd, IPCMessage* msg);

/* Non-blocking read of one IPCMessage from read_fd.
 * Returns  1 = message received,
 *          0 = no data yet (EAGAIN/EWOULDBLOCK),
 *         -1 = error or EOF (child exited). */
int  ipc_recv(int read_fd, IPCMessage* msg);

#endif
