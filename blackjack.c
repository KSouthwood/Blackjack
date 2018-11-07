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

/************
 * INCLUDES *
 ************/
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>

#include "curses_output.h"
#include "logger.h"
#include "blackjack.h"

/****************
 * DECLARATIONS *
 ****************/
uint8_t get_num_players();
player_t *init_players(uint8_t num_players);
dealer_t *init_dealer();
void play_game(uint8_t num_players, player_t *players, dealer_t *dealer);
void deal_hands(deck_t *shoe, uint8_t num_players, player_t *players, dealer_t *dealer);

int main()
{
#if DEBUG
    srand(1968);
#else
    srand(time(NULL));
#endif

    if (init_zlog("blackjack.conf", "log")) return ERR;

    init_window();
    zlog_info(zc, "ncurses initialized.");

    welcome_screen();
    zlog_info(zc, "Welcome screen displayed.");

    uint8_t num_players = get_num_players();
    if (!num_players)
    {
        zlog_debug(zc, "Got 0 back which means player wants to quit.");
        goto exit_3;
    }

    player_t *players = init_players(num_players);
    zdebug("players is at %#X.", (uint32_t) players);
    if (!players)
        {
            zerror("Couldn't allocate memory for players array.");
            goto exit_2;
        }

    dealer_t *dealer = init_dealer();
    zdebug("dealer is at %#X.", (uint32_t) dealer);
    if (!dealer)
        {
            zerror("Couldn't allocate memory for dealer struct.");
            goto exit_1;
        }

    zinfo("Calling play game with num_players: %i, players: %#X, dealer: %#X.",
            num_players, (uint32_t) players, (uint32_t) dealer);
    play_game(num_players, players, dealer);

exit_1:
    zlog_debug(zc, "Freeing memory for dealer.");
    free(dealer);

exit_2:
    zlog_debug(zc, "Freeing memory for players.");
    free(players);

exit_3:
    zlog_debug(zc, "Stopping ncurses.");
    end_window();
    end_zlog();

    return 0;
}

/***************
 *  Summary: Get the number of players
 *
 *  Description: Asks how many players will be playing, only accepting 1 to 5 players. All other
 *      keypresses will be ignored.
 *
 *  Parameter(s):
 *      N/A
 *
 *  Returns:
 *      num_players: The number of players playing blackjack
 */
uint8_t get_num_players()
{
    uint8_t num_players = 0;
    char input;

    curs_set(CURS_NORMAL);
    mvwaddstr(stdscr, 0, 0, "How many people are playing? (1-5 or q to quit.) ");

    while (num_players == 0)
    {
        input = wgetch(stdscr);

        // q was entered to quit program
        if (tolower(input) == 'q')
        {
            num_players = 0;
            zlog_debug(zc, "'q' was entered. Exiting loop.");
            break;
        }

        // convert input to a number and make sure it's in our acceptable range
        if (atoi(&input))
        {
            num_players = (atoi(&input) > 5) ? 0 : atoi(&input);
        }
    }

    zinfo("Got %i players as input.", num_players);
    waddch(stdscr, input);
    curs_set(CURS_INVIS);

    return num_players;
}

/***************
 *  Summary: Instantiate the players array
 *
 *  Description: Sets up the players array by getting the names of all the players and giving each
 *      one an initial bank of $1,000.
 *
 *  Parameter(s):
 *      N/A
 *
 *  Returns:
 *      players: a pointer to an array of player structs with their name and initial amount of money
 */
player_t *init_players(uint8_t num_players)
{
    player_t *players = calloc(num_players, sizeof(player_t));
    zlog_debug(zc, "Pointer to players is %#X", (uint32_t) players);
    if (!players) goto exit;

    /***** Get player names *****/
    curs_set(CURS_NORMAL); // enable cursor
    mvwaddstr(stdscr, 2, 0, "Player names are limited to 10 characters max.");
    for (uint8_t ii = 0; ii < num_players; ii++)
    {
        mvwprintw(stdscr, 3 + ii, 0, "What is player %i's name? ", ii + 1);
        echo(); // Turn echo on temporarily
        wgetnstr(stdscr, players[ii].name, 10);
        noecho();
        players[ii].money = 1000;
    }
    curs_set(CURS_INVIS); // disable cursor

exit:
    return players;
}

