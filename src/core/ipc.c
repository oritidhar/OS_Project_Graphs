#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include "core/ipc.h"

int ipc_open_pipes(int (*pipe_fds)[2], int n) {
    for (int i = 0; i < n; i++) {
        if (pipe(pipe_fds[i]) < 0) {
            perror("pipe");
            return -1;
        }
    }
    return 0;
}

void ipc_send(int write_fd, IPCMessage* msg) {
    write(write_fd, msg, sizeof(IPCMessage));
}

int ipc_recv(int read_fd, IPCMessage* msg) {
    ssize_t n = read(read_fd, msg, sizeof(IPCMessage));
    if (n == (ssize_t)sizeof(IPCMessage)) return 1;
    if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return 0; // no data yet
    return -1; // error or EOF
}