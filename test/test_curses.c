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
 *  test_curses.c
 *
 *  Created on: Nov 9, 2018
 *      Author: Keri Southwood-Smith
 *
 *  Description: Test suite for the Blackjack programs curses routines.
 */

/************
 * INCLUDES *
 ************/
#include <stdlib.h>
#include <locale.h>

#include "../src/blackjack.h"
#include "../src/logger.h"
#include "../src/curses_output.h"
#include "../src/deck_of_cards.h"

/***********
 * DEFINES *
 ***********/
#define CENTER(message) ((columns - strlen(message)) / 2)

/****************
 * DECLARATIONS *
 ****************/


int main(void)
{
    setlocale(LC_ALL, "");
    srand(1968);
    
    uint16_t lines, columns;
    
    if (init_zlog("test_curses.conf", "log")) printf(":p\n");

    char *msg1 = "stdscr has been initialized. Press a key to continue.";
    char *msg2 = "Testing welcome screen.";
    char *msg3 = "Testing dealer with one card down. Press a key to continue.";
    char *msg4 = "Testing dealer with both cards up. Press a key to continue.";
    char *msg5 = "Player box tested. Press a key to continue.";
    
    zinfo("Create and shuffle deck.");
    Deck shoe = *init_deck(1);
    shuffle_cards(&shoe);
    
    // Initialize the curses system
    zinfo("Initialize the curses system.");
    init_window();
    getmaxyx(stdscr, lines, columns);
    mvwaddstr(stdscr, 1, CENTER(msg1), msg1);
    zinfo("Initialized curses system.");
    wgetch(stdscr);
    
    // Print the welcome screen
    zinfo("Show the welcome screen.");
    mvwaddstr(stdscr, 2, CENTER(msg2), msg2);
    welcome_screen();
    zinfo("Welcome screen shown.");
    
    // Test the dealer window
    zinfo("Setting up dealer.");
    Dealer *dealer = calloc(1, sizeof(Dealer));
    dealer->name = "Dealer";
    dealer->faceup = FALSE;
    dealer->hand[0] = deal_card(&shoe);
    dealer->hand[1] = deal_card(&shoe);
    
    zinfo("Calling dealer window with hole card down.");
    display_dealer(dealer);
    mvwaddstr(stdscr, lines - 5, CENTER(msg3), msg3);
    wgetch(stdscr);
    
    zinfo("Calling dealer window with both cards up.");
    dealer->faceup = TRUE;
    display_dealer(dealer);
    mvwaddstr(stdscr, lines - 4, CENTER(msg4), msg4);
    wgetch(stdscr);
    
    zinfo("Setting up player.");
    Player *player = calloc(1, sizeof(Player));
    strcpy(player->name, "Charlotte");
    player->money = 99999;
    player->hand1[0] = deal_card(&shoe);
    player->hand1[1] = deal_card(&shoe);
    player->hand2[0] = deal_card(&shoe);
    player->hand2[1] = deal_card(&shoe);
    
    zinfo("Calline player window.");
    display_player(player);
    mvwaddstr(stdscr, lines - 3, CENTER(msg5), msg5);
    wgetch(stdscr);
    
    zinfo("Terminating curses mode.");
    end_window();
    end_zlog();
    return 0;
}
