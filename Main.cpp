//|***************************|
//|TO DO:                     | 
//|ZMIENIĆ DZIAŁAJĄCY SZKIELET|
//|NA OOP UZYWAJAC KLAS       |
//|***************************|

#include <ncursesw/curses.h>
#include <math.h>

// XMAX and YMAX must be multiples of 3.
#define XMAX 33
#define YMAX 18

// It contains the shape of each chip.
static char chipX[] = {1, 1, 1, 0, 0, 1, 1, 1,
                       0, 0, 1, 1, 1, 1, 0, 0,
                       0, 0, 0, 1, 1, 0, 0, 0,
                       0, 0, 1, 1, 1, 1, 0, 0,
                       1, 1, 1, 0, 0, 1, 1, 1
};

static char chipO[] = {0, 0, 1, 1, 1, 1, 0, 0,
                       1, 1, 1, 0, 0, 1, 1, 1,
                       1, 1, 0, 0, 0, 0, 1, 1,
                       1, 1, 1, 0, 0, 1, 1, 1,
                       0, 0, 1, 1, 1, 1, 0, 0
};

typedef struct {
    int x;
    int y;
    int lastChip[2];           // Check if the chip is in the same place.
    int player;                // Actual player.
    int moves;                 // Number of moves.
    int move;                  // Is moving any chip?
    int gameChips[2];          // Number of chips in the game for each player.
    char chips[3][3];          // Chips of the gameboard. (0 -> Empty / 1 -> X / 2 -> O)
} board;

void clearChips(board *gameBoard) {     // Clean chips
    int i, j;

    for (i=0; i<3; i++)
        for (j=0; j<3; j++)
            gameBoard->chips[i][j] = 0;
}

void drawBoard(board *gameBoard) {    // Draw gameboard
    int i, j;

    for(i=0; i<=YMAX; i++)
        for(j=0; j<=XMAX; j++) {
            if (i == 0) { // First row of the gameboard.
                if (j == 0)              // Upper-Left corner.
                    mvaddch(gameBoard->y+i, gameBoard->x+j, ACS_ULCORNER);
                else if (j == XMAX)     // Upper-Right corner.
                    mvaddch(gameBoard->y+i, gameBoard->x+j, ACS_URCORNER);
                else if (j % (XMAX / 3) == 0)   // Middle column.
                    mvaddch(gameBoard->y+i, gameBoard->x+j, ACS_TTEE);
                else                    // Normal lines.
                    mvaddch(gameBoard->y+i, gameBoard->x+j, ACS_HLINE);
            }

            else if (i % (YMAX / 3) == 0 && i != YMAX) { // Dividers.
                if (j == 0)             // First column.
                    mvaddch(gameBoard->y+i, gameBoard->x+j, ACS_LTEE);
                else if (j == XMAX)     // Last column.
                    mvaddch(gameBoard->y+i, gameBoard->x+j, ACS_RTEE);
                else if (j % (XMAX / 3) == 0)   // Middle column.
                    mvaddch(gameBoard->y+i, gameBoard->x+j, ACS_PLUS);
                else                    // Normal lines.
                    mvaddch(gameBoard->y+i, gameBoard->x+j, ACS_HLINE);
            }

            else if (i == YMAX) {           // Last row
                if (j == 0)              // Lower-Left corner.
                    mvaddch(gameBoard->y+i, gameBoard->x+j, ACS_LLCORNER);
                else if (j == XMAX)     // Lower-Right corner.
                    mvaddch(gameBoard->y+i, gameBoard->x+j, ACS_LRCORNER);
                else if (j % (XMAX / 3) == 0)   // Middle column.
                    mvaddch(gameBoard->y+i, gameBoard->x+j, ACS_BTEE);
                else                    // Normal lines.
                    mvaddch(gameBoard->y+i, gameBoard->x+j, ACS_HLINE);
            }

            else if (j % (XMAX / 3) == 0) { // Middle lines.
                mvaddch(gameBoard->y+i, gameBoard->x+j, ACS_VLINE);
            }
        }
}

