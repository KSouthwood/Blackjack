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
 *  curses_output.h
 *
 *  Created on: Oct 17, 2018
 *      Author: Keri Southwood-Smith
 *
 *  Description:
 */


#ifndef CURSES_OUTPUT_H_
#define CURSES_OUTPUT_H_

/************
 * INCLUDES *
 ************/
#include <ncurses.h>
#include <stdlib.h>

#include "blackjack.h"
#include "unicode_box_chars.h"
#include "logger.h"

/***********
 * DEFINES *
 ***********/
#define PLAYER_WINDOW_COLS 20
#define PLAYER_WINDOW_LINE 7

enum cursorMode {CURS_INVIS, CURS_NORMAL, CURS_VVIS};

/****************
 * DECLARATIONS *
 ****************/
void init_window();
void end_window();
void welcome_screen();
void display_dealer(Dealer *dealer);
void display_player(Player *player);

#endif /* CURSES_OUTPUT_H_ */
