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

/****************
 * DECLARATIONS *
 ****************/

/***************
 *  Summary: Instantiate one or more decks of cards
 *
 *  Description: Populates a deck_t struct of cards with the ranks and suits in order. Initial
 *               set-up for a card game before being shuffled. Also sets the number of cards and
 *               cards left to initial values to be used elsewhere as needed.
 *
 *  Parameter(s):
 *      decks: the number of decks included in the shoe
 *
 *  Returns:
 *      deck: pointer to the deck_t struct or NULL if an error
 */
Deck *init_deck(uint8_t decks)
{
    uint16_t cards = CARDS_IN_DECK * decks;

    // allocate memory
    Deck *deck = calloc(1, sizeof(Deck));
    deck->shoe = calloc(cards, sizeof(Card));
    if (!deck->shoe || !deck)
    {
        zlog_error(zc, "Memory allocation for shoe or deck failed.");
        goto error;
    }

    char *ranks[13] = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};
    char *suits[4] = {SPADE, CLUB, HEART, DIAMOND};
    uint8_t values[13] = {11, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10};

    for (uint16_t card = 0; card < cards; card++)
    {
        uint8_t cc = card % CARDS_IN_DECK;
        strcpy(deck->shoe[card].rank, ranks[cc % 13]);
        strcpy(deck->shoe[card].suit, suits[cc / 13]);
        strcpy(deck->shoe[card].face, deck->shoe[card].rank);
        strcat(deck->shoe[card].face, deck->shoe[card].suit);
        deck->shoe[card].value = values[cc % 13];
    }

    deck->cards = cards;
    deck->left = cards;

error:
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
    Card shoe_tmp;
    uint16_t swap;

    for (int card = shoe->cards - 1; card > 0; card--)
    {
        swap = rand() % card;

        shoe_tmp = shoe->shoe[swap];
        shoe->shoe[swap] = shoe->shoe[card];
        shoe->shoe[card] = shoe_tmp;
    }

    shoe->left = shoe->cards; // Re-set the cards left to the number of cards in the shoe

    return;
}

/***************
 *  Summary: Prints a shoe of cards in order.
 *
 *  Description: Prints a shoe of cards starting with the first card through the last card.
 *
 *  Parameter(s):
 *      shoe: the shoe of cards to print
 *      cards: the number of cards in the shoe
 *
 *  Returns:
 *      N/A
 */
void print_shoe(Deck *shoe)
{
    printf("Printing deck of cards:\n");

    for (uint16_t card = 0; card < shoe->cards; card++)
    {
        printf("%4s", shoe->shoe[card].face);
        printf("%s", ((card + 1) % 13) == 0 ? "\n" : ", ");
    }
    printf("\n");

    return;
}

/***************
 *  Summary: Deal a card from the shoe
 *
 *  Description: Using the supplied shoe struct, return the topmost card in the shoe. Update the
 *      counter in the struct as well.
 *
 *  Parameter(s):
 *      shoe: a deck_t struct
 *
 *  Returns:
 *      card: a card_t struct to be added to the hand of the caller
 */
Card deal_card(Deck *shoe)
{
    return shoe->shoe[shoe->cards - shoe->left--];
}

uint8_t blackjack_count(Card *hand)
{
//    card_t *emptyCard = calloc(1, sizeof(card_t));
    bool softCount = false;
    bool hasAce = false;
    uint8_t card = 0;
    uint8_t count = 0;

    while (strcmp(hand[card].rank, ""))
    {
        if (hand[card].value == 11)
        {
            if (!hasAce)
            {
                hasAce = true;
                softCount = true;
                count += 11;
            }
            else
            {
                count += 1;
            }
        }
        else
        {
            count += hand[card].value;
        }

        // check if we're over 21
        if (count > 21)
        {
            if (softCount)
            {
                softCount = false;
                count -= 10;
            }
            else
            {
                count = 0;
                break;      // player/dealer is over 21, no need to keep going so break out of loop
            }
        }

        card++;
    }
    return count;
}
