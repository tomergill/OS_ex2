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

static int board[ROWS][COLS];
static pid_t globalPidToSig;

void printBoardAsLine()
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
        printf("\n");
    }
    printf("\n");
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
                if (board[i][j] == 0)
                {
                    places[length][0] = i;
                    places[length][1] = j;
                    length++;
                }
            }
        }


        if (length)
        {
            int place = rand() % length;
            if (board[places[place][0]][places[place][1]] == 0)
                board[places[place][0]][places[place][1]] = 2;

            printBoardAsLine();
            if (kill(globalPidToSig, SIGUSR1) != 0)
                perror("kill error in SIGALRMHandler");
        }

        int x = rand() % 5 + 1;
        printf("setting alarm for %d seconds\n", x);
        //alarm(x); //new x
    }
}

void SIGINTHandler(int signal)
{
    if (signal == SIGINT)
    {
        alarm(0);
        exit(EXIT_SUCCESS);
    }
}

void newGame(pid_t pidToSig)
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

    printBoardAsLine();

    if (kill(pidToSig, SIGUSR1) != 0)
        perror("error first kill SIGUSR1 for printing");

    printf("\nsetting alarm to %d seconds\n", x);
    //alarm(x);
}

int isRestOfRowNotEmptyLeft(int row, int col)
{
    if (row < 0 || row >= ROWS)
        return 0;
    int sum = 0, j;
    for (j = col; j < COLS; ++j)
        sum += board[row][j];
    return sum;
}

int isRestOfRowNotEmptyRight(int row, int col)
{
    if (row < 0 || row >= ROWS)
        return 0;
    int sum = 0, j;
    for (j = col; j >= 0; --j)
        sum += board[row][j];
    return sum;
}

void moveLeft()
{
    int i, j, k, temp;

    for (i = 0; i < ROWS; ++i)
    {
            for (j = 0; j < COLS; ++j) {
                while (board[i][j] == 0 &&
                        isRestOfRowNotEmptyLeft(i, j)) {
                    for (k = j + 1; k < COLS; ++k) {
                        board[i][k - 1] = board[i][k];
                    }
                    board[i][COLS - 1] = 0;
                }
            }
    }

    for (i = 0; i < ROWS; ++i)
    {
        for (j = 1; j < COLS; ++j) //starts from second column
        {
            if (board[i][j] == 0) //empty cell, just continue
                continue;
            if (board[i][j] == board[i][j - 1]) //equal tiles, add
            {
                board[i][j - 1] += board[i][j];
                board[i][j] = 0;
            }
//            else if (board[i][j - 1] == 0) //left cell is empty
//            {
//                temp = j;
//
//                /*
//                 * while possible, move tile left.
//                 */
//                while (j - 1 >= 0 && board[i][j - 1] == 0)
//                {
//                    board[i][j - 1] = board[i][j];
//                    board[i][j] = 0;
//                    j--;
//                }
//                j = temp;
//            }
        }
    }
}

void moveRight()
{
    int i, j, k, temp;

    for (i = 0; i < ROWS; ++i)
    {
            for (j = COLS - 1; j >= 0; --j) {
                while (board[i][j] == 0 &&
                        isRestOfRowNotEmptyRight(i, j)) {
                    for (k = j - 1; k >= 0; --k) {
                        board[i][k + 1] = board[i][k];
                    }
                    board[i][0] = 0;
                }
            }
    }

    for (i = 0; i < ROWS; ++i)
    {
        for (j = COLS - 2; j >= 0; --j) //starts from second last column
        {
            if (board[i][j] == 0) //empty cell, just continue
                continue;
            if (board[i][j] == board[i][j + 1]) //equal tiles, add
            {
                board[i][j + 1] += board[i][j];
                board[i][j] = 0;
            }
//            else if (board[i][j + 1] == 0) //left cell is empty
//            {
//                temp = j;
//
//                /*
//                 * while possible, move tile left.
//                 */
//                while (j + 1 < ROWS && board[i][j + 1] == 0)
//                {
//                    board[i][j + 1] = board[i][j];
//                    board[i][j] = 0;
//                    j++;
//                }
//                j = temp;
//            }
        }
    }
}

int main(int argc, char *argv[])
{
    pid_t pidToSig;
    struct sigaction sigActALRM, sigActINT;

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

    newGame(pidToSig);
    int key;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (1)
    {
        system("stty cbreak -echo");
        key = getchar();
        system("stty cooked echo");

        alarm(0);

        switch (key)
        {
            case 'a':
            case 'A':
                moveLeft();
                break;
            case 'd':
            case 'D':
                moveRight();
                break;
            case 's':
            case 'S':
                newGame(pidToSig);
                continue;
            case 'q':
            case 'Q':
                kill(getpid(), SIGINT);
                break;
            case 'b':
            case 'B':
                kill(getpid(), SIGALRM);
                continue;
            default:
                continue;
        }
        printBoardAsLine();
        int x = rand() % 5 + 1;
        printf("\nsetting alarm to %d seconds\n", x);
//        alarm(x);
        kill(pidToSig, SIGUSR1);
    }
#pragma clang diagnostic pop
}