/***************
 *  Summary: Instantiate the dealer
 *
 *  Description: Set's up the dealer struct to initial values
 *
 *  Parameter(s):
 *      N/A
 *
 *  Returns:
 *      dealer: a pointer to dealer struct or NULL if an error occurred
 */

dealer_t *init_dealer()
{
    dealer_t *dealer = calloc(1, sizeof(dealer_t));
    zlog_debug(zc, "Pointer to dealer is %#X", (uint32_t) dealer);

    if (dealer != NULL)
    {
        dealer->name = "Dealer";
        dealer->faceup = FALSE;
    }

    return dealer;
}


/***************
 *  Summary: Main play loop
 *
 *  Description: Handle initializing and shuffling the deck, dealing the cards, and quitting.
 *
 *  Parameter(s):
 *      num_players: number of players at the table
 *      players: array of player_t structs
 *      dealer: dealer_t struct for the dealers hand
 *
 *  Returns:
 *      N/A
 */
void play_game(uint8_t num_players, player_t *players, dealer_t *dealer)
{
    zinfo("play_game called with num_players: %i, players: %#X, dealer: %#X.",
            num_players, (uint32_t) players, (uint32_t) dealer);
    uint8_t decks = (num_players < 4) ? 4 : 6; // determine how many decks in the shoe

    zlog_info(zc, "Instantiating %i decks for %i players.", decks, num_players);
    deck_t *shoe = init_deck(decks);
    zdebug("deck_t shoe pointer returned as %#X.", (uint32_t) shoe);
    zinfo("Shuffling shoe.");
    shuffle_cards(shoe);

    zdebug("Setting game_over flag and starting game loop.");
    bool game_over = FALSE;
    while (!game_over)
    {
        zinfo("Calling deal_hands for initial deal.");
        deal_hands(shoe, num_players, players, dealer);
        check_dealer_hand(num_players, players, dealer);
        zdebug("Setting game_over to TRUE to escape loop.");
        game_over = TRUE;
    }

    zdebug("Freeing memory for card_t struct shoe (%#X) in deck_t struct shoe.", (uint32_t) shoe->shoe);
    free(shoe->shoe);
    zdebug("Freeing memory for deck_t struct shoe (%#X).", (uint32_t) shoe);
    free(shoe);
    return;
}

/***************
 *  Summary: Deal the initial hands to the table
 *
 *  Description: Deal the initial round of cards to the table, setting the dealers faceup boolean to
 *      false.
 *
 *  Parameter(s):
 *  	shoe: deck_t struct containing the cards to be dealt
 *  	num_players: number of players at the table
 *  	players: array of player_t structs containing information about each player
 *  	dealer: dealer_t struct for the dealer
 *
 *	Returns:
 *		N/A
 */
void deal_hands(deck_t *shoe, uint8_t num_players, player_t *players, dealer_t *dealer)
{
    // re-shuffle the deck if we're nearing the end
    // TODO: Get rid of magic number. Switch to calculated random point or just use a #define
    if ((shoe->cards - shoe->left) < (shoe->cards * 0.8))
    {
        shuffle_cards(shoe);
    }

    for (uint8_t c = 0; c < 2; c++)
    {
        for (uint8_t i = 0; i < num_players; i++)
        {
            players[i].hand1[c] = deal_card(shoe);
            zinfo("Player %i got card: %s", i, players[i].hand1[c].face);
        }

        dealer->hand[c] = deal_card(shoe);
        zinfo("Dealer got card: %s", dealer->hand[c].face);
    }

    dealer->faceup = false;
    zinfo("Set dealer faceup flag to false. Returning.");

    return;
}

/***************
 *  Summary: Check dealer hand for blackjack
 *
 *  Description: Checks the dealer's hand in order to offer insurance or for blackjack which is an
 *      automatic win. First check dealer upcard for an Ace, and offer insurance to players if true.
 *      If not an Ace (or after offering insurance) check if we have a blackjack, and collect all
 *      bets if we do.
 *
 *  Parameter(s):
 *  	num_players:    number of players at the table
 *      players:        pointer to array of player_t structs
 *      dealer:         pointer to dealer_t struct
 *
 *	Returns:
 *		N/A
 */
void check_dealer_hand(uint8_t num_players, player_t *players, dealer_t *dealer)
{
    // check for Ace in dealer upcard
    bool dealer_ace = (dealer->hand[1].rank == "A");
    bool dealer_21  = (blackjack_count(dealer->hand) == 21);


}

