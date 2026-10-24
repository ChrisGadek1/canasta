#include "cardmemory.h"

void CardMemory::reset() {
    for (int playerNumber = 0; playerNumber < PLAYERS_COUNT; playerNumber++) {
        for (int value = 0; value < CARD_VALUES_COUNT; value++) {
            knownInHand[playerNumber][value] = 0;
            discardedBy[playerNumber][value] = 0;
        }
    }
}

void CardMemory::forgetCardInHand(int playerNumber, CardValue value) {
    if (knownInHand[playerNumber][value] > 0) {
        knownInHand[playerNumber][value]--;
    }
}

void CardMemory::onCardsTakenToHand(int playerNumber, const std::vector<Card*>& cards) {
    for (const auto* card : cards) {
        if (!card->isRedThree()) {
            knownInHand[playerNumber][card->value]++;
        }
    }
}

void CardMemory::onCardsMelded(int playerNumber, const std::vector<Card*>& cards) {
    for (const auto* card : cards) {
        forgetCardInHand(playerNumber, card->value);
    }
}

void CardMemory::onCardDiscarded(int playerNumber, const Card* card) {
    forgetCardInHand(playerNumber, card->value);
    discardedBy[playerNumber][card->value]++;
}

int CardMemory::getKnownInHandTotal(int playerNumber) const {
    int total = 0;
    for (int value = 0; value < CARD_VALUES_COUNT; value++) {
        total += knownInHand[playerNumber][value];
    }
    return total;
}
