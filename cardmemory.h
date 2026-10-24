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
    std::vector<int> knownInHandTurns[PLAYERS_COUNT][CARD_VALUES_COUNT];
    std::vector<int> discardTurns[PLAYERS_COUNT][CARD_VALUES_COUNT];
    void forgetCardInHand(int playerNumber, CardValue value);
    static int countSince(const std::vector<int>& turns, int sinceTurn);
public:
    void reset();
    void onCardsTakenToHand(int playerNumber, const std::vector<Card*>& cards, int turnNumber);
    void onCardsMelded(int playerNumber, const std::vector<Card*>& cards);
    void onCardDiscarded(int playerNumber, const Card* card, int turnNumber);
    int getKnownInHand(int playerNumber, CardValue value, int sinceTurn = 0) const { return countSince(knownInHandTurns[playerNumber][value], sinceTurn); }
    int getKnownInHandTotal(int playerNumber, int sinceTurn = 0) const;
    bool hasDiscarded(int playerNumber, CardValue value, int sinceTurn = 0) const { return countSince(discardTurns[playerNumber][value], sinceTurn) > 0; }
};

#endif // CARDMEMORY_H
