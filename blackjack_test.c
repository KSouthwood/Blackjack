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

#include "deck_of_cards.h"

/****************
 * DECLARATIONS *
 ****************/
void test_init_deck();
void test_init_shoe(unsigned short int decks);
void test_shuffle(unsigned short int decks);

int main()
{
    test_init_deck();

    test_init_shoe(1);
    test_init_shoe(2);

    test_shuffle(1);

    return 0;
}

/*
 * Test initializing the deck of cards.
 */

void test_init_deck()
{
    printf("Initializing deck of cards...\n");
    card *deck;
    deck = init_deck();
    printf("Printing deck of cards:\n");
    for (int c = 0; c < 52; c++)
    {
        printf("%s%s, ", deck[c].rank, deck[c].suit);
    }
    printf("\n");

    free(deck);
    return;
}

/*
 * Test initializing a shoe of cards.
 */

void test_init_shoe(unsigned short int decks)
{
    unsigned short int *shoe;
    int cards = 52 * decks;

    printf("\nInitializing shoe of cards with %u deck(s)...\n", decks);
    shoe = init_shoe(decks);
    printf("Printing shoe: ");
    for (int card = 0; card < cards; card++)
    {
        printf("%u, ", shoe[card]);
    }
    printf("\n");

    free(shoe);
    return;
}

/*
 * Test shuffling a deck of cards.
 */
void test_shuffle(unsigned short int decks)
{
    srand(68461);

    // Instantiate a deck of cards first
    unsigned short int *shoe;
    unsigned short int cards = 52 * decks;

    printf("\nInitializing shoe of cards with %u deck(s)...\n", decks);
    shoe = init_shoe(decks);
    printf("Printing shoe: ");
    for (int card = 0; card < cards; card++)
    {
        printf("%u, ", shoe[card]);
    }
    printf("\n");

    // Now shuffle the shoe
    printf("\nShuffling the shoe of cards...\n");
    shuffle(shoe, cards);
    printf("Printing shuffled shoe: ");
    for (int card = 0; card < cards; card++)
    {
        printf("%u, ", shoe[card]);
    }
    printf("\n");

    printf("\nRe-shuffling the shoe of cards...\n");
    shuffle(shoe, cards);
    printf("Printing re-shuffled shoe: ");
    for (int card = 0; card < cards; card++)
    {
        printf("%u, ", shoe[card]);
    }
    printf("\n");

    free(shoe);
    return;
}
