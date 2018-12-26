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
void print_hand(Hand *hand);
Player *init_player(void);
void free_player(Player *dealer);
void test_clear_hands(Hand *hand);
void test_split_hand(Hand *handToSplit, uint32_t *bank, Deck *shoe);
void test_deal_card(Deck *shoe, Hand *hand);
void test_clear_split_hands(Hand *hand);

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
    
    Player *player = init_player();
    print_hand(&player->hand);
    
    // deal 5H, 6C, 3C
    for (int i = 0; i < 3; i++)
    {
        test_deal_card(deck, &player->hand);
        print_hand(&player->hand);
        test_split_hand(&player->hand, &player->money, deck);
    }

    printf("Clearing one hand...\n");
    test_clear_split_hands(&player->hand);
    player->hand.bet = 150;
    print_hand(&player->hand);
    
    // deal Q
    test_deal_card(deck, &player->hand);
    print_hand(&player->hand);
    test_split_hand(&player->hand, &player->money, deck);
    
    // deal J
    test_deal_card(deck, &player->hand);
    print_hand(&player->hand);
    test_split_hand(&player->hand, &player->money, deck);
    
    // print hands after split (should be Q-K and J-K)
    print_hand(&player->hand);
    
    // re-split both hands
    Hand *currHand = &player->hand;
    while (currHand != NULL)
    {
        test_split_hand(currHand, &player->money, deck);
        print_hand(&player->hand);
        currHand = currHand->nextHand;
    }
    

    printf("Clearing hands...\n");
    test_clear_split_hands(&player->hand);
    print_hand(&player->hand);
    
    free_player(player);
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
void print_hand(Hand *hand)
{
    printf("Dealer hand(s) is/are:\n");
    
    Hand *handToPrint = hand;
    while (handToPrint != NULL)
    {
        CardList *cardToPrint = handToPrint->cards;
        printf("Count: %hhu - ", blackjack_count(*handToPrint));
        
        while (cardToPrint != NULL)
        {
            printf("%s ", cardToPrint->card->face);
            cardToPrint = cardToPrint->nextCard;
        }
        
        printf("\n");
        handToPrint = handToPrint->nextHand;
    }
    
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
Player *init_player(void)
{
    Player *player = calloc(1, sizeof *player);
    strncpy(player->name, "Konnor", 7);
    player->money = 500;
    player->hand.cards = NULL;
    player->hand.nextHand = NULL;
    player->hand.bet = 150;
    
    return player;
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
void free_player(Player *player)
{
//    test_clear_split_hands(&player->hand);
//    free(player->hand.cards);
//    free(player->hand.nextHand);
    free(player->name);
    return;
}

void test_clear_hands(Hand *hand)
{
    zinfo("Clearing hand of cards.");
    
        CardList *currCard, *tempCard;
        currCard = hand->cards;

        while (currCard != NULL)
        {
            tempCard = currCard->nextCard;
            free(currCard);
            currCard = tempCard;
        }
        hand->cards = NULL;
    return;
}

void test_clear_split_hands(Hand *hand)
{
    zinfo("Clearing hands of cards.");
    
    Hand *currHand = hand;
    Hand *tempHand = hand;
    
    while (currHand != NULL)
    {
        CardList *currCard, *tempCard;
        currCard = currHand->cards;

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
//        currHand = NULL;
        currHand = tempHand;
    }
    
    hand->bet = 0;
    hand->nextHand = NULL;
    
    return;
}

void test_split_hand(Hand *handToSplit, uint32_t *bank, Deck *shoe)
{
    // check if we have only one card in hand
    if (handToSplit->cards->nextCard == NULL)
    {
        zinfo("Hand to be split only has one card.");
        return;
    }
    
    // check if we have more than two cards in the hand
    if (handToSplit->cards->nextCard->nextCard != NULL)
    {
        zinfo("Hand has more than two cards.");
        return;
    }
    
    // we have only two cards, now make sure they're the same value
    if (handToSplit->cards->card->value == handToSplit->cards->nextCard->card->value)
    {
        // same value cards, do we have enough money to make the split
        if (handToSplit->bet > *bank)
        {
            zinfo("Not enough money to be able to split hands!");
        }
        else
        {
            // two cards same value and enough money, let's split the cards into two hands
            Hand *newHand = calloc(1, sizeof (Hand));
            newHand->cards = handToSplit->cards->nextCard;
            newHand->cards->nextCard = NULL;
            newHand->bet = handToSplit->bet;
            newHand->nextHand = handToSplit->nextHand;
            *bank -= handToSplit->bet;
            printf("Player has $%u left.\n", *bank);
            handToSplit->cards->nextCard = NULL;
            handToSplit->nextHand = newHand;
            zinfo("Succesfully split cards.");
            print_hand(handToSplit);
            printf("Dealing new cards.\n");
            test_deal_card(shoe, handToSplit);
            test_deal_card(shoe, newHand);
        }
    }
    else
    {
        zinfo("Cards are not the same value!");
    }
    
    return;
}

void test_deal_card(Deck *shoe, Hand *hand)
{
    // allocate new CardList and put new card in it
    CardList *newCard = calloc(1, sizeof(CardList));
    newCard->card = &shoe->shoe[shoe->deal++];
    newCard->nextCard = NULL;

    CardList *currCard = hand->cards;
    
    if (currCard == NULL)
    {
        hand->cards = newCard;
    }
    else
    {
        while (currCard->nextCard != NULL)
        {
            currCard = currCard->nextCard;
        }

        currCard->nextCard = newCard;
    }
    return;

}