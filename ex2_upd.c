/*******************************************************************************
 * Student name: Tomer Gill
 * Student: 318459450
 * Course Exercise Group: 01 (CS student, actual group is 89231-03)
 * Exercise name: Exercise 2
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#define ROWS 4 //num of rows
#define COLS 4 //num of columns


//#define DEBUG 0 //for debug purposes.

/*
 * The board and the pid are global so signal handler function can access them.
 */
static int board[ROWS][COLS];
static pid_t globalPidToSig;

/******************************************************************************
 * function name: PrintBoardAsLine
 * The Input: None.
 * The output: Prints the board to stdout in a row format.
 * The Function operation: Row format is value-comma-value, from the
 * upper-left corner of the board left and down.
*******************************************************************************/
void PrintBoardAsLine() {
    int row, col;
    char buffer[6];

    sprintf(buffer, "%d", board[0][0]);
    if (write(STDOUT_FILENO, buffer, strlen(buffer)) == -1)
        perror("error writing to file");
    col = 1;
    for (row = 0; row < ROWS; ++row) {
        for (; col < COLS; ++col) {
            //scanning each
            sprintf(buffer, ",%d", board[row][col]);
            if (write(STDOUT_FILENO, buffer, strlen(buffer)) == -1)
                perror("error writing to file");
        }
        col = 0;
//        printf("\n");
    }
    if (write(STDOUT_FILENO, "\n", strlen("\n")) == -1)
        perror("error writing to file");
}

/******************************************************************************
 * function name: RandomTile
 * The Input: Pointer to a tile (2 ints).
 * The output: Puts in said array random cordinates.
 * The Function operation:
*******************************************************************************/
void RandomTile(int tile[2]) {
    tile[0] = rand() % ROWS;
    tile[1] = rand() % COLS;
}

/******************************************************************************
 * function name: SIGALRMHandler
 * The Input: The signal.
 * The output: adds a 0002 to a random place in the board and print it.
 * The Function operation: Goes over the board, inserting to an array all the
 * empty places. Then randomly choose from them where to add the 0002, and
 * prints the board afterward. Also randomly decides a next waiting time
 * between 1 and 5 included.
*******************************************************************************/
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

            PrintBoardAsLine();
            if (kill(globalPidToSig, SIGUSR1) != 0)
                perror("kill error in SIGALRMHandler");
        }

        int x = rand() % 5 + 1;//new x
        //printf("setting alarm for %d seconds new alarm after alarm\n", x);
        alarm(x);
    }
}

/******************************************************************************
 * function name: SIGINTHandler
 * The Input: The signal.
 * The output: Exits.
 * The Function operation: Cancels any alarms there were, using system to
 * return writing into terminal (using the rows given us in the exercise) and
 * them exits.
*******************************************************************************/
void SIGINTHandler(int signal) {
    if (signal == SIGINT) {
        alarm(0);
        system("stty cooked echo");

        exit(EXIT_SUCCESS);
    }
}

/******************************************************************************
 * function name: NewGame
 * The Input: a pid to signal to.
 * The output: Creating a fresh board, printing it, sending SIGUSR1 to
 * pidToSig, and sets an alarm to 1 up to 5 seconds.
 * The Function operation: Puts zeros in all of board places, then choose 2
 * places at random that will hold 0002 instead, then printing, sending the
 * signal and creating a new alarm.
*******************************************************************************/
void NewGame(pid_t pidToSig) {
    int i, j, tile[2], otherTile[2], x;
    for (i = 0; i < ROWS; ++i) {
        for (j = 0; j < COLS; ++j) {
            board[i][j] = 0;
        }
    }

    x = rand() % 5 + 1; //sets x between 1 to 5 included.

    RandomTile(tile);
    do {
        RandomTile(otherTile);
    } while (tile[0] == otherTile[0] && tile[1] == otherTile[1]);

    board[tile[0]][tile[1]] = 2;
    board[otherTile[0]][otherTile[1]] = 2;

    PrintBoardAsLine();

    if (kill(pidToSig, SIGUSR1) != 0)
        perror("error first kill SIGUSR1 for printing");

//    printf("\nsetting alarm to %d seconds in NewGame()\n", x);
    alarm(x);
}

