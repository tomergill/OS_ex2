#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>

#define ROWS 4
#define COLS 4
#define CHARS_IN_TILE 7

int myPrint(char *string, unsigned int size)
{
    int bytes;
    if ((bytes = write(STDIN_FILENO, string, size)) == -1)
    {
        perror("error at printing");
        exit(-1);
    }
    return bytes;
}

void printBoard(int board[ROWS][COLS])
{
    int row, col;

    for (row = 0; row < ROWS; ++row)
    {
       for (col = 0; col < COLS; col++)
       {
           /*
            * printing a tile without a right border, in format of length 4.
            */
           printf("| %.4d ", board[row][col]);
       }
        printf("|\n");
    }
}

void boardFromInput(int board[ROWS][COLS])
{
    int row, col;

    scanf("%d", &board[0][0]);
    for (row = 0; row < ROWS; ++row)
    {
        for (col = 1; col < COLS; ++col)
        {
            scanf(",%d", &board[row][col]);
        }
    }
    scanf("\n");
}

int main() {
    printf("Hello, World!\n");
    return 0;
}