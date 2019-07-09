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
 *  deck_of_cards.h
 *
 *  Created on: Oct 11, 2018
 *      Author: Keri Southwood-Smith
 *
 *  Description:
 */

#ifndef DECK_OF_CARDS_H_
#define DECK_OF_CARDS_H_

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

/************
 * INCLUDES *
 ************/

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "logger.h"

/***********
 * DEFINES *
 ***********/
#define CARDS_IN_DECK 52
#define SPADE "\u2660"
#define CLUB "\u2663"
#define HEART "\u2665"
#define DIAMOND "\u2666"

typedef struct Card
{
    char rank[3];
    char suit[4];
    char face[6];
    uint8_t value;
} Card;

typedef struct Deck
{
    Card *shoe;
    uint16_t cards; // total cards in the deck
    uint16_t deal;  // which card we're at in the deck
    bool shuffle;   // flag for if the cards need to be shuffled
    uint16_t cut;   // what point in the deck do we re-shuffle
} Deck;

typedef struct CardList
{
    Card *card;
    struct CardList *nextCard;
} CardList;

typedef struct Hand
{
    Card cards[7];
    uint8_t numCards;
    uint32_t bet;
    uint8_t score;
    struct Hand *nextHand;
} Hand;

/****************
 * DECLARATIONS *
 ****************/
Deck *init_deck(uint8_t decks);
void shuffle_cards(Deck *shoe);
void deal_card(Deck *shoe, Hand *hand);
uint8_t blackjack_score(Hand hand);

#endif /* DECK_OF_CARDS_H_ */
