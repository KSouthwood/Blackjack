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
 *  deck_of_cards.c
 *
 *  Created on: Oct 11, 2018
 *      Author: Keri Southwood-Smith
 *
 *  Description: C module to instantiate a deck of cards (one or more as needed) and
 *               shuffle and deal as well.
 */


/************
 * INCLUDES *
 ************/
#include "deck_of_cards.h"

/***********
 * DEFINES *
 ***********/

/****************
 * DECLARATIONS *
 ****************/

/***************
 *  Summary: Instantiate one or more decks of cards
 *
 *  Description: Populates a Deck struct of cards with the ranks and suits in order. Initial
 *               set-up for a card game before being shuffled. Also sets the number of cards and
 *               card to be dealt to initial values to be used elsewhere as needed.
 *
 *  Parameter(s):
 *      decks: the number of decks included in the shoe
 *
 *  Returns:
 *      deck: pointer to the Deck struct or NULL if an error
 */
Deck *init_deck(uint8_t decks)
{
    log_call();
    uint16_t cards = decks * CARDS_IN_DECK;
    
    // allocate memory
    Deck *deck = calloc(1, sizeof(Deck));
    deck->shoe = calloc(cards, sizeof(Card));
    if (deck == NULL || deck->shoe == NULL)
    {
        zerror("Deck memory allocation failed.");
        return NULL;
    }
    
    char *ranks[13] = {" A", " 2", " 3", " 4", " 5", " 6", " 7", " 8", " 9", "10", " J", " Q", " K"};
    char *suits[4] =  {SPADE, CLUB, HEART, DIAMOND};
    uint8_t values[13] = {11, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10};
    
    for (uint16_t card = 0; card < cards; card++)
    {
        uint16_t cc = card % CARDS_IN_DECK;
        strncpy(deck->shoe[card].rank, ranks[cc % 13], 3);
        strncpy(deck->shoe[card].suit, suits[cc / 13], 4);
        strncpy(deck->shoe[card].face, ranks[cc % 13], 3);
        strncat(deck->shoe[card].face, suits[cc / 13], 4);
        deck->shoe[card].value = values[cc % 13];
    }
    
    deck->cards = cards;
    deck->deal = 0;
    deck->cut = 0;
    deck->shuffle = true;
    
    return deck;
}

/***************
 *  Summary: Shuffle a shoe of cards
 *
 *  Description: Using the Fisher-Yates algorithm, shuffle a shoe of cards consisting of one or
 *      more decks of cards.
 *
 *  Parameter(s):
 *      shoe: pointer to a shoe of cards
 *
 *  Returns:
 *      N/A
 */
void shuffle_cards(Deck *shoe)
{
    log_call();
    Card shoe_tmp;
    uint16_t swap;

    for (int card = shoe->cards - 1; card > 0; card--)
    {
        swap = random() % card;

        shoe_tmp = shoe->shoe[swap];
        shoe->shoe[swap] = shoe->shoe[card];
        shoe->shoe[card] = shoe_tmp;
    }

    shoe->deal = 0; // Set the card to be dealt to the first card
    shoe->cut = shoe->cards * 0.8;  // TODO: Make more random
    shoe->shuffle = false;
    return;
}

/***************
 *  Summary: Deal a card from the shoe
 *
 *  Description: Using the supplied shoe Deck struct, add the next card to be dealt onto the end of the hand Hand struct
 *      linked list. Also increments the shoe.deal counter.
 *
 *  Parameter(s):
 *      shoe: a Deck struct
 *      hand: a Hand struct
 *  Returns:
 *      N/A
 */
void deal_card(Deck *shoe, Hand *hand)
{
    log_call();
    // allocate new node and assign the next card in the deck to it
    CardList *newCard = calloc(1, sizeof(CardList));
    newCard->card = &shoe->shoe[shoe->deal++];
    newCard->nextCard = NULL;
    
    CardList *currCard = hand->cards;
    
    // add the newCard to the root node if there are no cards in the hand
    if (currCard == NULL)
    {
        hand->cards = newCard;
    }
    else    // otherwise add it to the end of cards
    {
        // move to the end of the linked list of cards
        while (currCard->nextCard != NULL)
        {
            currCard = currCard->nextCard;
        }

        currCard->nextCard = newCard;
    }
    
    zinfo("Card dealt is: %s", newCard->card->face);
    if (shoe->deal == shoe->cut)
    {
        shoe->shuffle = true;
        zinfo("Shuffle flag set to true.");
    }
    return;
}

/***************
 *  Summary: Return the total of the cards in a hand
 *
 *  Description: Adds up the total value of the cards in a supplied Hand struct accounting for variability of
 *      the value of Aces
 *
 *  Parameter(s):
 *      hand - Hand struct of cards to be counted
 *
 *  Returns:
 *      N/A
 */
uint8_t blackjack_score(Hand hand)
{
    log_call();
    if (hand.cards == NULL) return 0;
    zdebug("Initialize blackjack_count...");
    bool softCount = false;
    bool hasAce = false;
    uint8_t count = 0;
    CardList *countCard = hand.cards;
    if (hand.cards->card != NULL)
    {
        while (countCard != NULL)
        {
            zdebug("Card: %s, Count before: %2u.", countCard->card->face, count);
            if (countCard->card->value == 11)
            {
                zdebug("Ace found.");
                if(!hasAce)
                {
                    zdebug("First Ace in hand, setting hasAce and softCount to true.");
                    hasAce = true;
                    softCount = true;
                    count += 11;
                }
                else
                {
                    zdebug("Second or more Ace, adding 1.");
                    count += 1;
                }
            }
            else
            {
                zdebug("Adding face value.");
                count += countCard->card->value;
            }

            // check if we're over 21 with softCount true
            if ((count > 21) && softCount)
            {
                zdebug("softCount is true, set to false and subtract 10 from count.");
                softCount = false;
                count -= 10;
            }

            countCard = countCard->nextCard;
        }
    }
    zdebug("Final count: %u", count);
    return count;
}
