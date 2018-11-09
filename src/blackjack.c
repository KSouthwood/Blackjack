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

/***********
 * DEFINES *
 ***********/
enum errorCode {NO_ERROR, PLAYER_ALLOC, DEALER_ALLOC, DECK_ALLOC, PLAYER_QUIT};

/****************
 * DECLARATIONS *
 ****************/
uint8_t setup_table(Table *table);
uint8_t get_num_players();
Player *init_players(uint8_t num_players);
Dealer *init_dealer();
void play_game(Table *table);
void deal_hands(Table *table);
void check_dealer_hand(Dealer *dealer);

int main()
{
    // seed random number generator
#if DEBUG
    srand(1968);
#else
    srand(time(NULL));
#endif

    if (init_zlog("blackjack.conf", "log")) return ERR;

    init_window();
    zinfo("ncurses initialized.");
    welcome_screen();
    zinfo("Welcome screen displayed.");

    // create table struct
    Table *table = calloc(1, sizeof(Table));
    if (!table)
    {
        zerror("Couldn't allocate memory for Table struct!");
    }
    else
    {
        switch (setup_table(table))
        {
            /***** No breaks or default on purpose *****/
            case NO_ERROR:
                zinfo("Calling play game with num_players: %i, players: %#X, dealer: %#X.",
                        table->numPlayers, (uint32_t) table->players, (uint32_t) table->dealer);
                play_game(table);
                zdebug("Freeing memory for Card struct shoe (%#X) in Deck struct shoe.",
                        (uint32_t) table->shoe->shoe);
                free(table->shoe->shoe);
                zdebug("Freeing memory for Deck struct shoe (%#X).", (uint32_t) table->shoe);
                free(table->shoe);
            case DECK_ALLOC:
                zlog_debug(zc, "Freeing memory for dealer.");
                free(table->dealer);
            case DEALER_ALLOC:
                zlog_debug(zc, "Freeing memory for players.");
                free(table->players);
            case PLAYER_ALLOC:
            case PLAYER_QUIT:
                zdebug("Freeing memory for Table.");
                free(table);
        }
    }

    zlog_debug(zc, "Stopping ncurses.");
    end_window();
    end_zlog();

    return 0;
}

/***************
 *  Summary: Set-up the table for game play.
 *
 *  Description: Set-up each of the fields of the table by calling other functions in order. Exit
 *      immediately if we encounter any errors along the way.
 *
 *  Parameter(s):
 *  	table: a pointer to the Table struct
 *
 *	Returns:
 *		error:
 */
uint8_t setup_table(Table *table)
{
    // get the number of players at table
    table->numPlayers = get_num_players();
    if (!table->numPlayers)
    {
        zdebug("Got 0 back which means player wants to quit.");
        return PLAYER_QUIT;
    }

    // instantiate player(s)
    table->players = init_players(table->numPlayers);
    zdebug("players is at %#X.", (uint32_t) table->players);
    if (!table->players)
    {
        zerror("Couldn't allocate memory for players array.");
        return PLAYER_ALLOC;
    }

    table->dealer = init_dealer();
    zdebug("dealer is at %#X.", (uint32_t) table->dealer);
    if (!table->dealer)
        {
            zerror("Couldn't allocate memory for dealer struct.");
            return DEALER_ALLOC;
        }

    uint8_t decks = (table->numPlayers < 4) ? 4 : 6; // determine how many decks in the shoe
    zinfo("Instantiating %i decks for %i players.", decks, table->numPlayers);
    table->shoe = init_deck(decks);
    zdebug("Deck shoe pointer returned as %#X.", (uint32_t) table->shoe);
    if(!table->shoe)
    {
        zerror("Couldn't allocate memory for deck of cards.");
        return DECK_ALLOC;
    }

    return NO_ERROR;
}

/***************
 *  Summary: Get the number of players
 *
 *  Description: Asks how many players will be playing, only accepting 1 to 5 players. All other
 *      keypresses besides 'q' (for quitting the game) will be ignored.
 *
 *  Parameter(s):
 *      N/A
 *
 *  Returns:
 *      numOfPlayers: The number of players playing blackjack
 */
uint8_t get_num_players()
{
    uint8_t numOfPlayers = 0;
    char input;

    curs_set(CURS_NORMAL);
    mvwaddstr(stdscr, 0, 0, "How many people are playing? (1-5 or q to quit.) ");

    while (numOfPlayers == 0)
    {
        input = wgetch(stdscr);

        // q was entered to quit program
        if (tolower(input) == 'q')
        {
            numOfPlayers = 0;
            zdebug("'q' was entered. Exiting loop.");
            break;
        }

        // convert input to a number and make sure it's in our acceptable range
        if (atoi(&input))
        {
            numOfPlayers = (atoi(&input) > 5) ? 0 : atoi(&input);
        }
    }

    zinfo("Got %i players as input.", numOfPlayers);
    waddch(stdscr, input);
    curs_set(CURS_INVIS);

    return numOfPlayers;
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
 *          or NULL if we couldn't allocate memory
 */
Player *init_players(uint8_t num_players)
{
    Player *players = calloc(num_players, sizeof(Player));
    zlog_debug(zc, "Pointer to players is %#X", (uint32_t) players);
    if (players)
    {
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
    }

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

Dealer *init_dealer()
{
    Dealer *dealer = calloc(1, sizeof(Dealer));
    zlog_debug(zc, "Pointer to dealer is %#X", (uint32_t) dealer);

    if (dealer)
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
void play_game(Table *table)
{
    zinfo("play_game called with num_players: %i, players: %#X, dealer: %#X.",
            table->numPlayers, (uint32_t) table->players, (uint32_t) table->dealer);
    zinfo("Shuffling shoe.");
    shuffle_cards(table->shoe);

    zdebug("Setting game_over flag and starting game loop.");
    bool game_over = FALSE;
    while (!game_over)
    {
        zinfo("Calling deal_hands for initial deal.");
        deal_hands(table);
        check_dealer_hand(table->dealer);
        zdebug("Setting game_over to TRUE to escape loop.");
        game_over = TRUE;
    }

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
void deal_hands(Table *table)
{
    // re-shuffle the deck if we're nearing the end
    // TODO: Get rid of magic number. Switch to calculated random point or just use a #define
    if ((table->shoe->cards - table->shoe->left) < (table->shoe->left * 0.8))
    {
        shuffle_cards(table->shoe);
    }

    for (uint8_t c = 0; c < 2; c++)
    {
        for (uint8_t i = 0; i < table->numPlayers; i++)
        {
            table->players[i].hand1[c] = deal_card(table->shoe);
            zinfo("Player %i got card: %s", i, table->players[i].hand1[c].face);
        }

        table->dealer->hand[c] = deal_card(table->shoe);
        zinfo("Dealer got card: %s", table->dealer->hand[c].face);
    }

    zinfo("Set dealer faceup flag to false. Returning.");
    table->dealer->faceup = false;

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
void check_dealer_hand(Dealer *dealer)
{
    // check for Ace in dealer upcard
    bool dealer_ace = (strcmp(dealer->hand[1].rank, "A"));
    bool dealer_21  = (blackjack_count(dealer->hand) == 21);

    zinfo("%i %i", dealer_ace, dealer_21);

    return;
}

