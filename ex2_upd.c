//
// Created by tomer on 01/06/17.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

#define ROWS 4
#define COLS 4

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

int main(int argc, char *argv[])
{
    int board[ROWS][COLS], i, j, x;
    pid_t pidToSig;
    int tile[2], otherTile[2];

    srand(time(NULL));

    if (argc != 2)
    {
        perror("usage error in ex2_upd");
        exit(EXIT_FAILURE);
    }
    pidToSig = atoi(argv[1]);

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

    kill(pidToSig, SIGUSR1);
}