/***************************************
 * Draw a chip in the gameboard.
 * gameBoard -> Contains the information of the gameboard.
 * dest -> Matrix that contains the destiny [0]=y [1]=x
 * type -> It can be 'x', 'o' or any character (the last option cleans the chip)
 **************************************/
void drawChip(int *dest, char type, board *gameBoard) {
    int i, j;
    int y, x;

    for (i=0; i<5; i++)     // Chip height
        for (j=0; j<8; j++) {   // Chip width
            y = YMAX/3 * dest[0] + i + gameBoard->y + 1;
            x = XMAX/3 * dest[1] + j + gameBoard->x + 2;
            if (chipX[j+8*i] == 1 && type == 'x') {        // If X chip...
                attron(COLOR_PAIR(1));
                mvaddch(y, x, '#');
                attroff(COLOR_PAIR(1));
            }

            else if (chipO[j+8*i] == 1 && type == 'o') {   // If O chip...
                attron(COLOR_PAIR(2));
                mvaddch(y, x, '#');
                attroff(COLOR_PAIR(2));
            }

            else {  // Print spaces.
                mvaddch(y, x, ' ');
            }
        }
}

/*********************************************
 *          Function "putChip"
 * Return values:
 * 0 -> Correct.
 * 1 -> The player is trying to remove opponent's chips when he has to pick his own chips.
 * 2 -> The cell where the player wants to put the chip is taken.
 * 3 -> The player has already taken a chip.
 * 4 -> The player has put all his chips in the game, he has to pick his own chips.
 * 5 -> The player tries to put the chip in the same place that it was removed.
 ********************************************/
int placeChip(int *dest, board *TBoard) { // Put chip
    if (TBoard->chips[dest[0]][dest[1]] != 0) { // If the cell is taken by some chip...
        // If the player put all chips in the game but wants to remove one from the gameboard...
        if (TBoard->move == 0) {
            if (TBoard->gameChips[TBoard->player - 1] == 3 && TBoard->chips[dest[0]][dest[1]] == TBoard->player) {
                TBoard->lastChip[0] = dest[0];
                TBoard->lastChip[1] = dest[1];
                drawChip(dest, ' ', TBoard);
                TBoard->chips[dest[0]][dest[1]] = 0;
                TBoard->gameChips[TBoard->player - 1]--;
                TBoard->move = 1;
                return 6; // Do nothing.
            } else {
                if (TBoard->gameChips[TBoard->player - 1] == 3)
                    return 1;
                else
                    return 2;
            }
        } else {
            if (TBoard->chips[dest[0]][dest[1]] == TBoard->player) {
                return 3;
            } else {
                return 2;
            }
        }
    }
        // If the player is going to put a chip...
    else if (dest[0] != TBoard->lastChip[0] || dest[1] != TBoard->lastChip[1]) {
        if (TBoard->gameChips[TBoard->player - 1] < 3) { // If the actual player hasn't put all chips in the game...
            if (TBoard->player == 1) {

                drawChip(dest, 'x', TBoard); // Draw the chip.

                TBoard->chips[dest[0]][dest[1]] = TBoard->player; // Put the chip in the gameboard matrix.
                TBoard->gameChips[TBoard->player - 1]++; // Adds a chip to the player's variable.
                TBoard->player = 2; // Change the actual player.
            } else if (TBoard->player == 2) {

                drawChip(dest, 'o', TBoard);

                TBoard->chips[dest[0]][dest[1]] = TBoard->player;
                TBoard->gameChips[TBoard->player - 1]++;
                TBoard->player = 1;
            }

            // No need to check the latest position of the chip.
            TBoard->move = 0;
            TBoard->lastChip[0] = 3;
            TBoard->lastChip[1] = 3;
        } else {
            return 4;
        }
    } else {
        return 5;
    }

    return 0; // All correct.
}