/******************************************************************************
 * function name: IsRestOfRowNotEmptyLeft
 * The Input: a row and column.
 * The output: 0 if the rest of the row to the left from [row][col] is empty,
 * otherwise value > 0.
 * The Function operation: Goes left from [row][col] and sums the values.
*******************************************************************************/
int IsRestOfRowNotEmptyLeft(int row, int col) {
    if (row < 0 || row >= ROWS)
        return 0;
    int sum = 0, j;
    for (j = col; j < COLS; ++j)
        sum += board[row][j];
    return sum;
}

/******************************************************************************
 * function name: IsRestOfRowNotEmptyRight
 * The Input: a row and column.
 * The output: 0 if the rest of the row to the right from [row][col] is empty,
 * otherwise value > 0.
 * The Function operation: Goes right from [row][col] and sums the values.
*******************************************************************************/
int IsRestOfRowNotEmptyRight(int row, int col) {
    if (row < 0 || row >= ROWS)
        return 0;
    int sum = 0, j;
    for (j = col; j >= 0; --j)
        sum += board[row][j];
    return sum;
}

/******************************************************************************
 * function name: IsRestOfColNotEmptyUp
 * The Input: a row and column.
 * The output: 0 if the upper part of the column from [row][col] is empty,
 * otherwise value > 0.
 * The Function operation: Goes up from [row][col] and sums the values.
*******************************************************************************/
int IsRestOfColNotEmptyUp(int row, int col) {
    if (col < 0 || col >= COLS)
        return 0;
    int sum = 0, i;
    for (i = row; i < ROWS; ++i)
        sum += board[i][col];
    return sum;
}

/******************************************************************************
 * function name: IsRestOfColNotEmptyDown
 * The Input: a row and column.
 * The output: 0 if the lower part of the column from [row][col] is empty,
 * otherwise value > 0.
 * The Function operation: Goes down from [row][col] and sums the values.
*******************************************************************************/
int IsRestOfColNotEmptyDown(int row, int col) {
    if (col < 0 || col >= COLS)
        return 0;
    int sum = 0, i;
    for (i = row; i >= 0; --i)
        sum += board[i][col];
    return sum;
}

/******************************************************************************
 * function name: MoveLeft
 * The Input: None.
 * The output: Moves the board left.
 * The Function operation:
 * First each row is moved to the left as much it can go.
 * Then each row is passed from the left to the right, and each cell is checked:
 * # if the cell is 0, continue.
 * # if the cell is the same as the cell before it, they are merged to the
 *      previous cell and this cell is 0.
 * # if the previous cell is 0, the whole line moves left, and then is
 *      checked if can go more to the left - if so it continues as was done
 *      before, until it can no more go.
*******************************************************************************/
void MoveLeft() {
    int i, j, k, temp;

    for (i = 0; i < ROWS; ++i) {
        for (j = 0; j < COLS; ++j) {
            while (board[i][j] == 0 &&
                    IsRestOfRowNotEmptyLeft(i, j)) {
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
//                    board[i][j] = 0;
                    for (k = j + 1; k < COLS; ++k) {
                        board[i][k - 1] = board[i][k];
                    }
                    board[i][COLS - 1] = 0;
                    j--;
                }
                //j = temp;
            }
        }
    }
}

