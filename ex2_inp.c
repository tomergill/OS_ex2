#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>

#define ROWS 4
#define COLS 4
#define GOAL 2048

#define DEBUG 0 //for debugging purposes

pid_t pidToSig = 0;

typedef enum {
    NOT_OVER = 0, FULL_BOARD, CONTAINS_GOAL
} GAME_RESULT;

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
//    printf("read %d, ", board[0][0]);
    col = 1;
    for (row = 0; row < ROWS; ++row) {
        for (; col < COLS; ++col) {
            //scanning each
            scanf(",%d", &board[row][col]);
//            printf("%d, ", board[row][col]);
        }
        col = 0;
    }
    //scanf("\n");
//    printf("\n");
}

GAME_RESULT gameEndCheck(int board[ROWS][COLS]) {
    int i, j, containsGoal = 0, fullBoard = 1;

    //check if board contains a 2048 tile
    for (i = 0; i < ROWS && !containsGoal; ++i) {
        for (j = 0; j < COLS; ++j) {
            if (board[i][j] == GOAL) {
                containsGoal = 1;
                break;
            } else if (board[i][j] == 0) {
                fullBoard = 0;
            }
        }
    }

    if (containsGoal)
        return CONTAINS_GOAL;
    else if (fullBoard)
        return FULL_BOARD;
    else
        return NOT_OVER;
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
        GAME_RESULT result = gameEndCheck(board);
        if (result == CONTAINS_GOAL) {
            printf("Congratulations!\n");
            if (kill(pidToSig, SIGINT) != 0)
                perror("error sending end signal after win");
            exit(EXIT_SUCCESS);
        } else if (result == FULL_BOARD) {
            printf("Game Over!\n");
            if (kill(pidToSig, SIGINT) != 0)
                perror("error sending end signal after lose");
            exit(EXIT_SUCCESS);
        }
        //result == NOT_OVER
        if (kill(pidToSig, SIGUSR2) != 0)
            perror("error sending end of printing signal");
    }
}

void SIGINTHandler(int signal) {
    if (signal == SIGINT) {
        printf("BYE BYE\n");
        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char *argv[]) {
    int i, j;
    struct sigaction sigActUSR, sigActINT;
    sigset_t sigSetUSR, sigSetINT;

    if (argc != 2) {
        perror("usage error in ex2_inp");
        exit(EXIT_FAILURE);
    }
    pidToSig = atoi(argv[1]);

    sigActUSR.sa_handler = SIGUSR1Handler;
    sigemptyset(&sigSetUSR);
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

    while (1) {
#if DEBUG
        char buffer[256];
        buffer[0] = '\0';
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
        pause();
    }
}
