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
 *  blackjack_test.c
 *
 *  Created on: Oct 11, 2018
 *      Author: Keri Southwood-Smith
 *
 *  Description: Test suite for the Blackjack program.
 */


/************
 * INCLUDES *
 ************/
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include "../src/logger.h"
#include "../src/deck_of_cards.h"
#include "../src/curses_output.h"
#include "../src/blackjack.h"

/****************
 * DECLARATIONS *
 ****************/
void test_init_shoe(uint8_t decks);
void test_ncurses();
void deal_hand(Deck *shoe);

int main()
{
    setlocale(LC_ALL, "");

    /****** Initialize logging *****/
    init_zlog("blackjack_test.conf", "test_cat");

    zlog_info(zc, "Starting tests.");
    test_init_shoe(1);
    test_init_shoe(2);

//    zlog_info(zc, "Testing ncurses...");
//    test_ncurses();

    zlog_info(zc, "Tests finished.");
    end_zlog();

    return 0;
}

/***************
 *  Summary: Test instantiation of a deck of cards
 *
 *  Description: Instantiate a deck of cards and print it out to verify correct set-up of cards.
 *
 *  Parameter(s):
 *      N/A
 *
 *  Returns:
 *      N/A
 */
void test_init_shoe(uint8_t decks)
{
    zlog_info(zc, "Initializing a %u deck shoe...", decks);
    Deck *shoe = init_deck(decks);
    print_shoe(shoe);

    zlog_info(zc, "Shuffling a %u deck shoe...", decks);
    shuffle_cards(shoe);
    print_shoe(shoe);

    zlog_info(zc, "Deal 5 cards...");
    deal_hand(shoe);

    zlog_info(zc, "Re-shuffling a %u deck shoe...", decks);
    shuffle_cards(shoe);
    print_shoe(shoe);

    zlog_info(zc, "Deal 5 cards...");
    deal_hand(shoe);

    free(shoe->shoe);
    free(shoe);
    return;
}

/***************
 *  Summary: Test instantiation of a deck of cards
 *
 *  Description: Instantiate a deck of cards and print it out to verify correct set-up of cards.
 *
 *  Parameter(s):
 *      N/A
 *
 *  Returns:
 *      N/A
 */
void deal_hand(Deck *shoe)
{
    zlog_info(zc, "Allocate memory for player.");
    Player *player = calloc(1, sizeof(Player));

    if (!player)
    {
        zlog_error(zc, "Couldn't allocate memory for player.");
        return;
    }

    zlog_info(zc, "Setting up player.");
    strcpy(player->name, "John Doe");
    player->money = 9999;
    player->hand1[0] = deal_card(shoe);
    player->hand2[0] = deal_card(shoe);
    player->hand1[1] = deal_card(shoe);
    player->hand2[1] = deal_card(shoe);
    player->hand1[2] = deal_card(shoe);

    printf("Name  : %s\n", player->name);
    printf("Money : $%d\n", player->money);
    printf("Hand 1: %s, %s, %s, %s, %s - %d\n", player->hand1[0].face, player->hand1[1].face,
            player->hand1[2].face, player->hand1[3].face, player->hand1[4].face, blackjack_count(player->hand1));
    printf("Hand 2: %s, %s, %s, %s, %s - %d\n\n", player->hand2[0].face, player->hand2[1].face,
            player->hand2[2].face, player->hand2[3].face, player->hand2[4].face, blackjack_count(player->hand2));

    free(player);
    return;
}

/***************
 *  Summary: Test instantiation of a deck of cards
 *
 *  Description: Instantiate a deck of cards and print it out to verify correct set-up of cards.
 *
 *  Parameter(s):
 *      N/A
 *
 *  Returns:
 *      N/A
 */
void test_ncurses()
{
    init_window();

    uint16_t lines, columns;
    getmaxyx(stdscr, lines, columns);

    char *msg1 = "stdscr has been initialized. Press a key to continue.";
    char *msg2 = "Testing welcome screen. Press a key to continue.";
    char *msg3 = "Dealer box tested. Press a key to continue.";
    char *msg4 = "Player box tested. Press a key to continue.";

    mvwaddstr(stdscr, 1, (columns - strlen(msg1)) / 2, msg1);
    wgetch(stdscr);

    erase();
    mvwaddstr(stdscr, 3, (columns - strlen(msg2)) / 2, msg2);
    wgetch(stdscr);

    welcome_screen();

    Dealer *dealer = calloc(1, sizeof(Dealer));
    dealer->name = "Dealer";
    dealer->faceup = FALSE;
    strcpy(dealer->hand[0].rank, " A");
    strcpy(dealer->hand[0].suit, SPADE);
    strcpy(dealer->hand[1].rank, "10");
    strcpy(dealer->hand[1].suit, DIAMOND);

    display_dealer(dealer);

    mvwaddstr(stdscr, lines - 3, (columns - strlen(msg3)) / 2, msg3);
    wgetch(stdscr);
    erase();

    dealer->faceup = TRUE;
    display_dealer(dealer);

    mvwaddstr(stdscr, lines - 3, (columns - strlen(msg3)) / 2, msg3);
    wgetch(stdscr);
    erase();

    Player *player = calloc(1, sizeof(Player));
    strcpy(player->name, "Charlotte");
    player->money = 12345678;
    strcpy(player->hand1[0].rank, " A");
    strcpy(player->hand1[0].suit, SPADE);
    strcpy(player->hand2[0].rank, "10");
    strcpy(player->hand2[0].suit, DIAMOND);

    display_player(player);

    mvwaddstr(stdscr, lines - 2, (columns - strlen(msg4)) / 2, msg4);
    wgetch(stdscr);

    end_window();
}
