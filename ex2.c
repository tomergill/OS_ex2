//
// Created by tomer on 02/06/17.
//

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

pid_t updPid = 0, inpPid = 0;

void SIGALRMHandler(int signal) {
    if (signal == SIGALRM) {
        if (updPid)
            if (kill(updPid, SIGINT) != 0)
                perror("error sending upd sigint");
        if (inpPid)
            if (kill(inpPid, SIGINT) != 0)
                perror("error sending inp sigint");
        exit(EXIT_SUCCESS);
    }
}

void SIGUSR1Handler(int signal) {
    if (kill(inpPid, SIGUSR1) != 0)
        perror("error sending inp sigusr1");
}

int main(int argc, char *argv[])
{
    int y;
    struct sigaction sigActALRM;

    if (argc != 2) {
        perror("usage error in ex2 main");
        exit(EXIT_FAILURE);
    }
    y = atoi(argv[1]);

    sigActALRM.sa_flags = 0;
    sigemptyset(&(sigActALRM.sa_mask));
    sigActALRM.sa_handler = SIGALRMHandler;
    if (sigaction(SIGALRM, &sigActALRM, NULL) != 0) {
        perror("error with SIGALRM sigaction");
        exit(EXIT_FAILURE);
    }
}