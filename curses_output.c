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

void display_dealer(dealer_t *dealer)
{
    WINDOW *dealer_box = newwin(PLAYER_WINDOW_LINE, PLAYER_WINDOW_COLS, 0, 0);

    // build the strings to be displayed
    char *name_str = calloc(19, sizeof(char));
    char *hand_str = calloc(1, sizeof(dealer->hand));

    if (!name_str || !hand_str)
    {
        zlog_error(zc, "Memory allocation for dealer name or hand string failed.");
        goto error;
    }
    snprintf(name_str, 19, "----- %s -----", dealer->name);

    char cards[5][8];
    for (size_t card = 0; card < 5; card++)
    {
        snprintf(cards[card], sizeof(cards[card]), "%s%s",
                dealer->hand[card].rank, dealer->hand[card].suit);
    }
    snprintf(hand_str, sizeof(dealer->hand), "%s %s %s %s %s",
            ((dealer->faceup) ? " XX" : cards[0]), cards[1], cards[2], cards[3], cards[4]);

    box(dealer_box, 0, 0);
    mvwaddstr(dealer_box, 1, 1, name_str);
    mvwaddstr(dealer_box, 2, 1, hand_str);
    wrefresh(dealer_box);

//    wgetch(dealer_box);
    delwin(dealer_box);

error:
    return;
}

void display_player(player_t *player)
{
    WINDOW *player_box = newwin(PLAYER_WINDOW_LINE, PLAYER_WINDOW_COLS, 8, 0);

    // build the strings to be displayed
    char *name_str = calloc(19, sizeof(char));
    char *hnd1_str = calloc(1, sizeof(player->hand1));
    char *hnd2_str = calloc(1, sizeof(player->hand2));

    if (!name_str || !hnd1_str ||!hnd2_str)
    {
        zlog_error(zc, "Memory allocation failed for player strings.");
        goto error;
    }

    // Set up the strings
    snprintf(name_str, 19, "%-10s $%8u", player->name, player->money);

    char cards[5][8];
    for (size_t card = 0; card < 5; card++)
    {
        snprintf(cards[card], sizeof(cards[card]), "%s%s",
                player->hand1[card].rank, player->hand1[card].suit);
    }
    snprintf(hnd1_str, sizeof(player->hand1), "%s %s %s %s %s",
            cards[0], cards[1], cards[2], cards[3], cards[4]);

    for (size_t card = 0; card < 5; card++)
    {
        snprintf(cards[card], sizeof(cards[card]), "%s%s",
                player->hand2[card].rank, player->hand2[card].suit);
    }
    snprintf(hnd2_str, sizeof(player->hand2), "%s %s %s %s %s",
            cards[0], cards[1], cards[2], cards[3], cards[4]);

    box(player_box, 0, 0);
    mvwaddstr(player_box, 1, 1, name_str);
    mvwaddstr(player_box, 2, 1, hnd1_str);
    mvwaddstr(player_box, 3, 1, hnd2_str);
    wrefresh(player_box);

//    wgetch(player_box);
    delwin(player_box);

error:
    return;
}
