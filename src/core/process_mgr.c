#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "core/process_mgr.h"
#include "core/ipc.h"
#include "core/dijkstra.h"
#include "core/sync.h"

void spawn_travelers(Traveler* travelers, int n) {
        for (int  i = 0; i < n; i++)
        {
            pid_t pid  = fork();

            if (pid < 0) {
                perror("fork failed");
                exit(EXIT_FAILURE);
            }
        

            if (pid == 0){
                //child process
                //simulate travel time
                printf("[%d] started\n",getpid());
                pause(); //wait for signal to terminate
                exit(EXIT_SUCCESS);  //0 indicates successful termination
            }

            //store the child PID in the traveler struct
            travelers[i].pid = pid; 
            }
        }

    void wait_for_all_travelers(Traveler* travelers, int n) {

        // Two separate loops: first send SIGTERM to all children at once,
        // then wait for all of them. If we did kill+waitpid in one loop,
        // we would block waiting for child 0 before ever signaling child 1.

        for (int i = 0; i < n; i++) {
            if (travelers[i].pid > 0) {
                kill(travelers[i].pid, SIGTERM); //send termination signal to child 
            }
        }

        for (int i = 0; i < n; i++) {
            if (travelers[i].pid > 0) {
               waitpid(travelers[i].pid, NULL, 0); //wait for child to terminate
               travelers[i].pid = -1;
            }
        }

    }

    void spawn_travelers_ipc(Traveler* travelers, int n, int (*pipe_fds)[2], Graph* graph) {
        if (sync_init(graph->numVertices) != 0) {
            fprintf(stderr, "Failed to initialize node synchronization\n");
            exit(EXIT_FAILURE);
        }   
        for (int i = 0; i < n; i++){
            pid_t pid = fork();

            if (pid <0){
                perror("fork failed");
                exit(EXIT_FAILURE);
            }

            if (pid == 0){
                //child process
                //compute path and send IPC updates
                for (int j = 0; j < n; j++){
                    close(pipe_fds[j][0]); //close read end of all pipes
                }

                for (int j = 0; j < n; j++){
                    if (j != i){
                        close(pipe_fds[j][1]); //close write end of other pipes
                    }
                }

                int write_fd = pipe_fds[i][1];

                PathResult* result = dijkstra_compute_path(graph, travelers[i].src, travelers[i].dst);
                if (!result){
                    fprintf(stderr, "Failed to compute path for traveler %d\n", i);
                    exit(EXIT_FAILURE);
                }

                sigset_t grant_set;
                sigemptyset(&grant_set);
                sigaddset(&grant_set, SIGUSR1);
                sigprocmask(SIG_BLOCK, &grant_set, NULL);

                for (int step = 0; step < result->path_len; step++) {
                    int node = result->path[step];
                    int next = (step + 1 < result->path_len) ? result->path[step + 1] : -1;
                    struct timeval arrival;
                    gettimeofday(&arrival, NULL);

                    IPCMessage request = {0};
                    request.pid = getpid();
                    request.arrival_time =
                        (double)arrival.tv_sec + (double)arrival.tv_usec / 1000000.0;
                    request.path_remaining = result->path_len - step;
                    request.current_node = node;
                    request.next_node = next;
                    request.waiting_for_node = true;
                    request.blocked_at_node = node;
                    request.finished = false;
                    ipc_send(write_fd, &request);

                    int grant_signal;
                    sigwait(&grant_set, &grant_signal);
                    node_lock(node);

                    IPCMessage entered = {0};
                    entered.pid = getpid();
                    entered.path_remaining = result->path_len - step;
                    entered.current_node = node;
                    entered.next_node = next;
                    entered.waiting_for_node = false;
                    entered.blocked_at_node = -1;
                    entered.finished = false;
                    ipc_send(write_fd, &entered);

                    sleep(1);
                    node_unlock(node);

                    IPCMessage released = {0};
                    released.pid = getpid();
                    released.current_node = node;
                    released.next_node = next;
                    released.blocked_at_node = node;
                    ipc_send(write_fd, &released);

                    if (next != -1) {
                        usleep(400000);
                    }
                }

                IPCMessage done = {0};
                done.pid = getpid();
                done.current_node = travelers[i].dst;
                done.next_node = -1;
                done.waiting_for_node = false;
                done.blocked_at_node = -1;
                done.finished = true;
                ipc_send(write_fd, &done);

                free_path_result(result);
                close(write_fd);
                exit(EXIT_SUCCESS);
            }

            travelers[i].pid = pid; //store child PID in traveler struct
        }

        /* close all write ends after all children are forked */
        for (int i = 0; i < n; i++){
            close(pipe_fds[i][1]);
        }
        sync_cleanup();

    }
