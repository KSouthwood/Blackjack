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

#include "../src/curses_output.h"
#include "../src/logger.h"
#include "../src/blackjack.h"
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
    
    if (init_zlog("test_curses.conf", "log")) printf(":p\n");
    
    char *msg1 = "stdscr has been initialized. Press a key to continue.\n";
    char *msg2 = "Testing welcome screen.\n";
    char *msg3 = "Testing dealer with one card down. Press a key to continue.\n";
    char *msg4 = "Testing dealer with both cards up. Press a key to continue.\n";
    char *msg5 = "Player box tested. Press a key to continue.\n";
    
    // Initialize the curses system
    zinfo("Initialize the curses system.");
    init_window();
    WINDOW *messageWindow = init_message_window();
    zinfo("Initialized curses system.");
    print_message(messageWindow, msg1);
    wgetch(stdscr);

    zinfo("Create and shuffle deck.");
    print_message(messageWindow, "Creating and shuffling deck.\n");
    Table table;
    table.shoe = init_deck(1);
    shuffle_cards(table.shoe);
    
    
    // Print the welcome screen
    zinfo("Show the welcome screen.");
    print_message(messageWindow, msg2);
    welcome_screen();
    zinfo("Welcome screen shown.");
    
    // Test the dealer window
    zinfo("Setting up dealer.");
    table.dealer = calloc(1, sizeof(Dealer));
    strncpy(table.dealer->name, "Dealer", 7);
    table.dealer->faceup = FALSE;
    table.dealer->hand.nextHand = NULL;
    table.dealer->hand.cards = calloc(1, sizeof(CardList));
    deal_card(table.shoe, &table.dealer->hand);
    deal_card(table.shoe, &table.dealer->hand);
    
    zinfo("Calling dealer window with hole card down.");
    display_dealer(table.dealer);
    print_message(messageWindow, msg3);
    wgetch(stdscr);
    
    zinfo("Calling dealer window with both cards up.");
    table.dealer->faceup = TRUE;
    display_dealer(table.dealer);
    print_message(messageWindow, msg4);
    wgetch(stdscr);
    
    zinfo("Setting up player.");
    table.players = calloc(1, sizeof(Player));
    strncpy(table.players[0].name, "Charlotte", 10);
    table.players[0].money = 99999;
    table.players[0].hand1.cards = calloc(1, sizeof(CardList));
    table.players[0].hand2.cards = calloc(1, sizeof(CardList));
    deal_card(table.shoe, &table.players[0].hand1);
    deal_card(table.shoe, &table.players[0].hand1);
    
    zinfo("Calling player window.");
    display_player(&table.players[0]);
    print_message(messageWindow, msg5);
    wgetch(stdscr);
    
    zinfo("Freeing memory allocations.");
    delwin(messageWindow);
    free(table.players);
    free(table.dealer);
    free(table.shoe->shoe);
    free(table.shoe);
    
    zinfo("Terminating curses mode.");
    end_window(table.msgWin);
    end_zlog();
    return 0;
}
