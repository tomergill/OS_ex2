//
// Created by tomer on 02/06/17.
//

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

pid_t updPid = 0, inpPid = 0;
int fd = -1;

void SIGALRMHandler(int signal) {
    if (signal == SIGALRM) {
        if (updPid)
            if (kill(updPid, SIGINT) != 0)
                perror("error sending upd sigint");
        if (inpPid)
            if (kill(inpPid, SIGINT) != 0)
                perror("error sending inp sigint");
        if (fd > 2) {
            if (close(fd) != 0)
                perror("error closing file in SIGALRMHandler");
            if (unlink("temp.txt") != 0)
                perror("error deleting temp.txt");
        }

        exit(EXIT_SUCCESS);
    }
}

void SIGUSR1Handler(int signal) {
    if (signal == SIGUSR1) {
        lseek(fd, 0, SEEK_SET);
        if (kill(inpPid, SIGUSR1) != 0)
            perror("error sending inp sigusr1");
    }
}

int main(int argc, char *argv[])
{
    int y;
    struct sigaction sigActALRM, sigActUSR1;

    if (argc != 2) {
        perror("usage error in ex2 main");
        exit(EXIT_FAILURE);
    }
    y = atoi(argv[1]);

    sigActALRM.sa_flags = 0;
    sigemptyset(&(sigActALRM.sa_mask));
    sigaddset(&(sigActALRM.sa_mask), SIGUSR1);
    sigActALRM.sa_handler = SIGALRMHandler;
    if (sigaction(SIGALRM, &sigActALRM, NULL) != 0) {
        perror("error with SIGALRM sigaction");
        exit(EXIT_FAILURE);
    }

    alarm(y);

    sigActUSR1.sa_flags = 0;
    sigemptyset(&(sigActUSR1.sa_mask));
    sigActUSR1.sa_handler = SIGUSR1Handler;
    if (sigaction(SIGUSR1, &sigActUSR1, NULL) != 0)
    {
        perror("error with SIGUSR1 sigaction");
        exit(EXIT_FAILURE);
    }

    if ((fd = open("./temp.txt", O_CREAT | O_TRUNC | O_RDWR, S_IWUSR | S_IRUSR |
                                                  S_IRGRP)) == -1)
    {
        perror("error opening temp.txt");
        exit(EXIT_FAILURE);
    }

    if ((inpPid = fork()) < 0)
    {
        perror("error first fork to inp");
        if (close(fd) != 0)
            perror("error closing temp.txt in inp fork failure");
        exit(EXIT_FAILURE);
    }
    else if (inpPid == 0) //son
    {
        if (dup2(fd, STDIN_FILENO) == -1)
        {
            perror("error dup2 in inp");
        }
        char *args[] = {"./ex2_inp.out", NULL};
        if (execvp(args[0], args) != 0)
            perror("execvp error in inp");
        exit(EXIT_FAILURE); //only if execvp failed
    }

    //father continues
    if ((updPid = fork()) < 0)
    {
        perror("error second fork to upd");
        if (close(fd) != 0)
            perror("error closing temp.txt in upd fork failure");
        exit(EXIT_FAILURE);
    }
    else if (updPid == 0) //son
    {
        if (dup2(fd, STDOUT_FILENO) == -1)
        {
            perror("error dup2 in upd");
        }
        char pidBuffer[64];
        sprintf(pidBuffer, "%d", getppid());
        char *args[] = {"./ex2_upd.out", pidBuffer, NULL};
        if (execvp(args[0], args) != 0)
            perror("execvp error in upd");
        exit(EXIT_FAILURE); //only if execvp failed
    }



    //father continues endless loop, waiting for signals.
    while (1);
}