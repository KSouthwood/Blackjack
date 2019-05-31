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

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

//#ifndef _DEFAULT_SOURCE
//#define _DEFAULT_SOURCE
//#endif

/************
 * INCLUDES *
 ************/
#include <stdint.h>
#include <ncurses.h>
#include "deck_of_cards.h"

/***********
 * DEFINES *
 ***********/
typedef struct Player
{
    char name[11];
    uint32_t money;
    Hand hand;
    WINDOW *window;
} Player;

typedef struct Dealer
{
    char name[7];
    bool faceup;    // TRUE to show card
    Hand hand;
    WINDOW *window;
} Dealer;

typedef struct Table
{
    uint8_t numPlayers;
    Player *players;
    Dealer *dealer;
    Deck *shoe;
    WINDOW *msgWin;         // ncurses window to display messages in
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
