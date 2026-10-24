#include "cardmemory.h"
#include <algorithm>

void CardMemory::reset() {
    for (int playerNumber = 0; playerNumber < PLAYERS_COUNT; playerNumber++) {
        for (int value = 0; value < CARD_VALUES_COUNT; value++) {
            knownInHandTurns[playerNumber][value].clear();
            discardTurns[playerNumber][value].clear();
        }
    }
}

int CardMemory::countSince(const std::vector<int>& turns, int sinceTurn) {
    return static_cast<int>(std::count_if(turns.begin(), turns.end(), [sinceTurn](int turn) { return turn >= sinceTurn; }));
}

void CardMemory::forgetCardInHand(int playerNumber, CardValue value) {
    std::vector<int>& turns = knownInHandTurns[playerNumber][value];
    if (!turns.empty()) {
        turns.erase(turns.begin());
    }
}

void CardMemory::onCardsTakenToHand(int playerNumber, const std::vector<Card*>& cards, int turnNumber) {
    for (const auto* card : cards) {
        if (!card->isRedThree()) {
            knownInHandTurns[playerNumber][card->value].push_back(turnNumber);
        }
    }
}

void CardMemory::onCardsMelded(int playerNumber, const std::vector<Card*>& cards) {
    for (const auto* card : cards) {
        forgetCardInHand(playerNumber, card->value);
    }
}

void CardMemory::onCardDiscarded(int playerNumber, const Card* card, int turnNumber) {
    forgetCardInHand(playerNumber, card->value);
    discardTurns[playerNumber][card->value].push_back(turnNumber);
}

int CardMemory::getKnownInHandTotal(int playerNumber, int sinceTurn) const {
    int total = 0;
    for (int value = 0; value < CARD_VALUES_COUNT; value++) {
        total += getKnownInHand(playerNumber, static_cast<CardValue>(value), sinceTurn);
    }
    return total;
}
