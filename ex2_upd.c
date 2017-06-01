//
// Created by tomer on 01/06/17.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

#define ROWS 4
#define COLS 4

static int **globalBoard;
static pid_t globalPidToSig;

void printBoardAsLine(int board[ROWS][COLS])
{
    int row, col;

    printf("%d", board[0][0]);
    col = 1;
    for (row = 0; row < ROWS; ++row) {
        for (; col < COLS; ++col) {
            //scanning each
            printf(",%d", board[row][col]);
        }
        col = 0;
    }
}

void randomTile(int tile[2])
{
    tile[0] = rand() % ROWS;
    tile[1] = rand() % COLS;
}

void SIGALRMHandler(int signal)
{
    if (signal == SIGALRM)
    {
        int places[ROWS * COLS][2], length = 0, i, j;

        for (i = 0; i < ROWS; ++i)
        {
            for (j = 0; j < COLS; ++j)
            {
                if (globalBoard[i][j] == 0)
                {
                    places[length][0] = i;
                    places[length][1] = j;
                    length++;
                }
            }
        }

        int place = rand() % length;
        if (length)
        {
            if (globalBoard[places[place][0]][places[place][1]] == 0)
                globalBoard[places[place][0]][places[place][1]] = 2;

            printBoardAsLine(globalBoard);
            if (kill(globalPidToSig, SIGUSR1) != 0)
                perror("kill error in SIGALRMHandler");
        }

        alarm(rand() % 5 + 1); //new x
    }
}

void SIGINTHandler(int signal)
{
    alarm(0);
    exit(EXIT_SUCCESS);
}

void newGame(int board[ROWS][COLS], pid_t pidToSig)
{
    int i, j, tile[2], otherTile[2], x;
    for (i = 0; i < ROWS; ++i) {
        for (j = 0; j < COLS; ++j) {
            board[i][j] = 0;
        }
    }

    x = rand() % 5 + 1; //sets x between 1 to 5 included.

    randomTile(tile);
    do
    {
        randomTile(otherTile);
    } while (tile[0] == otherTile[0] && tile[1] == otherTile[1]);

    board[tile[0]][tile[1]] = 2;
    board[otherTile[0]][otherTile[1]] = 2;

    printBoardAsLine(board);

    if (kill(pidToSig, SIGUSR1) != 0)
        perror("error first kill SIGUSR1 for printing");

    printf("\nsetting alarm to %d seconds\n", x);
    alarm(x);
}

int main(int argc, char *argv[])
{
    int board[ROWS][COLS], x;
    pid_t pidToSig;
    int tile[2], otherTile[2];
    struct sigaction sigActALRM, sigActINT;

    globalBoard = board;

    srand(time(NULL));

    if (argc != 2)
    {
        perror("usage error in ex2_upd");
        exit(EXIT_FAILURE);
    }
    globalPidToSig = pidToSig = atoi(argv[1]);

    sigActALRM.sa_flags = 0;
    sigemptyset(&(sigActALRM.sa_mask));
    sigActALRM.sa_handler = SIGALRMHandler;
    if (sigaction(SIGALRM, &sigActALRM, NULL) != 0)
    {
        perror("error with SIGALRM sigaction");
        exit(EXIT_FAILURE);
    }

    sigemptyset(&(sigActINT.sa_mask));
    sigaddset(&(sigActINT.sa_mask), SIGALRM);
    sigActINT.sa_flags = 0;
    sigActINT.sa_handler = SIGINTHandler;
    if (sigaction(SIGINT, &sigActINT, NULL) != 0)
    {
        perror("error with SIGINT sigaction");
        exit(EXIT_FAILURE);
    }

    newGame(board, pidToSig);

    while (1);
}