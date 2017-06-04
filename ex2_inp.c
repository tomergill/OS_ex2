/*******************************************************************************
 * Student name: Tomer Gill
 * Student: 318459450
 * Course Exercise Group: 01 (CS student, actual group is 89231-03)
 * Exercise name: Exercise 2
*******************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>

#define ROWS 4 //num of rows
#define COLS 4 //num of columns
#define GOAL 2048 //goal tile

#define DEBUG 0 //for debugging purposes

pid_t pidToSig = 0; //pid of process to send signals to (sigint only).

typedef enum {
    NOT_OVER = 0, FULL_BOARD, CONTAINS_GOAL
} GAME_RESULT; //result of checking if the game has ended.

/******************************************************************************
 * function name: PrintBoard
 * The Input: The board, filled with values.
 * The output: Prints the board to the screen.
 * The Function operation: Prints each cell in format | xxxx |, where xxxx is
 * the number in a 4 digit form (padded with 0s if needed), or blank tile
 * (|      |) if tile's value is 0.
*******************************************************************************/
void PrintBoard(int board[ROWS][COLS]) {
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
    printf("\n");
}

/******************************************************************************
 * function name: BoardFromInput
 * The Input: Pointer to an empty board.
 * The output: Fills the board with values from stdin.
 * The Function operation: Reads from stdin values seperated by commas, and
 * put them in board.
*******************************************************************************/
void BoardFromInput(int board[ROWS][COLS]) {
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

/******************************************************************************
 * function name: GameEndCheck
 * The Input: A full board.
 * The output: CONTAINS_GOAL if goal is in board (won), FULL_BOARD if board
 * is full (lost), otherwise NOT_OVER indicating the game isn't over.
 * The Function operation: Goes over each cell and checks if one of them
 * contains the goal, and if not if board is full.
*******************************************************************************/
GAME_RESULT GameEndCheck(int board[ROWS][COLS]) {
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

/******************************************************************************
 * function name: SIGUSR1Handler
 * The Input: The signal.
 * The output: Gets the board in row format and prints it in cell format, and
 * if the game ends prints the appropriate message,
 * The Function operation: Scans the board from stdin, prints it to stdout
 * and goes over it to check if game is over, and if is sends SIGINT to
 * pidToSig.
*******************************************************************************/
void SIGUSR1Handler(int signal) {
    if (signal == SIGUSR1) {
        int board[ROWS][COLS], i, j;
        for (i = 0; i < ROWS; ++i) {
            for (j = 0; j < COLS; ++j) {
                board[i][j] = 0;
            }
        }
        BoardFromInput(board);
        PrintBoard(board);
        GAME_RESULT result = GameEndCheck(board);
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
//        if (kill(pidToSig, SIGUSR2) != 0)
//            perror("error sending end of printing signal");
    }
}

/******************************************************************************
 * function name: SIGINTHandler
 * The Input: The signal.
 * The output: Prints "BYE BYE" and exits.
 * The Function operation: Prints "BYE BYE" and exits.
*******************************************************************************/
void SIGINTHandler(int signal) {
    if (signal == SIGINT) {
        printf("BYE BYE\n");
        exit(EXIT_SUCCESS);
    }
}

/******************************************************************************
 * function name: main
 * The Input: A pid to signal SIGINT to when game is over.
 * The output: Acts as GUI ti the game.
 * The Function operation: in an endless loop, pausing the program until a
 * signal will end it.
*******************************************************************************/
int main(int argc, char *argv[]) {
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
