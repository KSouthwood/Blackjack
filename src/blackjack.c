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
#include <errno.h>
#include <limits.h>
#include <unistd.h>

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
void play_hands(Table *table);
void get_bets(Table *table);
void play_dealer_hand(Dealer *dealer, Deck *shoe);
bool double_down(Player player);
void check_table(Table table, bool dealerBlackjack);

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
    zdebug("table pointer: %#X.", (uint32_t) table);
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
                zinfo("Calling play game with table->numPlayers: %i, table->players: %#X, table->dealer: %#X.",
                        table->numPlayers, (uint32_t) table->players, (uint32_t) table->dealer);
                play_game(table);
                zdebug("Freeing table->shoe->shoe: %#X.", (uint32_t) table->shoe->shoe);
                free(table->shoe->shoe);
                zdebug("Freeing table->shoe: %#X.", (uint32_t) table->shoe);
                free(table->shoe);
            case DECK_ALLOC:
                zlog_debug(zc, "Freeing table->dealer: %#X.", (uint32_t) table->dealer);
                free(table->dealer);
            case DEALER_ALLOC:
                zlog_debug(zc, "Freeing table->players: %#X.", (uint32_t) table->players);
                free(table->players);
            case PLAYER_ALLOC:
            case PLAYER_QUIT:
                zdebug("Freeing table: %#X.", (uint32_t) table);
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
    if (table->numPlayers == 0)
    {
        zdebug("Got 0 back which means player wants to quit.");
        return PLAYER_QUIT;
    }

    // instantiate player(s)
    table->players = init_players(table->numPlayers);
    zdebug("table->players pointer: %#X.", (uint32_t) table->players);
    if (!table->players)
    {
        zerror("Couldn't allocate memory for players array.");
        return PLAYER_ALLOC;
    }
    
    table->dealer = init_dealer();
    zdebug("table->dealer pointer: %#X.", (uint32_t) table->dealer);
    if (!table->dealer)
        {
            zerror("Couldn't allocate memory for dealer struct.");
            return DEALER_ALLOC;
        }

    uint8_t decks = (table->numPlayers < 4) ? 4 : 6; // determine how many decks in the shoe
    zinfo("Instantiating %i decks for %i players.", decks, table->numPlayers);
    table->shoe = init_deck(decks);
    zdebug("table->shoe pointer: %#X.", (uint32_t) table->shoe);
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
            numOfPlayers = (atoi(&input) > 5) ? 0 : 1; // TODO change back to atoi(&input) when ready
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
 *      numPlayers: the number of player structs to set up. Must be between 1 and 5 inclusive.
 *
 *  Returns:
 *      players: a pointer to an array of player structs with their name and initial amount of money
 *          or NULL if we couldn't allocate memory
 */
