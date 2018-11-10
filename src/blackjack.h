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
 *  blackjack.h
 *
 *  Created on: Oct 18, 2018
 *      Author: Keri Southwood-Smith
 *
 *  Description:
 */


#ifndef BLACKJACK_H_
#define BLACKJACK_H_

/************
 * INCLUDES *
 ************/
#include <stdint.h>

#include "deck_of_cards.h"

/***********
 * DEFINES *
 ***********/
typedef struct Player
{
    char name[11];
    uint32_t money;
    uint32_t bet;
    Card hand1[5];
    Card hand2[5];
} Player;

typedef struct Dealer
{
    Card hand[5];
    char *name;
    bool faceup;
} Dealer;

typedef struct Table
{
    uint8_t numPlayers;
    Player *players;
    Dealer *dealer;
    Deck *shoe;
} Table;

typedef enum PlayerChoice
{
    STAND, HIT, DOUBLE, SPLIT
} PlayerChoice;

#define DEBUG 1 // set to 0 to get true random shuffle, etc.

/****************
 * DECLARATIONS *
 ****************/

#endif /* BLACKJACK_H_ */
