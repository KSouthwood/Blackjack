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
 *  test_blackjack.c
 *
 *  Created on: Nov 25, 2018
 *      Author: Keri Southwood-Smith
 *
 *  Description: Test suite for the Blackjack program (just the deck_of_cards module).
 */

/************
 * INCLUDES *
 ************/
#include "../src/blackjack.h"

#include <stdio.h>
#include <locale.h>
#include <stdlib.h>

#include "../src/logger.h"
#include "../src/deck_of_cards.h"

/***********
 * DEFINES *
 ***********/

/****************
 * DECLARATIONS *
 ****************/
void init_test_deck();
void print_shoe(Deck *shoe);
void print_hand(Hand hand);
Dealer *init_dealer(void);
void free_dealer(Dealer *dealer);
void test_clear_hands(Hand hand);

int main(void)
{
    setlocale(LC_ALL, "");
    srandom(1968);
    
    // Initialize logging
    if (init_zlog("test_blackjack.conf", "test_cat"))
    {
        return EXIT_FAILURE;
    }
    
    init_test_deck();
    
    end_zlog();
    return 0;
}

/***************
 *  Summary: Instantiate, shuffle and deal from a deck of cards
 *
 *  Description: Instantiates then shuffles a deck of cards before dealing five cards from the deck. Print out the deck
 *      and the dealt hand after each step.
 *
 *  Parameter(s):
 *      N/A
 *
 *  Returns:
 *      N/A
 */
void init_test_deck()
{
    Deck *deck = init_deck(1);
    print_shoe(deck);
    
    printf("Shuffling cards...\n");
    shuffle_cards(deck);
    print_shoe(deck);
    
    Dealer *dealer = init_dealer();
    print_hand(dealer->hand);
    deal_card(deck, &dealer->hand);
    print_hand(dealer->hand);
    deal_card(deck, &dealer->hand);
    print_hand(dealer->hand);
    printf("Clearing hands...\n");
    test_clear_hands(dealer->hand);
    print_hand(dealer->hand);
    deal_card(deck, &dealer->hand);
    print_hand(dealer->hand);
    deal_card(deck, &dealer->hand);
    print_hand(dealer->hand);
    deal_card(deck, &dealer->hand);
    print_hand(dealer->hand);
    printf("Clearing hands...\n");
    test_clear_hands(dealer->hand);
    print_hand(dealer->hand);
    
    free_dealer(dealer);
    free(deck->shoe);
    free(deck);
    return;
}

/***************
 *  Summary: Print a shoe of cards
 *
 *  Description: Use a for loop to go through an array of Card's printing out each one inserting a newline every
 *      thirteen cards.
 *
 *  Parameter(s):
 *      shoe - the shoe (multiple decks) of cards to print out
 *
 *  Returns:
 *      N/A
 */
void print_shoe(Deck *shoe)
{
    printf("Printing deck of cards:\n");

    for (uint16_t card = 0; card < shoe->cards; card++)
    {
        printf("%5s", shoe->shoe[card].face);
        printf("%s", ((card + 1) % 13) == 0 ? "\n" : ", ");
    }
    printf("\n");

    return;
}

/***************
 *  Summary: Print a hand of cards.
 *
 *  Description: Print out a hand of cards following the pointers until the end is reached.
 *
 *  Parameter(s):
 *      hand - a Hand struct containing the cards to be printed
 *
 *  Returns:
 *      N/A
 */
void print_hand(Hand hand)
{
    printf("Dealer hand is: ");

    CardList *printCard = hand.cards;
    
    if (hand.cards->card != NULL)
    {
        while (printCard != NULL)
        {
            printf("%s ", printCard->card->face);
            printCard = printCard->nextCard;
        }
    }
    
    printf(" - Count: %hhu\n", blackjack_count(hand));
    return;
}

/***************
 *  Summary: Instantiate the dealer struct
 *
 *  Description: Instantiate a dealer struct and initalize the fields with default values for testing
 *
 *  Parameter(s):
 *      N/A
 *
 *  Returns:
 *      dealer - pointer to the Dealer struct
 */
Dealer *init_dealer(void)
{
    Dealer *dealer = calloc(1, sizeof(Dealer));
    strncpy(dealer->name, "Dealer", 7);
    dealer->faceup = FALSE;
    dealer->hand.cards = calloc(1, sizeof(CardList));
    dealer->hand.nextHand = NULL;
    
    return dealer;
}

/***************
 *  Summary: Free the memory of a Dealer struct
 *
 *  Description: Take a Dealer struct and free the memory associated with its members
 *
 *  Parameter(s):
 *      dealer - a pointer to a Dealer struct
 *
 *  Returns:
 *      N/A
 */
void free_dealer(Dealer *dealer)
{
    // traverse the card list if any cards
    if (dealer->hand.cards != NULL)
    {
        CardList *currCard, *tempCard;
        currCard = dealer->hand.cards->nextCard;
        dealer->hand.cards->nextCard = NULL;
        
        while (currCard != NULL)
        {
            tempCard = currCard->nextCard;
            free(currCard);
            currCard = tempCard;
        }
    }
    free(dealer->hand.cards);
    free(dealer->hand.nextHand);
    free(dealer->name);
    return;
}

void test_clear_hands(Hand hand)
{
    zinfo("Clearing hand of cards.");
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