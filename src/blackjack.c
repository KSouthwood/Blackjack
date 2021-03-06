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
#include "blackjack.h"

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

/***********
 * DEFINES *
 ***********/
enum errorCode {NO_ERROR, ERR_PLAYER_ALLOC, ERR_DEALER_ALLOC, ERR_DECK_ALLOC, PLAYER_QUIT};

/****************
 * DECLARATIONS *
 ****************/
uint8_t setup_table(Table *table);
uint8_t get_num_players();
Player *init_players(uint8_t num_players);
Dealer *init_dealer();
void play_game(Table *table);
void deal_hands(Table *table);
bool check_dealer_hand(Table *table);
void play_hands(Table *table);
bool get_bets(Table *table);
void play_dealer_hand(Dealer *dealer, Deck *shoe, WINDOW* msgWin);
bool double_down(Player *player, Hand *hand, WINDOW* msgWin);
bool check_table(Table table, bool dealerBlackjack);
void clear_hands(Hand *hand);
bool split_hand(Hand *handToSplit, uint32_t *bank, Deck *shoe);

int main()
{
    // seed random number generator
#if DEBUG
    srandom(1968);
#else
    srandom(time(NULL));
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
            case ERR_DECK_ALLOC:
                zlog_debug(zc, "Freeing table->dealer: %p.", table->dealer);
                free(table->dealer);
            case ERR_DEALER_ALLOC:
                zlog_debug(zc, "Freeing table->players: %p.", table->players);
                free(table->players);
            case ERR_PLAYER_ALLOC:
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
        return ERR_PLAYER_ALLOC;
    }
    
    table->dealer = init_dealer();
    zdebug("table->dealer pointer: %p.", table->dealer);
    if (!table->dealer)
        {
            zerror("Couldn't allocate memory for dealer struct.");
            return ERR_DEALER_ALLOC;
        }

    uint8_t decks = (table->numPlayers < 4) ? 1 : 1; // determine how many decks in the shoe; TODO: Change to 4 : 6
    zinfo("Instantiating %i decks for %i players.", decks, table->numPlayers);
    table->shoe = init_deck(decks);
    zdebug("table->shoe pointer: %p.", table->shoe);
    if(!table->shoe)
    {
        zerror("Couldn't allocate memory for deck of cards.");
        return ERR_DECK_ALLOC;
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
            players[ii].hand.cards = NULL;
            players[ii].hand.bet = 0;
            players[ii].hand.nextHand = NULL;
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
        dealer->hand.cards = NULL;
        dealer->hand.bet = 0;
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
    bool gameOver = FALSE;
    while (!gameOver)
    {
        // Reset players & dealer number of cards to 0
        for (uint8_t i = 0; i < table->numPlayers; i++)
        {
            zinfo("Clearing hands for %s", table->players[i].name);
            clear_hands(&table->players[i].hand);
        }
        clear_hands(&table->dealer->hand);
        table->dealer->faceup = FALSE;

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
        if ((gameOver = get_bets(table))) continue;
        zinfo("Calling deal_hands for initial deal.");
        deal_hands(table);
        zinfo("Check dealer hand for blackjack.");
        if (check_dealer_hand(table) == FALSE)  // if dealer has blackjack we skip the players turns
        {
            play_hands(table);
            play_dealer_hand(table->dealer, table->shoe, table->msgWin);
        }
        zinfo("******************************");
        gameOver = check_table(*table, FALSE);
        zinfo("******************************");
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
            deal_card(table->shoe, &table->players[i].hand);
        }
        deal_card(table->shoe, &table->dealer->hand);
    }

//    zinfo("Set dealer faceup flag to false. Returning.");
//    table->dealer->faceup = false;

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
 *		bool: TRUE if the dealer has blackjack, FALSE otherwise
 */
