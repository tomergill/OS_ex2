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
#define GOAL 2048

#define DEBUG 1

static int board[ROWS][COLS];
static pid_t globalPidToSig;

void printBoardAsLine() {
    int row, col;

    printf("%d", board[0][0]);
    col = 1;
    for (row = 0; row < ROWS; ++row) {
        for (; col < COLS; ++col) {
            //scanning each
            printf(",%d", board[row][col]);
        }
        col = 0;
//        printf("\n");
    }
    printf("\n");
}

void randomTile(int tile[2]) {
    tile[0] = rand() % ROWS;
    tile[1] = rand() % COLS;
}

void SIGALRMHandler(int signal) {
    if (signal == SIGALRM) {
        int places[ROWS * COLS][2], length = 0, i, j;

        for (i = 0; i < ROWS; ++i) {
            for (j = 0; j < COLS; ++j) {
                if (board[i][j] == 0) {
                    places[length][0] = i;
                    places[length][1] = j;
                    length++;
                }
            }
        }


        if (length) {
            int place = rand() % length;
            if (board[places[place][0]][places[place][1]] == 0)
                board[places[place][0]][places[place][1]] = 2;

            printBoardAsLine();
            if (kill(globalPidToSig, SIGUSR1) != 0)
                perror("kill error in SIGALRMHandler");
        }

        int x = rand() % 5 + 1;//new x
        //printf("setting alarm for %d seconds new alarm after alarm\n", x);
        alarm(x);
    }
}

void SIGINTHandler(int signal) {
    if (signal == SIGINT) {
        alarm(0);
        exit(EXIT_SUCCESS);
    }
}

void newGame(pid_t pidToSig) {
    int i, j, tile[2], otherTile[2], x;
    for (i = 0; i < ROWS; ++i) {
        for (j = 0; j < COLS; ++j) {
            board[i][j] = 0;
        }
    }

    x = rand() % 5 + 1; //sets x between 1 to 5 included.

    randomTile(tile);
    do {
        randomTile(otherTile);
    } while (tile[0] == otherTile[0] && tile[1] == otherTile[1]);

    board[tile[0]][tile[1]] = 2;
    board[otherTile[0]][otherTile[1]] = 2;

    printBoardAsLine();

    if (kill(pidToSig, SIGUSR1) != 0)
        perror("error first kill SIGUSR1 for printing");

//    printf("\nsetting alarm to %d seconds in newGame()\n", x);
    alarm(x);
}

int isRestOfRowNotEmptyLeft(int row, int col) {
    if (row < 0 || row >= ROWS)
        return 0;
    int sum = 0, j;
    for (j = col; j < COLS; ++j)
        sum += board[row][j];
    return sum;
}

int isRestOfRowNotEmptyRight(int row, int col) {
    if (row < 0 || row >= ROWS)
        return 0;
    int sum = 0, j;
    for (j = col; j >= 0; --j)
        sum += board[row][j];
    return sum;
}

int isRestOfColNotEmptyUp(int row, int col) {
    if (col < 0 || col >= COLS)
        return 0;
    int sum = 0, i;
    for (i = row; i < ROWS; ++i)
        sum += board[i][col];
    return sum;
}

int isRestOfColNotEmptyDown(int row, int col) {
    if (col < 0 || col >= COLS)
        return 0;
    int sum = 0, i;
    for (i = row; i >= 0; --i)
        sum += board[i][col];
    return sum;
}