void placeCursor(int *orig, int *dest, board *TBoard) { // Put cursor
    int i, j;
    int y, x;

    for (i = 0; i < 5; i++) {
        // Clean the last cursor
        y = YMAX / 3 * orig[0] + i + TBoard->y + 1;
        x = XMAX / 3 * orig[1] + TBoard->x + 1;
        mvaddch(y, x, ' ');
        x = x + 9;
        mvaddch(y, x, ' ');

        // Set the new cursor
        y = YMAX / 3 * dest[0] + i + TBoard->y + 1;
        x = XMAX / 3 * dest[1] + TBoard->x + 1;

        attron(COLOR_PAIR(3) | A_BLINK);
        mvaddch(y, x, ACS_DIAMOND);
        x = x + 9;
        mvaddch(y, x, ACS_DIAMOND);
        attroff(COLOR_PAIR(3) | A_BLINK);
    }
}

int checkWinner(board *TBoard) { // Check winner
    int i;
    int temp;

    // Upper-Left to Bottom-Right.
    temp = TBoard->chips[0][0];
    for (i = 1; i < 3; i++)
        if (TBoard->chips[i][i] != temp || TBoard->chips[i][i] == 0) // If the next chip NOT equals...
            temp = 4; // To avoid overlapping with empty cells or players chips.
        else if (i == 2)
            return TBoard->chips[i][i]; // Returns the winner.

    // Bottom-Left to Upper-Right.
    temp = TBoard->chips[2][0];
    for (i = 1; i < 3; i++)
        if (TBoard->chips[2 - i][i] != temp || TBoard->chips[2 - i][i] == 0)
            temp = 4;
        else if (i == 2)
            return TBoard->chips[2 - i][i];

    // First row.
    temp = TBoard->chips[0][0];
    for (i = 1; i < 3; i++)
        if (TBoard->chips[0][i] != temp || TBoard->chips[0][i] == 0)
            temp = 4;
        else if (i == 2)
            return TBoard->chips[0][i];

    // Second row.
    temp = TBoard->chips[1][0];
    for (i = 1; i < 3; i++)
        if (TBoard->chips[1][i] != temp || TBoard->chips[1][i] == 0)
            temp = 4;
        else if (i == 2)
            return TBoard->chips[1][i];

    // Third row.
    temp = TBoard->chips[2][0];
    for (i = 1; i < 3; i++)
        if (TBoard->chips[2][i] != temp || TBoard->chips[2][i] == 0)
            temp = 4;
        else if (i == 2)
            return TBoard->chips[2][i];

    // First column.
    temp = TBoard->chips[0][0];
    for (i = 1; i < 3; i++)
        if (TBoard->chips[i][0] != temp || TBoard->chips[i][0] == 0)
            temp = 4;
        else if (i == 2)
            return TBoard->chips[i][0];

    // Second column.
    temp = TBoard->chips[0][1];
    for (i = 1; i < 3; i++)
        if (TBoard->chips[i][1] != temp || TBoard->chips[i][1] == 0)
            temp = 4;
        else if (i == 2)
            return TBoard->chips[i][1];

    // Third column.
    temp = TBoard->chips[0][2];
    for (i = 1; i < 3; i++)
        if (TBoard->chips[i][2] != temp || TBoard->chips[i][2] == 0)
            temp = 4;
        else if (i == 2)
            return TBoard->chips[i][2];

    return 0; // There isn't any winner.
}

void printGameMessages(board *TBoard) { // Print game messages
    mvprintw(5, 4, "      PLAYER");
    mvprintw(6, 10, "PLAYER %d TURN", TBoard->player);

    mvprintw(5, 63, "NUMBER OF");
    mvprintw(6, 61, "  MOVES: %d", TBoard->moves);
}

