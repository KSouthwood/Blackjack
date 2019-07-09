/***********************************************************************************
 *  MIT License                                                                    *
 *                                                                                 *
 *  Copyright (c) 2018 Keri Southwood-Smith                                        *
 *                                                                                 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy   *
 *  of this software and associated documentation files (the "Software"), to deal  *
 *  in the Software without restriction, including without limitation the rights   *
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      *
 *  copies of the Software, and to permit persons to whom the Software is          *
 *  furnished to do so, subject to the following conditions:                       *
 *                                                                                 *
 *  The above copyright notice and this permission notice shall be included in all *
 *  copies or substantial portions of the Software.                                *
 *                                                                                 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     *
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       *
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    *
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         *
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  *
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  *
 *  SOFTWARE.                                                                      *
 ***********************************************************************************/

/*
 *  curses_output.c
 *
 *  Created on: Oct 17, 2018
 *      Author: Keri Southwood-Smith
 *
 *  Description:
 */


/************
 * INCLUDES *
 ************/
#include "curses_output.h"

/****************
 * DECLARATIONS *
 ****************/

/***************
 *  Summary: Start ncurses
 *
 *  Description: Initialize ncurses mode and set the options we need.
 *
 *  Parameter(s):
 *      N/A
 *
 *  Returns:
 *      N/A
 */
bool init_window(void)
{
    log_call();
    initscr();              // Initialize ncurses mode
    cbreak();               // Make keypresses available immediately
    noecho();               // Don't echo keypresses to the screen
    curs_set(CURS_INVIS);   // Hide the cursor
    keypad(stdscr, TRUE);   // Enable keypad

    if ((COLS < 80) || (LINES < 40))
    {
        endwin();
        printf("The terminal needs to be at least 80 x 40 in order to play.\n");
        return false;
    }

    return true;
}

/***************
 *  Summary: Ends ncurses mode
 *
 *  Description: Call the endwin() routine to stop ncurses
 *
 *  Parameter(s):
 *      N/A
 *
 *  Returns:
 *      N/A
 */
void end_window(void)
{
    log_call();
    //TODO remove block when program is done or play_game loop is sufficiently finished
    uint16_t columns, lines;
    getmaxyx(stdscr, lines, columns);
    mvwaddstr(stdscr, lines - 2, (columns - 35) / 2, "(Game over. Press any key to exit.)");
    wgetch(stdscr);
    endwin();

    return;
}

/***************
 *  Summary: Display the welcome screen for the game
 *
 *  Description: Print the welcome screen and instructions, then wait for a keypress to continue.
 *
 *  Parameter(s):
 *      N/A
 *
 *  Returns:
 *      N/A
 */
void welcome_screen(void)
{
    log_call();
    uint16_t columns, lines;    // the size of stdscr window
    uint8_t win_cols = 40;      // the width of our window
    uint8_t win_lines = 20;     // the height of our window

    getmaxyx(stdscr, lines, columns);

    // calculate starting position for the window
    uint8_t begin_x = (columns - win_cols) / 2;
    uint8_t begin_y = (lines - win_lines) / 2;

    WINDOW *welcome = newwin(win_lines, win_cols, begin_y, begin_x);

    box(welcome, 0, 0); // draw a border around our window

    // print the welcome message
    mvwaddstr(welcome, 1, 5, "*** WELCOME TO BLACKJACK ***");
    mvwaddstr(welcome, 3, 1, "Time to play cards to 21 without going");
    mvwaddstr(welcome, 4, 1, "over. Go up against the house to win");
    mvwaddstr(welcome, 5, 1, "money. You'll get $1,000 to start with");
    mvwaddstr(welcome, 6, 1, "in your quest to earn more. Lose it");
    mvwaddstr(welcome, 7, 1, "all and your game is over. Good luck!");
    mvwaddstr(welcome, 18, 7, "(Press a key to continue.)");
    wrefresh(welcome);

    // wait for a keypress before continuing and removing the window
    wgetch(welcome);
    wclear(welcome);
    wrefresh(welcome);
    delwin(welcome);

    return;
}