void moveLeft() {
    int i, j, k, temp;

    for (i = 0; i < ROWS; ++i) {
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

    for (i = 0; i < ROWS; ++i) {
        for (j = 1; j < COLS; ++j) //starts from second column
        {
            if (board[i][j] == 0) //empty cell, just continue
                continue;
            if (board[i][j] == board[i][j - 1]) //equal tiles, add
            {
                board[i][j - 1] += board[i][j];
                board[i][j] = 0;
            } else if (board[i][j - 1] == 0) //left cell is empty
            {
                temp = j;

                /*
                 * while possible, move tile left.
                 */
                while (j - 1 >= 0 && board[i][j - 1] == 0) {
                    board[i][j - 1] = board[i][j];
                    board[i][j] = 0;
                    j--;
                }
                j = temp;
            }
        }
    }
}

void moveRight() {
    int i, j, k, temp;

    for (i = 0; i < ROWS; ++i) {
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

    for (i = 0; i < ROWS; ++i) {
        for (j = COLS - 2; j >= 0; --j) //starts from second last column
        {
            if (board[i][j] == 0) //empty cell, just continue
                continue;
            if (board[i][j] == board[i][j + 1]) //equal tiles, add
            {
                board[i][j + 1] += board[i][j];
                board[i][j] = 0;
            } else if (board[i][j + 1] == 0) //left cell is empty
            {
                temp = j;

                /*
                 * while possible, move tile left.
                 */
                while (j + 1 < COLS && board[i][j + 1] == 0) {
                    board[i][j + 1] = board[i][j];
                    board[i][j] = 0;
                    j++;
                }
                j = temp;
            }
        }
    }
}

void moveUp() {
    int i, j, k, temp;

    for (j = 0; j < COLS; ++j) {
        for (i = 0; i < ROWS; ++i) {
            while (board[i][j] == 0 &&
                   isRestOfColNotEmptyUp(i, j)) {
                for (k = i + 1; k < ROWS; ++k) {
                    board[k - 1][j] = board[k][j];
                }
                board[ROWS - 1][j] = 0;
            }
        }
    }

    for (j = 0; j < COLS; ++j) {
        for (i = 1; i < ROWS; ++i)  //starts from second column
        {
            if (board[i][j] == 0) //empty cell, just continue
                continue;
            if (board[i][j] == board[i - 1][j]) //equal tiles, add
            {
                board[i - 1][j] += board[i][j];
                board[i][j] = 0;
            } else if (board[i - 1][j] == 0) //upper cell is empty
            {
                temp = i;

                /*
                 * while possible, move tile left.
                 */
                while (i - 1 >= 0 && board[i - 1][j] == 0) {
                    board[i - 1][j] = board[i][j];
                    board[i][j] = 0;
                    i--;
                }
                i = temp;
            }
        }
    }
}

void moveDown() {
    int i, j, k, temp;

    for (j = 0; j < COLS; ++j) {
        for (i = ROWS - 1; i >= 0; --i) {
            while (board[i][j] == 0 && isRestOfColNotEmptyDown(i, j)) {
                for (k = i - 1; k >= 0; --k) {
                    board[k + 1][j] = board[k][j];
                }
                board[0][j] = 0;
            }
        }
    }

    for (j = 0; j < COLS; ++j) {
        for (i = ROWS - 2; i >= 0; --i) //starts from second last column
        {
            if (board[i][j] == 0) //empty cell, just continue
                continue;
            if (board[i][j] == board[i + 1][j]) //equal tiles, add
            {
                board[i + 1][j] += board[i][j];
                board[i][j] = 0;
            } else if (board[i + 1][j] == 0) //lower cell is empty
            {
                temp = i;

                /*
                 * while possible, move tile left.
                 */
                while (i + 1 < ROWS && board[i + 1][j] == 0) {
                    board[i + 1][j] = board[i][j];
                    board[i][j] = 0;
                    i++;
                }
                i = temp;
            }
        }
    }
}


void gameEndCheck() {
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

    if (containsGoal || fullBoard) {
        kill(globalPidToSig, SIGUSR2);
        exit(EXIT_SUCCESS);
    }

}

int main(int argc, char *argv[]) {
    pid_t pidToSig;
    struct sigaction sigActALRM, sigActINT;

    srand(time(NULL));

    if (argc != 2) {
        perror("usage error in ex2_upd");
        exit(EXIT_FAILURE);
    }
    globalPidToSig = pidToSig = atoi(argv[1]);

    sigActALRM.sa_flags = 0;
    sigemptyset(&(sigActALRM.sa_mask));
    sigActALRM.sa_handler = SIGALRMHandler;
    if (sigaction(SIGALRM, &sigActALRM, NULL) != 0) {
        perror("error with SIGALRM sigaction");
        exit(EXIT_FAILURE);
    }

    sigemptyset(&(sigActINT.sa_mask));
    sigaddset(&(sigActINT.sa_mask), SIGALRM);
    sigActINT.sa_flags = 0;
    sigActINT.sa_handler = SIGINTHandler;
    if (sigaction(SIGINT, &sigActINT, NULL) != 0) {
        perror("error with SIGINT sigaction");
        exit(EXIT_FAILURE);
    }

    newGame(pidToSig);
    int key;
    while (1) {
        system("stty cbreak -echo");
        key = getchar();
        system("stty cooked echo");

        switch (key) {
            case 'a':
            case 'A':
                alarm(0);
                moveLeft();
                break;
            case 'd':
            case 'D':
                alarm(0);
                moveRight();
                break;
            case 'w':
            case 'W':
                alarm(0);
                moveUp();
                break;
            case 'x':
            case 'X':
                alarm(0);
                moveDown();
                break;
            case 's':
            case 'S':
                alarm(0);
                newGame(pidToSig);
                continue;
#if DEBUG
            case 'q':
            case 'Q':
                alarm(0);
                kill(getpid(), SIGINT);
                break;
            case 'b':
            case 'B':
                alarm(0);
                kill(getpid(), SIGALRM);
                continue;
#endif
            default:
                continue;
        }
        printBoardAsLine();
        gameEndCheck();
        int x = rand() % 5 + 1;
        alarm(x);
        kill(pidToSig, SIGUSR1);
    }
}