/******************************************************************************
 * function name: MoveRight
 * The Input: None.
 * The output: Moves the board right.
 * The Function operation:
 * First each row is moved to the right as much it can go.
 * Then each row is passed from the right to the left, and each cell is checked:
 * # if the cell is 0, continue.
 * # if the cell is the same as the cell before it, they are merged to the
 *      previous cell and this cell is 0.
 * # if the previous cell is 0, the whole line moves right, and then is
 *      checked if can go more to the right - if so it continues as was done
 *      before, until it can no more go.
*******************************************************************************/
void MoveRight() {
    int i, j, k, temp;

    for (i = 0; i < ROWS; ++i) {
        for (j = COLS - 1; j >= 0; --j) {
            while (board[i][j] == 0 &&
                    IsRestOfRowNotEmptyRight(i, j)) {
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
                    for (k = j - 1; k >= 0; --k) {
                        board[i][k + 1] = board[i][k];
                    }
                    board[i][0] = 0;
//                    board[i][j] = 0;
                    j++;
                }
//                j = temp;
            }
        }
    }
}

/******************************************************************************
 * function name: MoveUp
 * The Input: None.
 * The output: Moves the board up.
 * The Function operation:
 * First each column is moved up as much it can go.
 * Then each column is passed from the top to the bottom, and each cell is
 * checked:
 * # if the cell is 0, continue.
 * # if the cell is the same as the cell before it, they are merged to the
 *      previous cell and this cell is 0.
 * # if the previous cell is 0, the whole column moves up, and then is
 *      checked if can go more to the top - if so it continues as was done
 *      before, until it can no more go.
*******************************************************************************/
void MoveUp() {
    int i, j, k, temp;

    for (j = 0; j < COLS; ++j) {
        for (i = 0; i < ROWS; ++i) {
            while (board[i][j] == 0 &&
                    IsRestOfColNotEmptyUp(i, j)) {
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
                    for (k = i + 1; k < ROWS; ++k) {
                        board[k - 1][j] = board[k][j];
                    }
                    board[ROWS - 1][j] = 0;
//                    board[i][j] = 0;
                    i--;
                }
//                i = temp;
            }
        }
    }
}

/******************************************************************************
 * function name: MoveDown
 * The Input: None.
 * The output: Moves the board down.
 * The Function operation:
 * First each column is moved down as much it can go.
 * Then each column is passed from the bottom to the top, and each cell is
 * checked:
 * # if the cell is 0, continue.
 * # if the cell is the same as the cell before it, they are merged to the
 *      previous cell and this cell is 0.
 * # if the previous cell is 0, the whole column moves down, and then is
 *      checked if can go more to the bottom - if so it continues as was done
 *      before, until it can no more go.
*******************************************************************************/
void MoveDown() {
    int i, j, k, temp;

    for (j = 0; j < COLS; ++j) {
        for (i = ROWS - 1; i >= 0; --i) {
            while (board[i][j] == 0 && IsRestOfColNotEmptyDown(i, j)) {
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
//                    board[i][j] = 0;
                    for (k = i - 1; k >= 0; --k) {
                        board[k + 1][j] = board[k][j];
                    }
                    board[0][j] = 0;
                    i++;
                }
//                i = temp;
            }
        }
    }
}

/******************************************************************************
 * function name: main
 * The Input: A pid to signal SIGUSR1 to.
 * The output: Manages the game.
 * The Function operation: In an endless loop, receives the key pressed by the
 * user, moves the board in the correct direction / starts the board anew,
 * prints it, sets a new time for an alarm (between 1 to 5 seconds) and
 * signals the received pid SIGUSR1.
*******************************************************************************/
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

    NewGame(pidToSig);
    int key;
    while (1) {
        key = 0;
        system("stty cbreak -echo");
        key = getchar();
        system("stty cooked echo");
        switch (key) {
            case 'a':
            case 'A':
                alarm(0);
                MoveLeft();
                break;
            case 'd':
            case 'D':
                alarm(0);
                MoveRight();
                break;
            case 'w':
            case 'W':
                alarm(0);
                MoveUp();
                break;
            case 'x':
            case 'X':
                alarm(0);
                MoveDown();
                break;
            case 's':
            case 'S':
                alarm(0);
                NewGame(pidToSig);
                continue;
#ifdef DEBUG
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
        PrintBoardAsLine();
        int x = rand() % 5 + 1;
        alarm(x);
        kill(pidToSig, SIGUSR1);
    }
}