/***************
 *  Summary: Display the dealer's hand in its own window
 *
 *  Description: Print the dealer's name and hand in a bordered window on the screen
 *
 *  Parameter(s):
 *      dealer: Dealer struct with the dealer's information
 *
 *  Returns:
 *      N/A
 */
void display_dealer(Dealer *dealer)
{
    log_call();
    werase(dealer->window);
    int winCols = getmaxx(dealer->window);

    // build the strings to be displayed
    char *nameString = calloc(19, sizeof(char));
    char *statString = calloc(19, sizeof(char));
    char *handString = calloc(1, 50); // TODO: Calculate???? instead of magic number

    if (!nameString || !handString || !statString)
    {
        zerror("Memory allocation for dealer name, stat string or hand string failed.");
        goto error;
    }

    snprintf(nameString, 9, " %s ", dealer->name);
//    snprintf(statString, 19, "Count: %u", blackjack_count(dealer->hand));

    hand_to_string(&dealer->hand, handString, dealer->faceup);

    box(dealer->window, 0, 0);
    mvwaddstr(dealer->window, 0, ((winCols / 2) - (strlen(nameString) / 2)), nameString);
    mvwaddstr(dealer->window, 2, 1, handString);
    wrefresh(dealer->window);

error:
    free(statString);
    free(nameString);
    free(handString);
    return;
}

/***************
 *  Summary: Display the player's hand in its own window
 *
 *  Description: Print the player's name, money and hand in a bordered window on the screen
 *
 *  Parameter(s):
 *      player: Player struct with the player's information
 *
 *  Returns:
 *      N/A
 */
void display_player(Player *player)
{
    log_call();
    zinfo("Displaying player %s.", player->name);

    werase(player->window);
    int winCols = getmaxx(player->window);

    // build the strings to be displayed
    char *nameString = calloc(19, sizeof(char));
    char *statString = calloc(19, sizeof(char));
    char *handString = calloc(1, 50);

    if (!nameString || !handString)
    {
        zerror("Memory allocation failed for player strings.");
        goto error;
    }

    // Set up the strings
    snprintf(nameString, 17, " %s ", player->name);
    snprintf(statString, 19, "        $%'9u", player->money);
    box(player->window, 0, 0);
    mvwaddstr(player->window, 0, ((winCols / 2) - (strlen(nameString) / 2)), nameString);
    mvwaddstr(player->window, 1, winCols - 19 - 1, statString);

    Hand *handToPrint = &player->hand;
    uint8_t lineToPrint = 2;
    while (handToPrint != NULL)
    {
        hand_to_string(handToPrint, handString, TRUE);
        mvwaddstr(player->window, lineToPrint++, 1, handString);
        handToPrint = handToPrint->nextHand;
        strcpy(handString, "");
    }

    wrefresh(player->window);

error:
    free(statString);
    free(nameString);
    free(handString);
    return;
}

/***************
 *  Summary: Get a choice from the player on how to play their hand
 *
 *  Description: Ask the player how they want to play their hand and return that to the calling routine. Uses an enum
 *      as the return values.
 *
 *  Parameter(s):
 *      player: Player struct with the player's information
 *
 *  Returns:
 *      N/A
 */
