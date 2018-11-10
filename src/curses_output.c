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
    //TODO remove block when program is done or play_game loop is sufficently finished
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
    WINDOW *dealerWindow = newwin(PLAYER_WINDOW_LINE, PLAYER_WINDOW_COLS, 0, 0);    // TODO change to variable coordinates

    // build the strings to be displayed
    char *nameString = calloc(19, sizeof(char));
    char *handString = calloc(1, sizeof(dealer->hand));

    if (!nameString || !handString)
    {
        zerror("Memory allocation for dealer name or hand string failed.");
        goto error;
    }
    snprintf(nameString, 19, "----- %s -----", dealer->name);

    snprintf(handString, sizeof(dealer->hand), "%s %s %s %s %s",
            ((dealer->faceup) ? dealer->hand[0].face : " XX"), dealer->hand[1].face, dealer->hand[2].face,
            dealer->hand[3].face, dealer->hand[4].face);

    box(dealerWindow, 0, 0);
    mvwaddstr(dealerWindow, 1, 1, nameString);
    mvwaddstr(dealerWindow, 2, 1, handString);
    wrefresh(dealerWindow);
    delwin(dealerWindow);

error:
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
    WINDOW *playerWindow = newwin(PLAYER_WINDOW_LINE, PLAYER_WINDOW_COLS, 8, 0);    // TODO change to variable coordinates

    // build the strings to be displayed
    char *nameString = calloc(19, sizeof(char));
    char *hand1String = calloc(1, sizeof(player->hand1));
    char *hand2String = calloc(1, sizeof(player->hand2));

    if (!nameString || !hand1String || !hand2String)
    {
        zerror("Memory allocation failed for player strings.");
        goto error;
    }

    // Set up the strings
    snprintf(nameString, 19, "%-10s $%6u", player->name, player->money);

    snprintf(hand1String, sizeof(player->hand1), "%s %s %s %s %s",
            player->hand1[0].face, player->hand1[1].face, player->hand1[2].face, player->hand1[3].face, player->hand1[4].face);

    snprintf(hand2String, sizeof(player->hand2), "%s %s %s %s %s",
            player->hand2[0].face, player->hand2[1].face, player->hand2[2].face, player->hand2[3].face, player->hand2[4].face);

    box(playerWindow, 0, 0);
    mvwaddstr(playerWindow, 1, 1, nameString);
    mvwaddstr(playerWindow, 2, 1, hand1String);
    mvwaddstr(playerWindow, 3, 1, hand2String);
    wrefresh(playerWindow);

    delwin(playerWindow);

error:
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
PlayerChoice get_player_choice(Player *player)
{
    bool choiceMade = FALSE;
    PlayerChoice choice;
    char input;
    
    mvwaddstr(stdscr, 15, 0, "[S]tand, [H]it, [D]ouble down or S[p]lit? ");
    echo();
    
    while (!choiceMade)
    {
        choiceMade = TRUE;
        input = wgetch(stdscr);
        switch(input)
        {
            case 's':
            case 'S':
                choice = STAND;
                break;
            case 'h':
            case 'H':
                choice = HIT;
                break;
            case 'd':
            case 'D':
                choice = DOUBLE;
                break;
            case 'p':
            case 'P':
                choice = SPLIT;
                break;
            default:
                choiceMade = FALSE;
        }
    }
    
    noecho();
    
    return choice;
}