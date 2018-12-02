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
void init_window()
{
    initscr();              // Initialize ncurses mode
    cbreak();               // Make keypresses available immediately
    noecho();               // Don't echo keypresses to the screen
    curs_set(CURS_INVIS);   // Hide the cursor

    keypad(stdscr, TRUE);

    return;
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
void end_window()
{
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
void welcome_screen()
{
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
    zinfo("Displaying dealer.");
    WINDOW *dealerWindow = newwin(PLAYER_WINDOW_LINE, PLAYER_WINDOW_COLS, 0, 0);    // TODO change to variable coordinates

    // build the strings to be displayed
    char *nameString = calloc(19, sizeof(char));
    char *handString = calloc(1, 50); // TODO: Calculate???? instead of magic number

    if (!nameString || !handString)
    {
        zerror("Memory allocation for dealer name or hand string failed.");
        goto error;
    }
    
    snprintf(nameString, 19, "----- %s -----", dealer->name);

    CardList *printCard = dealer->hand.cards;
    if (printCard->card != NULL)
    {
//        hand_to_string(handString, dealer->faceup);
        if (dealer->faceup == FALSE)
        {
            strncat(handString, "XXX ", 4);
            printCard = printCard->nextCard;
        }

        while (printCard != NULL)
        {
            strncat(handString, printCard->card->face, 6);
            strncat(handString, " ", 1);
            printCard = printCard->nextCard;
        }
    }

    box(dealerWindow, 0, 0);
    mvwaddstr(dealerWindow, 1, 1, nameString);
    mvwaddstr(dealerWindow, 2, 1, handString);
    wrefresh(dealerWindow);
    delwin(dealerWindow);

error:
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
    zinfo("Displaying player %s.", player->name);
    WINDOW *playerWindow = newwin(PLAYER_WINDOW_LINE, PLAYER_WINDOW_COLS, 8, 0);    // TODO change to variable coordinates
    // build the strings to be displayed
    char *nameString = calloc(19, sizeof(char));
    char *hand1String = calloc(1, 50);
    char *hand2String = calloc(1, 50);

    if (!nameString || !hand1String || !hand2String)
    {
        zerror("Memory allocation failed for player strings.");
        goto error;
    }

    // Set up the strings
    snprintf(nameString, 19, "%-10s $%6u", player->name, player->money);

    CardList *printCard1 = player->hand1.cards;
    if (printCard1->card != NULL)
    {
        while (printCard1 != NULL)
        {
            strncat(hand1String, printCard1->card->face, 6);
            strncat(hand1String, " ", 1);
            printCard1 = printCard1->nextCard;
        }
    }
    
    CardList *printCard2 = player->hand2.cards;
    if (printCard2->card != NULL)
    {
        while (printCard2 != NULL)
        {
            strncat(hand2String, printCard2->card->face, 6);
            strncat(hand2String, " ", 1);
            printCard2 = printCard2->nextCard;
        }
    }
    
    box(playerWindow, 0, 0);
    mvwaddstr(playerWindow, 1, 1, nameString);
    mvwaddstr(playerWindow, 2, 1, hand1String);
    mvwaddstr(playerWindow, 3, 1, hand2String);

    wrefresh(playerWindow);
    delwin(playerWindow);

error:
    free(nameString);
    free(hand1String);
    free(hand2String);
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
PlayerChoice get_player_choice(Player *player, WINDOW* msgWin)
{
    bool choiceMade = FALSE;
    PlayerChoice choice;
    char input;
    char msg[80];
    
    snprintf(msg, sizeof(msg), "%s: [S]tand, [H]it, [D]ouble down or S[p]lit? ", player->name);
    print_message(msgWin, msg);
    echo();
    
    while (!choiceMade)
    {
        zinfo("Ask for players choice.");
        choiceMade = TRUE;
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
                choice = DOUBLE;
                print_message(msgWin, "Double down\n");
                zinfo("Player chose DOUBLE.");
                break;
            case 'p':
            case 'P':
                choice = SPLIT;
                print_message(msgWin, "Split\n");
                zinfo("Player chose SPLIT.");
                break;
            default:
                choiceMade = FALSE;
        }
    }
    
    noecho();
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
WINDOW *init_message_window()
{
    zinfo("init_message_window() called.");
    uint16_t columns, lines;        // width and height of the stdscr window
    getmaxyx(stdscr, lines, columns);
    
    // set up dimensions of our window
    uint16_t msgY = lines - 12;
    uint16_t msgX = 2;
    uint16_t msgLines = 10;
    uint16_t msgColumns = columns - 4;
    
    // create window to display messages in and window for the border
    WINDOW *msgWindow = newwin(msgLines, msgColumns, msgY, msgX);
    zinfo("msgWindow pointer: %p", msgWindow);
    wmove(msgWindow, 9, 0);
    
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
    wmove(msgWindow, 9, 0);
    waddstr(msgWindow, msg);
    wrefresh(msgWindow);

    return;
}