Player *init_players(uint8_t numPlayers)
{
    if (numPlayers < 1 || numPlayers > 5)
    {
        zerror("numPlayers value passed in is out of range. Got %u when it should be 1-5.", numPlayers);
        return NULL;
    }
    
    Player *players = calloc(numPlayers, sizeof(Player));
    zdebug("players pointer: %#X.", (uint32_t) players);
    if (players)
    {
        /***** Get player names *****/
        curs_set(CURS_NORMAL); // enable cursor
        mvwaddstr(stdscr, 2, 0, "Player names are limited to 10 characters max.");
        echo(); // Turn echo on temporarily
        for (uint8_t ii = 0; ii < numPlayers; ii++)
        {
            mvwprintw(stdscr, 3 + ii, 0, "What is player %i's name? ", ii + 1);
            wgetnstr(stdscr, players[ii].name, 10);
            players[ii].money = 1000;
            players[ii].bet = 0;
            players[ii].hand1.numCards = 0;
            players[ii].hand2.numCards = 0;
        }
        noecho();
        curs_set(CURS_INVIS);   // disable cursor
        wclear(stdscr);         // clear screen before returning
        wrefresh(stdscr);       // display the screen
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
    zdebug("Dealer pointer: %#X", (uint32_t) dealer);

    if (dealer)
    {
        strncpy(dealer->name, "Dealer", 7);
        dealer->faceup = FALSE;
        dealer->hand.numCards = 0;
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
    uint8_t hands = 5;  // TODO: remove
    while (!game_over)
    {
        zinfo("Display windows.");
        wclear(stdscr);
        wrefresh(stdscr);
        display_dealer(table->dealer);
        for (uint8_t i = 0; i < table->numPlayers; i++)
        {
            display_player(&table->players[i]);
        }
        zinfo("Get bets from players.");
        get_bets(table);
        zinfo("Calling deal_hands for initial deal.");
        deal_hands(table);
        zinfo("Check dealer hand for blackjack.");
        check_dealer_hand(table->dealer); // TODO: make boolean function so we can skip player hands
        play_hands(table);
        zinfo("Player %s has %u cards.", table->players[0].name, table->players[0].hand1.numCards);
        play_dealer_hand(table->dealer, table->shoe);
        zinfo("******************************");
        check_table(*table, FALSE);
        zinfo("******************************");
        game_over = (--hands) ? FALSE : TRUE;
        if (game_over)
        {
            zdebug("Setting game_over to TRUE to escape loop.");
        }
        else
        {
            zdebug("Playing %u more hands.", hands);
        }
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
    // TODO Get rid of magic number. Switch to calculated random point or just use a #define
    if ((table->shoe->cards - table->shoe->left) < (table->shoe->left * 0.8))
    {
        shuffle_cards(table->shoe);
    }

    // Reset players & dealer number of cards to 0
    for (uint8_t i = 0; i < table->numPlayers; i++)
    {
        table->players[i].hand1.numCards = 0;
        table->players[i].hand2.numCards = 0;
        memset(table->players[i].hand1.hand, '\0', sizeof(table->players[i].hand1.hand));
        memset(table->players[i].hand2.hand, '\0', sizeof(table->players[i].hand2.hand));
    }
    table->dealer->hand.numCards = 0;
    memset(table->dealer->hand.hand, '\0', sizeof(table->dealer->hand.hand));
    
    // deal two cards to players and dealer
    for (uint8_t c = 0; c < 2; c++)
    {
        for (uint8_t i = 0; i < table->numPlayers; i++)
        {
            table->players[i].hand1.hand[table->players->hand1.numCards++] = deal_card(table->shoe);
            zinfo("Player %i got card: %s", i, table->players[i].hand1.hand[c].face);
        }

        table->dealer->hand.hand[table->dealer->hand.numCards++] = deal_card(table->shoe);
        zinfo("Dealer got card: %s", table->dealer->hand.hand[c].face);
    }

    zinfo("Set dealer faceup flag to false. Returning.");
    table->dealer->faceup = false;

    // display player and dealer hands
    display_dealer(table->dealer);
    for (uint8_t i = 0; i < table->numPlayers; i++)
    {
        display_player(&table->players[i]);
    }
    
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
 *      dealer: pointer to Dealer struct
 *
 *	Returns:
 *		N/A
 */
void check_dealer_hand(Dealer *dealer)
{
    // if our upcard is not a face card or Ace, no need to run the checks
    if (dealer->hand.hand[1].value < 10)
    {
        zinfo("Upcard is not an Ace or face card. Exiting check.");
        return;
    }
    
    // check for Ace in dealer upcard and offer insurance if it is
    if (!strcmp(dealer->hand.hand[1].rank, "A"))
    {
        zinfo("Dealer is showing an Ace.");
        //TODO offer players insurance
    }

    // No Ace or we've offered insurance, now check if we have blackjack
    if (blackjack_count(dealer->hand) == 21)
    {
        zinfo("Dealer has blackjack. Players lose.");
        //TODO: Take all players money - dealer blackjack
    }

    return;
}

/***************
 *  Summary: Play each players hand in order
 *
 *  Description: Go around the table getting what each player wants to do in order. Move to the next player once the
 *      current player has chosen stand or gone bust. Return from the function once all players have finished.
 *
 *  Parameter(s):
 *  	table:    the Table struct so we can access numPlayers, players and shoe
 *
 *	Returns:
 *		N/A
 */
void play_hands(Table *table)
{
    for (uint8_t player = 0; player < table->numPlayers; player++)
    {
        Player *currentPlayer = &table->players[player];
        zinfo("***** Playing %s's hand. *****", currentPlayer->name);
        bool playHand = TRUE;
        while (playHand)
        {
            display_player(currentPlayer);
            switch(get_player_choice(currentPlayer))
            {
                case STAND:
                    playHand = FALSE;
                    break;
                case HIT:
                    // get a new card
                    currentPlayer->hand1.hand[currentPlayer->hand1.numCards++] = deal_card(table->shoe);
                    zinfo("Player %u got card %s.", player, currentPlayer->hand1.hand[currentPlayer->hand1.numCards - 1].face);
                    if (blackjack_count(currentPlayer->hand1) > 21) playHand = FALSE;    // player busted
                    break;
                case DOUBLE:
                    if (double_down(*currentPlayer))
                    {
                        currentPlayer->hand1.hand[currentPlayer->hand1.numCards++] = deal_card(table->shoe);
                        playHand = FALSE;
                        zinfo("Player %u got card %s.", player, currentPlayer->hand1.hand[currentPlayer->hand1.numCards - 1].face);
                    }
                    break;
                case SPLIT:
                    // TODO: split cards into two hands
                    break;
                default:
                    // no default case
                    break;
            }
            sleep(1);
        }
        
    }
    
    return;
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
void get_bets(Table *table)
{
    zinfo("Set variables");
    char input[8];
    char *endptr = NULL;
    long bet = 0;
    echo();
    
    zinfo("Start player loop.");
    for (uint8_t player = 0; player < table->numPlayers; player++)
    {
        zinfo("Input while loop");
        bool validBet = FALSE;
        while (!validBet)
        {
            zinfo("Print prompt and get input");
            wmove(stdscr, LINES - 3, 0);
            wclrtobot(stdscr);
            refresh();
            mvwaddstr(stdscr, LINES - 3, 0, table->players[player].name);
            waddstr(stdscr, ", how much money do you wish to bet? ");
            wgetnstr(stdscr, input, 7);
            
            zinfo("Convert input to long.");
            errno = 0;
            bet = strtol(input, &endptr, 10);
            // check for errors
            zinfo("Check for input range error.");
            if ((errno = ERANGE && (bet == LONG_MAX || bet == LONG_MIN)) || // returned value out of range
                    (errno != 0 && bet == 0) ||                             // ??
                    (endptr == input))                                      // no digits found in input
            {
                continue;
            }
            
            zinfo("Check bet amount is between 0 and amount of player money.");
            if ((bet >= 0) && (bet <= table->players[player].money))
            {
                table->players[player].bet = (uint32_t) bet;
                table->players[player].money -= (uint32_t) bet;
                validBet = TRUE;
            }
            else
            {
                char output[80];
                snprintf(output, 80, "Invalid amount bet. Must be between 0 and %7u.\nPress a key to try again.",
                        table->players[player].money);
                mvwaddstr(stdscr, LINES - 2, 0, output);
                wrefresh(stdscr);
                wgetch(stdscr);
            }
        }
    }
    
    noecho();
    wmove(stdscr, LINES - 3, 0);
    clrtobot();
    wrefresh(stdscr);
    
    return;
}

/***************
 *  Summary: Play the dealers hand
 *
 *  Description: Play the dealer hand by hitting if we are at 16 or less. We stand at 17 or more regardless of
 *      if it is a soft or hard count.
 *
 *  Parameter(s):
 *      dealer: Dealer struct with dealer's hand
 *      shoe:   shoe of cards to deal a card from if needed
 *
 *  Returns:
 *      N/A
 */
void play_dealer_hand(Dealer *dealer, Deck *shoe)
{
    // TODO add a pause using sleep()
    zinfo("Set dealer->faceup flag to TRUE.");
    dealer->faceup = TRUE;
    display_dealer(dealer);
    
    while (blackjack_count(dealer->hand) < 17)
    {
        dealer->hand.hand[dealer->hand.numCards++] = deal_card(shoe);
        zinfo("Dealer got dealt %s.", dealer->hand.hand[dealer->hand.numCards - 1].face);
        display_dealer(dealer);
        sleep(1);
    }
    
    zinfo("Dealer is at 17 or greater. Standing.");
    return;
}

/***************
 *  Summary: Handle the double down player option
 *
 *  Description: Player has chosen the double down option which involves dealing an extra card and doubling their bet.
 *      We need to make sure they have enough money before we allow the double down.
 *
 *  Parameter(s):
 *      player: Player struct with player's hand
 *
 *  Returns:
 *      bool:   TRUE if we could double down, FALSE if couldn't
 */
bool double_down(Player player)
{
    // check we have enough money to double down
    if (player.bet > player.money)
    {
        // todo: print error message here??
        return FALSE;
    }
    
    player.money -= player.bet;
    player.bet *=2;
    return TRUE;
}

/***************
 *  Summary: Compare player and dealer hands
 *
 *  Description: Compare the player and dealer hands and pay out or collect bets as required.
 *      Compare player hand to dealer hand to determine either player won or lost.
 *      Dealer blackjack overrides comparisions and is automatic loss for player.
 *      Player insurance pays only if dealer blackjack has occured.
 *
 *  Parameter(s):
 *      table: pointer to Table struct
 *      dealerBlackjack: boolean if dealer has blackjack
 *
 *  Returns:
 *      N/A
 */
void check_table(Table table, bool dealerBlackjack)
{
    bool playerWon;
    uint8_t playerCount;
    zinfo("Get dealer count.");
    uint8_t dealerCount = blackjack_count(table.dealer->hand);
    zinfo("Dealer has %u. Checking players hands now.", dealerCount);
    
    for (uint8_t player = 0; player < table.numPlayers; player++)
    {
        // determine if player has won or not
        playerWon = FALSE;
        if (dealerBlackjack == FALSE)   // check players hand only if dealer doesn't have blackjack
        {
            playerCount = blackjack_count(table.players[player].hand1);
            zinfo("Dealer doesn't have blackjack. Player has %u.", playerCount);
            if (playerCount <= 21)      // make sure player hasn't gone over 21
            {
                zinfo("Player hasn't busted. Comparing against dealer.");
                if (dealerCount > 21 || playerCount >= dealerCount)
                {
                    zinfo("Player won. Setting flag to TRUE.");
                    playerWon = TRUE;   // player has won only if dealer busted or we have higher count
                                        // also TRUE if playerCount equals dealerCount which is a tie
                }
            }
        }
        
        // handle pay out or collection
        if (playerWon == TRUE)
        {
            if  (playerCount == dealerCount)
            {
                zinfo("Player tied with dealer.");
                table.players[player].money += table.players[player].bet;
            }
            else
            {
                zinfo("Player won. Get twice bet back: %u.", (2 * table.players[player].bet));
                table.players[player].money += 2 * table.players[player].bet;
                if (playerCount == 21 && table.players[player].hand1.numCards == 2) // it's blackjack
                {
                    zinfo("Player has blackjack. Get half bet added: %u.", (table.players[player].bet / 2));
                    table.players[player].money += table.players[player].bet / 2;
                }
            }
        }
        
        // TODO: handle insurance bets here
        
        // reset bet amount here
        zinfo("Reset players bet back to zero.");
        table.players[player].bet = 0;
    }

    return;
}
