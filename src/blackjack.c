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
#include <locale.h>

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
void play_dealer_hand(Dealer *dealer, Deck *shoe, WINDOW* msgWin);
bool double_down(Player player, WINDOW* msgWin);
void check_table(Table table, bool dealerBlackjack);
void clear_hands(Hand hand);

int main()
{
    // seed random number generator
#if DEBUG
    srand(1968);
#else
    srand(time(NULL));
#endif
    setlocale(LC_ALL, "");

    if (init_zlog("blackjack.conf", "log")) return ERR;

    init_window();
    zinfo("ncurses initialized.");
    welcome_screen();
    zinfo("Welcome screen displayed.");

    // create table struct
    Table *table = calloc(1, sizeof(Table));
    zdebug("table pointer: %p.", table);
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
                zinfo("Calling play game with table->numPlayers: %i, table->players: %p, table->dealer: %p.",
                        table->numPlayers, table->players, table->dealer);
                play_game(table);
                zdebug("Freeing message window: %p.", table->msgWin);
                delwin(table->msgWin);
                zdebug("Freeing table->shoe->shoe: %p.", table->shoe->shoe);
                free(table->shoe->shoe);
                zdebug("Freeing table->shoe: %p.", table->shoe);
                free(table->shoe);
            case DECK_ALLOC:
                zlog_debug(zc, "Freeing table->dealer: %p.", table->dealer);
                free(table->dealer);
            case DEALER_ALLOC:
                zlog_debug(zc, "Freeing table->players: %p.", table->players);
                free(table->players);
            case PLAYER_ALLOC:
            case PLAYER_QUIT:
                zdebug("Freeing table: %p.", table);
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
    zdebug("table->players pointer: %p.", table->players);
    if (!table->players)
    {
        zerror("Couldn't allocate memory for players array.");
        return PLAYER_ALLOC;
    }
    
    table->dealer = init_dealer();
    zdebug("table->dealer pointer: %p.", table->dealer);
    if (!table->dealer)
        {
            zerror("Couldn't allocate memory for dealer struct.");
            return DEALER_ALLOC;
        }

    uint8_t decks = (table->numPlayers < 4) ? 4 : 6; // determine how many decks in the shoe
    zinfo("Instantiating %i decks for %i players.", decks, table->numPlayers);
    table->shoe = init_deck(decks);
    zdebug("table->shoe pointer: %p.", table->shoe);
    if(!table->shoe)
    {
        zerror("Couldn't allocate memory for deck of cards.");
        return DECK_ALLOC;
    }
    
    table->msgWin = init_message_window();
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
            numOfPlayers = (atoi(&input) > 5) ? 0 : 1; // TODO change 1 back to atoi(&input) when ready
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
    zdebug("players pointer: %p.", players);
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
            players[ii].hand1.cards = calloc(1, sizeof(CardList));
            players[ii].hand2.cards = calloc(1, sizeof(CardList));
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
    zdebug("Dealer pointer: %p", dealer);

    if (dealer)
    {
        strncpy(dealer->name, "Dealer", 7);
        dealer->faceup = FALSE;
        dealer->hand.cards = calloc(1, sizeof(CardList));
        dealer->hand.nextHand = NULL;
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
    zinfo("play_game called with num_players: %i, players: %p, dealer: %p.",
            table->numPlayers, table->players, table->dealer);
    zinfo("Shuffling shoe.");
    shuffle_cards(table->shoe);
    print_message(table->msgWin, "Shuffling the shoe.");

    zdebug("Setting game_over flag and starting game loop.");
    bool game_over = FALSE;
    uint8_t hands = 5;  // TODO: remove when ready to play fully
    while (!game_over)
    {
        // Reset players & dealer number of cards to 0
        for (uint8_t i = 0; i < table->numPlayers; i++)
        {
            zinfo("Clearing hands for %s", table->players[i].name);
            clear_hands(table->players[i].hand1);
            clear_hands(table->players[i].hand2);
        }
        clear_hands(table->dealer->hand);

        // Display the windows for players & dealer
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
        play_dealer_hand(table->dealer, table->shoe, table->msgWin);
        zinfo("******************************");
        check_table(*table, FALSE);
        zinfo("******************************");
        game_over = (--hands) ? FALSE : TRUE; // TODO: remove when ready to play fully
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
 *  	table: Table struct containing everything
 *
 *	Returns:
 *		N/A
 */
void deal_hands(Table *table)
{
    // re-shuffle the deck if we're nearing the end
    // TODO Get rid of magic number. Switch to calculated random point or just use a #define
    if ((table->shoe->cards - table->shoe->deal) < (table->shoe->cards * 0.2))
    {
        zinfo("Re-shuffling deck.");
        print_message(table->msgWin, "Re-shuffling the deck.");
        shuffle_cards(table->shoe);
    }

    print_message(table->msgWin, "Dealing cards.");
    
    // deal two cards to players and dealer
    for (uint8_t c = 0; c < 2; c++)
    {
        for (uint8_t i = 0; i < table->numPlayers; i++)
        {
            deal_card(table->shoe, &table->players[i].hand1);
        }
        deal_card(table->shoe, &table->dealer->hand);
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
    if (dealer->hand.cards == NULL)
    {
        zerror("No dealer cards to check!");
        return;
    }
    
    // if our upcard is not a face card or Ace, no need to run the checks
    if (dealer->hand.cards->nextCard->card->value < 10)
//    if (dealer->hand.cards[1].value < 10)
    {
        zinfo("Upcard is not an Ace or face card. Exiting check.");
        return;
    }
    
    // check for Ace in dealer upcard and offer insurance if it is
    if (!strcmp(dealer->hand.cards->nextCard->card->rank, " A"))
//    if (!strcmp(dealer->hand.cards[1].rank, "A"))
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
            switch(get_player_choice(currentPlayer, table->msgWin))
            {
                case STAND:
                    playHand = FALSE;
                    break;
                case HIT:
                    // get a new card
                    deal_card(table->shoe, &currentPlayer->hand1);
                    if (blackjack_count(currentPlayer->hand1) > 21)
                    {
                        print_message(table->msgWin, "You've busted!");
                        playHand = FALSE;    // player busted
                    }
                    display_player(currentPlayer);
                    break;
                case DOUBLE:
                    if (double_down(*currentPlayer, table->msgWin))
                    {
                        deal_card(table->shoe, &currentPlayer->hand1);
                        playHand = FALSE;
                    }
                    display_player(currentPlayer);
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
    char msg[80];
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
            snprintf(msg, sizeof(msg), "%s, how much money do you wish to bet? ", table->players[player].name);
            print_message(table->msgWin, msg);
            wgetnstr(table->msgWin, input, 7);
            
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
                snprintf(output, 80, "Invalid amount bet. Must be between 0 and %u. Press a key to try again.",
                        table->players[player].money);
                print_message(table->msgWin, output);
            }
        }
    }
    
    noecho();
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
void play_dealer_hand(Dealer *dealer, Deck *shoe, WINDOW* msgWin)
{
    // TODO add a pause using sleep()
    zinfo("Set dealer->faceup flag to TRUE.");
    dealer->faceup = TRUE;
    display_dealer(dealer);
    
    while (blackjack_count(dealer->hand) < 17)
    {
        print_message(msgWin, "Dealer hits.");
        deal_card(shoe, &dealer->hand);
        display_dealer(dealer);
        sleep(1);
    }
    
    print_message(msgWin, "Dealer stands.");
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
bool double_down(Player player, WINDOW* msgWin)
{
    // check we have enough money to double down
    if (player.bet > player.money)
    {
        print_message(msgWin, "Not enough money to double down! Choose another option.");
        return FALSE;
    }
    
    player.money -= player.bet;
    player.bet *=2;
    print_message(msgWin, "Doubling bet.");
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
    char msg[80];
    zinfo("Get dealer count.");
    uint8_t dealerCount = blackjack_count(table.dealer->hand);
    snprintf(msg, sizeof(msg), "Dealer has %u.", dealerCount);
    print_message(table.msgWin, msg);
    zinfo("Dealer has %u. Checking players hands now.", dealerCount);
    
    for (uint8_t player = 0; player < table.numPlayers; player++)
    {
        // determine if player has won or not
        playerWon = FALSE;
        if (dealerBlackjack == FALSE)   // check players hand only if dealer doesn't have blackjack
        {
            playerCount = blackjack_count(table.players[player].hand1);
            snprintf(msg, sizeof(msg), "%s has %u.", table.players[player].name, playerCount);
            print_message(table.msgWin, msg);
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
            else
            {
                snprintf(msg, sizeof(msg), "%s has busted.", table.players[player].name);
                print_message(table.msgWin, msg);
            }
        }
        
        // handle pay out or collection
        if (playerWon == TRUE)
        {
            uint32_t moneyWon = 0;
            if  (playerCount == dealerCount)
            {
                snprintf(msg, sizeof(msg), "%s tied with dealer. Get your bet of %u back.",
                         table.players[player].name, table.players[player].bet);
                print_message(table.msgWin, msg);
                zinfo("Player tied with dealer.");
                table.players[player].money += table.players[player].bet;
            }
            else
            {
                if (playerCount == 21 && (table.players[player].hand1.cards->nextCard->nextCard == NULL)) // it's blackjack
                {
                    moneyWon = table.players[player].bet * 2.5;
                    snprintf(msg, sizeof(msg), "%s has blackjack! You win %u.", table.players[player].name, moneyWon);
                    print_message(table.msgWin, msg);
                    zinfo("Player has blackjack. Get half bet added: %u.", moneyWon);
                    table.players[player].money += moneyWon;
                }
                else
                {
                    moneyWon = table.players[player].bet * 2;
                    snprintf(msg, sizeof(msg), "%s wins %u.", table.players[player].name, moneyWon);
                    print_message(table.msgWin, msg);
                    zinfo("Player won. Get twice bet back: %u.", moneyWon);
                    table.players[player].money += moneyWon;
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

/***************
 *  Summary: Clear the hand of cards
 *
 *  Description: Clear the hand of cards, setting to NULL or freeing as needed.
 *
 *  Parameter(s):
 *      hand: Hand struct of the hand to clear
 *
 *  Returns:
 *      N/A
 */
void clear_hands(Hand hand)
{
    if (hand.cards != NULL)
    {
        CardList *currCard, *tempCard;
        currCard = hand.cards->nextCard;
        hand.cards->nextCard = NULL;
        hand.cards->card = NULL;
        
        while (currCard != NULL)
        {
            tempCard = currCard->nextCard;
            free(currCard);
            currCard = tempCard;
        }
    }
    
    return;
}