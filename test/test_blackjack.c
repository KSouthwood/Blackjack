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
 *  test_blackjack.c
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
#include "../src/blackjack.h"

/****************
 * DECLARATIONS *
 ****************/
void test_init_shoe(uint8_t decks);
void deal_hand(Deck *shoe);

int main()
{
    setlocale(LC_ALL, "");
    srand(1968);

    /****** Initialize logging *****/
    init_zlog("test_blackjack.conf", "test_cat");

    zinfo("Starting tests.");
    test_init_shoe(1);
    test_init_shoe(2);

    zinfo("Tests finished.");
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
    Table *table = calloc(1, sizeof(Table));
    
    zlog_info(zc, "Initializing a %u deck shoe...", decks);
    table->shoe = init_deck(decks);
    print_shoe(table->shoe);

    zlog_info(zc, "Shuffling a %u deck shoe...", decks);
    shuffle_cards(table->shoe);
    print_shoe(table->shoe);

    zlog_info(zc, "Deal 5 cards...");
    deal_hand(table->shoe);

    zlog_info(zc, "Re-shuffling a %u deck shoe...", decks);
    shuffle_cards(table->shoe);
    print_shoe(table->shoe);

    zlog_info(zc, "Deal 5 cards...");
    deal_hand(table->shoe);

    free(table->shoe->shoe);
    free(table->shoe);
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
    zinfo("Allocate memory for player.");
    Player *player = calloc(1, sizeof(Player));

    if (!player)
    {
        zlog_error(zc, "Couldn't allocate memory for player.");
        return;
    }

    zlog_info(zc, "Setting up player.");
    strcpy(player->name, "John Doe");
    player->money = 9999;
    player->hand1.hand[player->hand1.numCards++] = deal_card(shoe);
    player->hand2.hand[player->hand2.numCards++] = deal_card(shoe);
    player->hand1.hand[player->hand1.numCards++] = deal_card(shoe);
    player->hand2.hand[player->hand2.numCards++] = deal_card(shoe);
    player->hand1.hand[player->hand1.numCards++] = deal_card(shoe);

    printf("Name  : %s\n", player->name);
    printf("Money : $%u\n", player->money);
    printf("Hand 1: %5s, %5s, %5s, %5s, %5s - %hhu\n", player->hand1.hand[0].face, player->hand1.hand[1].face,
            player->hand1.hand[2].face, player->hand1.hand[3].face, player->hand1.hand[4].face, blackjack_count(player->hand1.hand));
    printf("Hand 2: %5s, %5s, %5s, %5s, %5s - %hhu\n\n", player->hand2.hand[0].face, player->hand2.hand[1].face,
            player->hand2.hand[2].face, player->hand2.hand[3].face, player->hand2.hand[4].face, blackjack_count(player->hand2.hand));

    free(player);
    return;
}