PlayerChoice get_player_choice(char *name, Hand *hand, WINDOW* msgWin)
{
    log_call();
    bool choiceMade = false;
    bool doubleValid = false;
    bool splitValid = false;
    PlayerChoice choice;
    char input;
    char msg[80];

    // build message string of valid choices
    char defaultChoices[15] = "[S]tand, [H]it";
    char doubleDown[16] = ", [D]ouble down";
    char split[10] = ", S[p]lit";

    if (hand->numCards == 2)
    {
        doubleValid = true;
        if (hand->cards[0].value == hand->cards[1].value)
        {
            splitValid = true;
        }
        else    // the two cards are not the same value so splitting is not an option
        {
            zdebug("Have two cards not same value, setting split message to NULL.");
            split[0] = '\0';
        }
    }
    else
    {
        // we have more than two cards so neither choice is valid
        zdebug("Have more than two cards, setting double and split to null");
        doubleDown[0] = '\0';
        split[0] = '\0';
    }

    snprintf(msg, sizeof(msg), "%s: %s%s%s? ", name, defaultChoices, doubleDown, split);
    print_message(msgWin, msg);

    while (!choiceMade)
    {
        zinfo("Ask for players choice.");
        choiceMade = true;
        input = wgetch(stdscr);
        switch(input)
        {
            case 's':
            case 'S':
                choice = STAND;
                print_message(msgWin, "Stand\n");
                zinfo("Player chose STAND.");
                break;
            case 'h':
            case 'H':
                choice = HIT;
                print_message(msgWin, "Hit\n");
                zinfo("Player chose HIT.");
                break;
            case 'd':
            case 'D':
                if (doubleValid)
                {
                    choice = DOUBLE;
                    print_message(msgWin, "Double down\n");
                    zinfo("Player chose DOUBLE.");
                }
                else
                {
                    choiceMade = false;
                }
                break;
            case 'p':
            case 'P':
                if (splitValid)
                {
                    choice = SPLIT;
                    print_message(msgWin, "Split\n");
                    zinfo("Player chose SPLIT.");
                }
                else
                {
                    choiceMade = false;
                }
                break;
            default:
                choiceMade = false;
        }
    }

    return choice;
}

/***************
 *  Summary: Instantiate the message window
 *
 *  Description: Create and return a WINDOW pointer to a curses window for use to print messages.
 *
 *  Parameter(s):
 *      N/A
 *
 *  Returns:
 *      N/A
 */
WINDOW *init_message_window(void)
{
    log_call();
    uint16_t columns, lines;        // width and height of the stdscr window
    getmaxyx(stdscr, lines, columns);

    // set up dimensions of our window
    uint16_t msgY = lines - ((lines < 30) ? 7 : 20);
    uint16_t msgX = 2;
    uint16_t msgLines = (lines < 30) ? 5 : 10;
    uint16_t msgColumns = columns - 4;

    // create window to display messages in and window for the border
    WINDOW *msgWindow = newwin(msgLines, msgColumns, msgY, msgX);
    zinfo("msgWindow pointer: %p", msgWindow);
    wmove(msgWindow, (lines - 1), 0);

    return msgWindow;
}

/***************
 *  Summary: Display a message to the screen in a special window
 *
 *  Description: Display a message to the screen in a dedicated window with a border around it. The lines scroll from
 *      the bottom to the top with the message passed in printed at the bottom.
 *
 *  Parameter(s):
 *      msgWindow:  WINDOW pointer to the curses window the message gets printed to
 *      msg:        pointer to a char string to be printed
 *
 *  Returns:
 *      N/A
 */
void print_message(WINDOW *msgWindow, char *msg)
{
    log_call();
    uint16_t maxY, maxX, begY, begX;
    getbegyx(msgWindow, begY, begX);
    getmaxyx(msgWindow, maxY, maxX);

    // draw a border around the message window
    WINDOW *boxwin = newwin(maxY + 4, maxX + 4, begY - 2, begX - 2);
    box(boxwin, 0, 0);
    wrefresh(boxwin);
    delwin(boxwin);

    // delete the topmost line moving the other lines up and move the cursor to the bottom line
    touchwin(msgWindow);
    wmove(msgWindow, 0, 0);
    wdeleteln(msgWindow);
    wmove(msgWindow, (maxY - 1), 0);
    waddstr(msgWindow, msg);
    wrefresh(msgWindow);

    return;
}

/***************
 *  Summary:
 *
 *  Description:
 *
 *  Parameter(s):
 *      N/A
 *
 *  Returns:
 *      N/A
 */

void hand_to_string(Hand *hand, char *handString, bool showCard)
{
    log_call();
    for (uint8_t card = 0; card < hand->numCards; card++)
    {
        if ((card == 0) && (showCard == FALSE))
        {
            strncat(handString, "XXX ", 5);
            continue;
        }

        strncat(handString, hand->cards[card].face, 6);
        strncat(handString, " ", 2);
    }
    return;
}