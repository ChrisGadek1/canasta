#ifndef CARDMEMORY_H
#define CARDMEMORY_H

#include "card.h"
#include "cardvalue.h"
#include <vector>

class CardMemory
{
public:
    static constexpr int PLAYERS_COUNT = 4;
    static constexpr int CARD_VALUES_COUNT = CardValue::JOKER + 1;
private:
    int knownInHand[PLAYERS_COUNT][CARD_VALUES_COUNT];
    int discardedBy[PLAYERS_COUNT][CARD_VALUES_COUNT];
    void forgetCardInHand(int playerNumber, CardValue value);
public:
    CardMemory() { reset(); }
    void reset();
    void onCardsTakenToHand(int playerNumber, const std::vector<Card*>& cards);
    void onCardsMelded(int playerNumber, const std::vector<Card*>& cards);
    void onCardDiscarded(int playerNumber, const Card* card);
    int getKnownInHand(int playerNumber, CardValue value) const { return knownInHand[playerNumber][value]; }
    int getKnownInHandTotal(int playerNumber) const;
    bool hasDiscarded(int playerNumber, CardValue value) const { return discardedBy[playerNumber][value] > 0; }
};

#endif // CARDMEMORY_H
