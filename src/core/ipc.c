/*
 * ipc.c — pipe creation and atomic IPCMessage read/write.
 *
 * Each traveler child writes one IPCMessage at a time.  sizeof(IPCMessage) is
 * well under PIPE_BUF (4096 bytes on Linux), so each write() is guaranteed to
 * be atomic — no partial message will ever be read by the parent.
 *
 * The read ends are set to O_NONBLOCK so the parent's GUI loop can poll all
 * pipes in a single pass without blocking on a slow or idle traveler.
 */

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include "core/ipc.h"

int ipc_open_pipes(int (*pipe_fds)[2], int n) {
    for (int i = 0; i < n; i++) {
        if (pipe(pipe_fds[i]) < 0) {
            perror("pipe");
            return -1;
        }

        /* Set the read end to non-blocking so the parent's poll loop never
         * stalls waiting for a message from one specific traveler. */
        int flags = fcntl(pipe_fds[i][0], F_GETFL, 0);
        if (flags < 0 || fcntl(pipe_fds[i][0], F_SETFL, flags | O_NONBLOCK) < 0) {
            perror("fcntl");
            return -1;
        }
    }

    return 0;
}

/* Atomic write: sizeof(IPCMessage) < PIPE_BUF so this never splits a message. */
void ipc_send(int write_fd, IPCMessage* msg) {
    write(write_fd, msg, sizeof(IPCMessage));
}

int ipc_recv(int read_fd, IPCMessage* msg) {
    ssize_t n = read(read_fd, msg, sizeof(IPCMessage));

    if (n == (ssize_t)sizeof(IPCMessage)) {
        return 1;
    }

    if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        return 0;  /* nothing available right now */
    }

    return -1;  /* EOF (child exited) or real error */
}