bool check_dealer_hand(Table *table)
{
    Dealer dealer = *table->dealer;
//    WINDOW msgWin = *table->msgWin;
    
    if (dealer.hand.cards == NULL)
    {
        zerror("No dealer cards to check!");
        return FALSE;
    }
    
    // if our upcard is not a face card or Ace, no need to run the checks
    if (dealer.hand.cards->nextCard->card->value < 10)
    {
        zinfo("Upcard is not an Ace or face card. Exiting check.");
        return FALSE;
    }
    
    // check for Ace in dealer upcard and offer insurance if it is
    if (!strcmp(dealer.hand.cards->nextCard->card->rank, " A"))
    {
        zinfo("Dealer is showing an Ace.");
        print_message(table->msgWin, "Dealer is showing an Ace. Offering insurance bets.");
        //TODO offer players insurance
    }

    // No Ace or we've offered insurance, now check if we have blackjack
    if (blackjack_count(dealer.hand) == 21)
    {
        zinfo("Dealer has blackjack. Players lose.");
        print_message(table->msgWin, "Dealer has blackjack! Everybody loses.");
        return TRUE;
    }

    return FALSE;
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
    struct timespec sleep = {.tv_nsec = 500000000, .tv_sec = 0};
    struct timespec remain;
    for (uint8_t player = 0; player < table->numPlayers; player++)
    {
        Player *currentPlayer = &table->players[player];
        Hand *currentHand = &currentPlayer->hand;
        
        while (currentHand != NULL)
        {
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
                        deal_card(table->shoe, currentHand);
                        if (blackjack_count(*currentHand) > 21)
                        {
                            print_message(table->msgWin, "You've busted!");
                            playHand = FALSE;    // player busted
                        }
                        display_player(currentPlayer);
                        break;
                    case DOUBLE:
                        if (double_down(currentPlayer, currentHand, table->msgWin))
                        {
                            deal_card(table->shoe, currentHand);
                            playHand = FALSE;
                        }
                        display_player(currentPlayer);
                        break;
                    case SPLIT:
                        if (split_hand(currentHand, &currentPlayer->money, table->shoe))
                        {
                            print_message(table->msgWin, "Hand split successfully.");
                        }
                        else
                        {
                            print_message(table->msgWin, "Hand not split. Cards not same value or not enough money.");
                        }
                        display_player(currentPlayer);
                        break;
                    default:
                        // no default case
                        break;
                }
                nanosleep(&sleep, &remain);
            }
        currentHand = currentHand->nextHand;
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
bool get_bets(Table *table)
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
            snprintf(msg, sizeof(msg), "%s, how much money do you wish to bet? ('q' to quit.) ", table->players[player].name);
            print_message(table->msgWin, msg);
            wgetnstr(table->msgWin, input, 7);
            
            // check if player wants to quit
            if (tolower(input[0]) == 'q')
            {
                table->numPlayers--;
                validBet = TRUE;
                continue;
            }
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
                table->players[player].hand.bet = (uint32_t) bet;
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
    return (table->numPlayers == 0);
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
    zinfo("Set dealer->faceup flag to TRUE.");
    dealer->faceup = TRUE;
    display_dealer(dealer);
    struct timespec sleep = {.tv_nsec = 500000000, .tv_sec = 0};
    struct timespec remain;
    
    while (blackjack_count(dealer->hand) < 17)
    {
        print_message(msgWin, "Dealer hits.");
        deal_card(shoe, &dealer->hand);
        display_dealer(dealer);
        nanosleep(&sleep, &remain);
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
bool double_down(Player *player, Hand *hand, WINDOW* msgWin)
{
    // check we have enough money to double down
    if (hand->bet > player->money)
    {
        print_message(msgWin, "Not enough money to double down! Choose another option.");
        return FALSE;
    }
    
    player->money -= hand->bet;
    hand->bet *=2;
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
bool check_table(Table table, bool dealerBlackjack)
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
        Hand *currentHand = &table.players[player].hand;
        while (currentHand != NULL)
        {
            // determine if player has won or not
            playerWon = FALSE;
            if (dealerBlackjack == FALSE)   // check players hand only if dealer doesn't have blackjack
            {
                playerCount = blackjack_count(*currentHand);
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
                             table.players[player].name, currentHand->bet);
                    print_message(table.msgWin, msg);
                    zinfo("Player tied with dealer.");
                    table.players[player].money += currentHand->bet;
                }
                else
                {
                    if (playerCount == 21 && (currentHand->cards->nextCard->nextCard == NULL)) // it's blackjack
                    {
                        moneyWon = currentHand->bet * 2.5;
                        snprintf(msg, sizeof(msg), "%s has blackjack! You win %u.", table.players[player].name, moneyWon);
                        print_message(table.msgWin, msg);
                        zinfo("Player has blackjack. Get half bet added: %u.", moneyWon);
                        table.players[player].money += moneyWon;
                    }
                    else
                    {
                        moneyWon = currentHand->bet * 2;
                        snprintf(msg, sizeof(msg), "%s wins %u.", table.players[player].name, moneyWon);
                        print_message(table.msgWin, msg);
                        zinfo("Player won. Get twice bet back: %u.", moneyWon);
                        table.players[player].money += moneyWon;
                    }
                }
            }
            // TODO: handle insurance bets here

            currentHand = currentHand->nextHand;
        }
//        // reset bet amount here
//        zinfo("Reset players bet back to zero.");
//        table.players[player].hand.bet = 0;
        
        if (table.players[player].money == 0)
        {
            table.numPlayers--;
        }
    }

    return (table.numPlayers == 0);
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
void clear_hands(Hand *hand)
{
    Hand *currHand = hand;
    Hand *tempHand = NULL;
    
    while (currHand != NULL)
    {
        CardList *currCard = currHand->cards;
        CardList *tempCard = NULL;
        
        while (currCard != NULL)
        {
            tempCard = currCard->nextCard;
            free(currCard);
            currCard = tempCard;
        }
        
        currHand->cards = NULL;
        tempHand = currHand->nextHand;
        if (currHand != hand)
        {
            free(currHand);
        }
        currHand = tempHand;
    }
    
    hand->bet = 0;
    hand->nextHand = NULL;
    
    return;
}

