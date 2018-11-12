/*
 * Omer Zucker
 * 200876548
 */

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

# define BOARD_SIZE 20

typedef enum base_status { Vertical, Horizontal} baseStatus;
typedef enum game_status { On, Off } gameStatus;

/*
 * struct of single cell
 */
typedef struct cell_t {

    char value;

} Cell;

/*
 * 3 Bcell structs creates a base
 */
typedef struct base_cell {

    int row;
    int col;

} Bcell;

/*
 * a base which move its place by the user
 */
typedef struct threeCells {

    Bcell cell_1;
    Bcell cell_2;
    Bcell cell_3;
    baseStatus status;

} Base;

/*
 * the game comprise of a board, base and status
 */
typedef struct tetris {

    Cell board[BOARD_SIZE][BOARD_SIZE];
    Base base;
    gameStatus status;

} Game;


// our tetris game
Game game;
// error for system call
char* error = "Error in system call";


/*
 * initialize user's base
 */
void initBase();

/*
 * board creation
 */
void createBoard();

/*
 * game creation by board, base and status
 */
void createGame();

/*
 * prints the board and base
 */
void showGame();

/*
 * runs games in loop until user press 'q'
 */
void runGame();

/*
 * base is moving one step down
 */
void moveOneStep();

/*
 * read from user
 */
void readSignal(int num);

/*
 * handle user's input
 */
void alarmSignal(int num);

/*
 * check whther base is in the bottom of screen
 */
int isEndOfTurn();

/*
 * user pressed 'a', moving the base left
 */
void moveLeft();

/*
 * user pressed 'd', moving the base right
 */
void moveRight();

/*
 * user pressed 's', moving the base down
 */
void moveDown();

/*
 * user pressed 'w', flip the base
 */
void flipBase();


/********************
 *
 *  Implementation
 *
 *******************/

void moveLeft() {
    if (game.base.cell_1.col != 1 && game.base.cell_3.col != 1) {
        game.base.cell_1.col--;
        game.base.cell_2.col--;
        game.base.cell_3.col--;
    }
}

void moveRight() {
    if (game.base.cell_1.col != BOARD_SIZE-2 && game.base.cell_3.col != BOARD_SIZE-2) {
        game.base.cell_1.col++;
        game.base.cell_2.col++;
        game.base.cell_3.col++;
    }
}

void moveDown() {
    if (game.base.cell_1.row != BOARD_SIZE-2 && game.base.cell_3.row != BOARD_SIZE-2) {
        game.base.cell_1.row++;
        game.base.cell_2.row++;
        game.base.cell_3.row++;
    }
}

void flipBase() {
    // base status is horizontal
    if (game.base.status == Horizontal && 0 < game.base.cell_2.row < BOARD_SIZE-1) {
        // cell_1 is a lefty
        if (game.base.cell_1.col < game.base.cell_3.col) {
            game.base.cell_1.row--;
            game.base.cell_1.col++;
            game.base.cell_3.row++;
            game.base.cell_3.col--;
        }
            // cell_3 is lefty
        else {
            game.base.cell_1.row++;
            game.base.cell_1.col--;
            game.base.cell_3.row--;
            game.base.cell_3.col++;
        }
    }
    // base status is vertical
    if (game.base.status == Vertical && 1 < game.base.cell_2.col < BOARD_SIZE-1) {
        // cell_1 is up
        if (game.base.cell_1.row < game.base.cell_3.row) {
            game.base.cell_1.row++;
            game.base.cell_1.col++;
            game.base.cell_3.row--;
            game.base.cell_3.col--;
        }
            // cell_3 is up
        else {
            game.base.cell_1.row--;
            game.base.cell_1.col--;
            game.base.cell_3.row++;
            game.base.cell_3.col++;
        }
    }
}

void readSignal(int num) {

    // pipe buffer
    char buf;
    if (read(0,&buf,sizeof(char)) < 0) { perror(error); }

    switch(buf) {
        // left
        case ('a'):
            moveLeft();
            break;
        // right
        case ('d'):
            moveRight();
            break;
        // down
        case('s'):
            moveDown();
            break;
        // flip the base
        case('w'):
            flipBase();
            break;
        // quit game
        case('q'):
            game.status = Off;
            break;

        default:
            break;
    }
    // print to console
    showGame();
}

void alarmSignal(int num) {

    moveOneStep();
    // print to console
    showGame();
    alarm(1);

}

void initBase() {

    Bcell cell1, cell2, cell3;

    game.base.status = Horizontal;
    cell1.row = cell2.row = cell3.row = 0;
    cell1.col = BOARD_SIZE/2 - 2;
    cell2.col = BOARD_SIZE/2 - 1;
    cell3.col = BOARD_SIZE/2;

    game.base.cell_1 = cell1;
    game.base.cell_2 = cell2;
    game.base.cell_3 = cell3;
}

void createBoard() {

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            // boarders
            if (j == 0 || j == BOARD_SIZE-1 || i == BOARD_SIZE - 1) {
                game.board[i][j].value = '*';
            }
                // an empty ceLL
            else {
                game.board[i][j].value = ' ';
            }
        }
    }
}

void createGame() {
    // create board
    createBoard();
    // init base
    initBase();
    // set status
    game.status = On;
}

void showGame() {

    system("clear");
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            // we are in base location
            if ((i == game.base.cell_1.row && j == game.base.cell_1.col) ||
                (i == game.base.cell_2.row && j == game.base.cell_2.col) ||
                (i == game.base.cell_3.row && j == game.base.cell_3.col)) {

                printf("%c", '-');
            }
            else {
                printf("%c", game.board[i][j].value);
            }
        }
        printf("\n");
    }
}

void moveOneStep() {

    // base reached to end of board
    if (game.base.cell_1.row == BOARD_SIZE-2 || game.base.cell_3.row == BOARD_SIZE-2) {
        initBase();
    }
    // move one step
    else {
        game.base.cell_1.row++;
        game.base.cell_2.row++;
        game.base.cell_3.row++;
    }
}

void runGame() {
    // init alarm for move one step
    alarm(1);
    // game is running
    while (game.status == On) {}
    // end of game
    close(0);
}

int main() {
    // init signals
    signal(SIGUSR2, readSignal);
    signal(SIGALRM, alarmSignal);

    createGame();
    runGame();
}




