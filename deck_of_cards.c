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
 *  deck_of_cards.c
 *
 *  Created on: Oct 11, 2018
 *      Author: Keri Southwood-Smith
 *
 *  Description: C module to instantiate a deck of cards (one or more as needed) and
 *               shuffle and deal as well.
 */


/************
 * INCLUDES *
 ************/
#include "deck_of_cards.h"

/****************
 * DECLARATIONS *
 ****************/

/*
 * Instantiate the deck of cards.
 *
 * Build a character array to hold the graphical representation of each card (rank and suit).
 *
 *
 */
card *init_deck()
{
    card *deck = malloc(52 * sizeof(card));
    char *ranks[13] = {" A", " 2", " 3", " 4", " 5", " 6", " 7", " 8", " 9", "10", " J", " Q", " K"};
    char *suits[4] = {SPADE, CLUB, HEART, DIAMOND};

    for (int c = 0; c < 52; c++)
    {
        strcpy(deck[c].rank, ranks[c % 13]);
        strcpy(deck[c].suit, suits[c / 13]);
    }
    return deck;
}

/*
 * Instantiate a shoe of cards.
 *
 * Build an array for the number of decks of cards being used. Initially it'll hold the sequence
 * 0 - 51 one or more times in order before shuffling.
 *
 */
unsigned short int *init_shoe(unsigned short int decks)
{
    unsigned short int *shoe = malloc(52 * decks * sizeof(unsigned short int));

    for (int deck = 0; deck < decks; deck++)
    {
        for (int card = 0; card < 52; card++)
        {
            shoe[(deck * 52) + card] = card;
        }
    }
    return shoe;
}

/*
 * Shuffle the deck of cards.
 *
 * Shuffle the deck(s) of cards using the Fisher-Yates shuffle algorithm..
 *
 */

void shuffle(unsigned short int *shoe, unsigned short int cards)
{
    unsigned short int shoe_tmp;
    int swap;

    for (int card = cards - 1; card > 0; card--)
    {
        swap = rand() % cards;

        shoe_tmp = shoe[swap];
        shoe[swap] = shoe[card];
        shoe[card] = shoe_tmp;
    }

    return;
}