/***************
 *  Summary: Offer insurance bets to the players
 *
 *  Description: Ask each player in turn if they'd like the insurance bet when the dealer is showing an Ace for an
 *      upcard. Deducts the bet from each player if they have enough money otherwise we move on.
 *
 *  Parameter(s):
 *      N/A
 *
 *  Returns:
 *      N/A
 */
void offer_insurance()
{
    return;
}

/***************
 *  Summary: Split a hand of two cards of same value into two hands
 *
 *  Description: Splits a hand of two cards into two hands. There must be two cards only of the same value, and the
 *      player must have enough money to replicate the bet of the hand being split. (The new hand will have the same bet
 *      as the hand being split.)
 *
 *  Parameter(s):
 *      hand - Hand struct to be split
 *
 *  Returns:
 *      N/A
 */
bool split_hand(Hand *handToSplit, uint32_t *bank, Deck *shoe)
{
    // check if we have one card in the hand
    if (handToSplit->cards->nextCard == NULL)
    {
        zerror("Hand to be split only has one card.");
        return FALSE;
    }
    
    // check if we have more than two cards in the hand
    if(handToSplit->cards->nextCard->nextCard != NULL)
    {
        zerror("Hand has more than two cards.");
        return FALSE;
    }
    
    // we have only two cards so make sure they're the same value
    if (handToSplit->cards->card->value == handToSplit->cards->nextCard->card->value)
    {
        // cards have same value, do we have enough money to cover the additional bet
        if (handToSplit->bet < *bank)
        {
            Hand *newHand = calloc(1, sizeof(Hand));
            newHand->cards = handToSplit->cards->nextCard;
            newHand->cards->nextCard = NULL;
            newHand->bet = handToSplit->bet;
            newHand->nextHand = handToSplit->nextHand;
            handToSplit->cards->nextCard = NULL;
            handToSplit->nextHand = newHand;
            deal_card(shoe, handToSplit);
            deal_card(shoe, newHand);
            *bank -= handToSplit->bet;
            return TRUE;
        }
        else
        {
            zinfo("Not enough money to split cards.");
            return FALSE;
        }
    }
    
    zinfo("Cards are not the same value!");
    return FALSE;
}