int mainGame(board *TBoard) { // Main game function
    int key = 0;
    int winner = 0;
    int checkMove = 0;
    int originCursor[] = {0, 0};
    int destCursor[] = {0, 0};

    placeCursor(originCursor, destCursor, TBoard);

    while (key != '0' && winner == 0) { // Loop until press 0 key or game ends.
        printGameMessages(TBoard);
        key = getch();
        switch (key) {
            case KEY_LEFT:
                if (originCursor[1] > 0) {
                    destCursor[1] = originCursor[1] - 1;
                    placeCursor(originCursor, destCursor, TBoard);
                    originCursor[1] = destCursor[1];
                }
                break;

            case KEY_RIGHT:
                if (originCursor[1] < 2) {
                    destCursor[1] = originCursor[1] + 1;
                    placeCursor(originCursor, destCursor, TBoard);
                    originCursor[1] = destCursor[1];
                }
                break;

            case KEY_UP:
                if (originCursor[0] > 0) {
                    destCursor[0] = originCursor[0] - 1;
                    placeCursor(originCursor, destCursor, TBoard);
                    originCursor[0] = destCursor[0];
                }
                break;

            case KEY_DOWN:
                if (originCursor[0] < 2) {
                    destCursor[0] = originCursor[0] + 1;
                    placeCursor(originCursor, destCursor, TBoard);
                    originCursor[0] = destCursor[0];
                }
                break;

            case ' ':
                move(23, 0);
                deleteln();
                checkMove = placeChip(destCursor, TBoard);
                switch (checkMove) {
                    case 0:
                        winner = checkWinner(TBoard);
                        TBoard->moves++;
                        break;
                    case 1:
                        mvprintw(23, (80 - 41) / 2, "You cannot pick an opponent's chip.");
                        break;
                    case 2:
                        mvprintw(23, (80 - 48) / 2, "Cannot place a chip on an occupied cell.");
                        break;
                    case 3:
                        mvprintw(23, (80 - 55) / 2, "You have already picked a chip. Place it in an empty cell.");
                        break;
                    case 4:
                        mvprintw(23, (80 - 56) / 2, "You have now placed all your chips in the game. Move a chip.");
                        break;
                    case 5:
                        mvprintw(23, (80 - 47) / 2, "Cannot restore the chip to its previous state.");
                        break;
                }
                break;
        }
    }

    return winner;
}

int main() {
    int key;     // Pressed key
    int exitGame = 0; // Exit
    int winner;  // Winner
    board TBoard;

    // Centering the gameboard on the screen.
    TBoard.x = (80 - XMAX) / 2;
    TBoard.y = 3;

    initscr();
    start_color();
    noecho();
    cbreak;
    curs_set(FALSE); // Disable the console cursor.
    keypad(stdscr, TRUE); // Enable the keyboard arrows.

    init_pair(0, COLOR_WHITE, COLOR_BLACK);
    init_pair(1, COLOR_GREEN, COLOR_BLACK); // Player 1 color.
    init_pair(2, COLOR_YELLOW, COLOR_BLACK); // Player 2 color.
    init_pair(3, COLOR_BLUE, COLOR_BLACK); // Cursor.
    init_pair(4, COLOR_BLACK, COLOR_CYAN); // Winner text

    while (!exitGame) {
        key = 0;
        winner = 0;
        TBoard.player = 1;
        TBoard.lastChip[0] = 3; // The last moved chip is outside from gameboard to check it after.
        TBoard.lastChip[1] = 3; // ""
        TBoard.gameChips[0] = 0;
        TBoard.gameChips[1] = 0;
        TBoard.moves = 0;

        clear();
        mvprintw(1, (80 - 9) / 2, "");

        clearChips(&TBoard); // Clean the chip matrix inside struct TBoard (Gameboard).
        drawBoard(&TBoard); // Draw the gameboard.
        winner = mainGame(&TBoard); // Begins the game.

        if (winner != 0) {
            attron(COLOR_PAIR(4));
            mvprintw(10, (80 - 21) / 2, "CONGRATULATIONS PLAYER %d", winner);
            attron(A_BLINK);
            mvprintw(11, (80 - 12) / 2, "YOU WON!");
            attroff(A_BLINK);
            mvprintw(12, (80 - 29) / 2, "WANNA PLAY AGAIN? (y/n)");
            attroff(COLOR_PAIR(4));

            while (key != 'n' && key != 's') {
                key = getch();
                if (key == 'n')
                    exitGame = 1;
            }
        } else {
            exitGame = 1;
        }
    }

    resetty();
    endwin();

    return 0;
