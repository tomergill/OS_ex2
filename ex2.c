//
// Created by tomer on 02/06/17.
//

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

pid_t updPid = 0, inpPid = 0;
int fdUpd = -1, fdInp = -1;

void SIGALRMHandler(int signal) {
    if (signal == SIGALRM) {
//        printf("!!!!!!ALARM!\n");
        if (updPid)
            if (kill(updPid, SIGINT) != 0)
                perror("error sending upd sigint");
        if (inpPid)
            if (kill(inpPid, SIGINT) != 0)
                perror("error sending inp sigint");
        if (fdUpd > 2) {
            if (close(fdUpd) != 0)
                perror("error closing file in SIGALRMHandler");
            if (close(fdInp) != 0) {
                perror("error closing temp.txt 2");
            }
            if (unlink("temp.txt") != 0)
                perror("error deleting temp.txt");
        }
        wait(NULL);
        wait(NULL);
        exit(EXIT_SUCCESS);
    }
}

void SIGUSR1Handler(int signal) {
    if (signal == SIGUSR1) {
//        printf("***********SIGUSR1\n");
        //lseek(fdUpd, 0, SEEK_SET);
        if (kill(inpPid, SIGUSR1) != 0)
            perror("error sending inp sigusr1");
    }
}

//void SIGUSR2Handler(int signal)
//{
//    if (signal == SIGUSR2)
//    {
//        printf("###########SIGUSR2\n");
//        //truncate("./temp.txt", 0);
//        //lseek(fdUpd, 0, SEEK_SET);
//    }
//}

void SIGINTHandler(int signal)
{
    if (signal == SIGINT)
    {
//        printf("~~~~~~~~~~SIGINT\n");
        if (updPid) {
            if (kill(updPid, SIGKILL) != 0)
                perror("error sending upd sigint");
            wait(NULL);
            //sleep(1);
//            if (kill(inpPid, SIGKILL) != 0)
//                perror("error killing inp");
            //wait(NULL);
        }
        if (fdUpd > 2 ) {
            if (close(fdUpd) != 0) {
                perror("error closing temp.txt 1");
            }
            if (close(fdInp) != 0) {
                perror("error closing temp.txt 2");
            }
            if (unlink("temp.txt") != 0)
                perror("error deleting temp.txt");
        }

        sleep(1);
        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char *argv[])
{
    int y;
    struct sigaction sigActALRM, sigActUSR1, sigActINT;

    if (argc != 2) {
        perror("usage error in ex2 main");
        exit(EXIT_FAILURE);
    }
    y = atoi(argv[1]);

    sigActALRM.sa_flags = 0;
    sigemptyset(&(sigActALRM.sa_mask));
    sigaddset(&(sigActALRM.sa_mask), SIGUSR1);
    sigaddset(&(sigActALRM.sa_mask), SIGUSR2);
    sigActALRM.sa_handler = SIGALRMHandler;
    if (sigaction(SIGALRM, &sigActALRM, NULL) != 0) {
        perror("error with SIGALRM sigaction");
        exit(EXIT_FAILURE);
    }

//    printf("setting alarm to %d seconds\n", y);
    alarm(y);

    sigActUSR1.sa_flags = 0;
    sigemptyset(&(sigActUSR1.sa_mask));
    sigActUSR1.sa_handler = SIGUSR1Handler;
    if (sigaction(SIGUSR1, &sigActUSR1, NULL) != 0)
    {
        perror("error with SIGUSR1 sigaction");
        exit(EXIT_FAILURE);
    }

    /*sigActUSR2.sa_flags = 0;
    sigemptyset(&(sigActUSR2.sa_mask));
    sigaddset(&(sigActUSR2.sa_mask), SIGUSR1);
    sigActUSR2.sa_handler = SIGUSR2Handler;
    if (sigaction(SIGUSR2, &sigActUSR2, NULL) != 0) {
        perror("error with SIGUSR2 sigaction");
        exit(EXIT_FAILURE);
    }*/

    sigActINT.sa_flags = 0;
    sigemptyset(&(sigActINT.sa_mask));
    sigaddset(&(sigActINT.sa_mask), SIGUSR1);
    sigaddset(&(sigActINT.sa_mask), SIGUSR2);
    sigaddset(&(sigActINT.sa_mask), SIGALRM);
    sigActINT.sa_handler = SIGINTHandler;
    if (sigaction(SIGINT, &sigActINT, NULL) != 0) {
        perror("error with SIGINT sigaction");
        exit(EXIT_FAILURE);
    }

    if ((fdUpd = open("./temp.txt", O_CREAT | O_TRUNC | O_WRONLY,
                      S_IWUSR | S_IRUSR | S_IRGRP)) == -1)
    {
        perror("error opening temp.txt");
        exit(EXIT_FAILURE);
    }

    if ((fdInp = open("./temp.txt", O_RDONLY)) == -1)
    {
        perror("error opening temp.txt");
        exit(EXIT_FAILURE);
    }

//    printf("after opening temp.txt, before forks\n");


    //father continues
    if ((updPid = fork()) < 0)
    {
        perror("error second fork to upd");
        if (close(fdUpd) != 0)
            perror("error closing temp.txt in upd fork failure");
        exit(EXIT_FAILURE);
    }
    else if (updPid == 0) //son
    {
        if (dup2(fdUpd, STDOUT_FILENO) == -1)
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

//    printf("after second fork to upd, starting busy-waiting\n");



    if ((inpPid = fork()) < 0)
    {
        perror("error first fork to inp");
        if (close(fdUpd) != 0)
            perror("error closing temp.txt in inp fork failure");
        exit(EXIT_FAILURE);
    }
    else if (inpPid == 0) //son
    {

        if (dup2(fdInp, STDIN_FILENO) == -1)
        {
            perror("error dup2 in inp");
        }
        char pidBuffer[64];
        sprintf(pidBuffer, "%d", getppid());
        char *args[] = {"./ex2_inp.out", pidBuffer, NULL};
        if (execvp(args[0], args) != 0)
            perror("execvp error in inp");
        exit(EXIT_FAILURE); //only if execvp failed
    }
//    printf("after fork to inp");

    //father continues endless loop, waiting for signals.
    while (1) {pause();}
}