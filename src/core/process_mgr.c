#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>  //fork(), pause()
#include <signal.h>     //SIGTERM, kill()
#include <sys/wait.h>       //waitpid()
#include "core/process_mgr.h"

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

