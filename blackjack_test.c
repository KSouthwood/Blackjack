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
void test_init_shoe(uint8_t decks);
void print_shoe(card *shoe, uint16_t cards);

int main()
{
    test_init_shoe(1);
    test_init_shoe(2);
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
    uint16_t cards = decks * CARDS_IN_DECK;

    printf("Initialize a %u deck shoe...\n", decks);
    card *shoe = init_deck(decks);
    print_shoe(shoe, cards);

    printf("Shuffling a %u deck shoe...\n", decks);
    shuffle_cards(shoe, cards);
    print_shoe(shoe, cards);

    free(shoe);
    return;
}

/***************
 *  Summary: Prints a shoe of cards in order.
 *
 *  Description: Prints a shoe of cards starting with the first card through the last card.
 *
 *  Parameter(s):
 *      shoe - the shoe of cards to print
 *      cards - the number of cards in the shoe
 *
 *  Returns:
 *      N/A
 */
void print_shoe(card *shoe, uint16_t cards)
{
    printf("Printing deck of cards:\n");

    for (uint16_t c = 0; c < cards; c++)
    {
        printf("%s%s", shoe[c].rank, shoe[c].suit);
        if ((c + 1) % 13 == 0)
        {
            printf("\n");
        }
        else
        {
            printf(", ");
        }
    }

    return;
}
