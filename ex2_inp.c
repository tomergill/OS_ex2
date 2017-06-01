#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>

#define ROWS 4
#define COLS 4

#define DEBUG

void printBoard(int board[ROWS][COLS]) {
    int row, col;

    for (row = 0; row < ROWS; ++row) {
        for (col = 0; col < COLS; col++) {
            /*
             * if board[row][col] is not 0, print a tile without a right
             * border, in format of length 4.
             * otherwise, print blank tile (5 spaces).
             */
            if (board[row][col] != 0)
                printf("| %.4d ", board[row][col]);
            else
                printf("|      ");
        }
        printf("|\n");
    }
}

void boardFromInput(int board[ROWS][COLS]) {
    int row, col;

    scanf("%d", &board[0][0]);
    col = 1;
    for (row = 0; row < ROWS; ++row) {
        for (; col < COLS; ++col) {
            //scanning each
            scanf(",%d", &board[row][col]);
        }
        col = 0;
    }
    //scanf("\n");
}

void SIGUSR1Handler(int signal) {
    if (signal == SIGUSR1) {
        int board[ROWS][COLS], i, j;
        for (i = 0; i < ROWS; ++i) {
            for (j = 0; j < COLS; ++j) {
                board[i][j] = 0;
            }
        }
        boardFromInput(board);
        printBoard(board);
    }
}

void SIGINTHandler(int signal) {
    if (signal == SIGINT) {
        printf("BYE BYE\n");
        exit(EXIT_SUCCESS);
    }
}

int main() {
    int i, j;
    struct sigaction sigActUSR, sigActINT;
    sigset_t sigSetUSR, sigSetINT;


    sigActUSR.sa_handler = SIGUSR1Handler;
    sigemptyset(&sigSetUSR);
    sigaddset(&sigSetUSR, SIGINT);
    sigActUSR.sa_mask = sigSetUSR;
    sigActUSR.sa_flags = 0;

    if (sigaction(SIGUSR1, &sigActUSR, NULL) != 0) {
        perror("sigaction of SIGUSR1 error");
        exit(EXIT_FAILURE);
    }

    sigActINT.sa_handler = SIGINTHandler;
    sigemptyset(&sigSetINT);
    sigaddset(&sigSetINT, SIGUSR1);
    sigActINT.sa_mask = sigSetINT;
    sigActINT.sa_flags = 0;

    if (sigaction(SIGINT, &sigActINT, NULL) != 0) {
        perror("sigaction of SIGINT error");
        exit(EXIT_FAILURE);
    }

    char buffer[256];
    buffer[0] = '\0';
    while (1) {
#ifdef DEBUG
        printf("type \"usr\" for SIGUSR1 (inserting a board).\ntype \"cls\" "
                       "for SIGINT (closing the program).\n");
        scanf("%s", buffer);
        if (strcmp(buffer, "usr") == 0)
            kill(getpid(), SIGUSR1);
        else if (strcmp(buffer, "cls") == 0)
            kill(getpid(), SIGINT);
        else
            continue;
#endif
    }
}