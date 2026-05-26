#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "core/process_mgr.h"
#include "core/ipc.h"
#include "core/dijkstra.h"

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

    void spawn_travelers_ipc(Traveler* travelers, int n, int (*pipe_fds)[2], struct Graph* graph) {
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

                for (int step = 0; step < result->path_len; step++){
                    IPCMessage msg;
                    msg.pid = getpid();
                    msg.current_node = result->path[step];
                    msg.next_node = (step +1  < result->path_len) ? result->path[step + 1] : -1;
                    msg.finished = false;

                    ipc_send(write_fd, &msg);
                    usleep(1400000); /* 1.4 s per node */
                }

                IPCMessage done;
                done.pid = getpid();
                done.current_node = travelers[i].dst;
                done.next_node = -1;
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

    }
