#include "meld.h"
#include "cardvalue.h"

CardValue Meld::getValue() {
    return this->value;
}

void Meld::addCard(Card* card) {
    this->cards.push_back(card);
}

std::vector<Card*>& Meld::getCards() {
    return this->cards;
}

Player& Meld::getPlayer() {
    return this->player;
}

bool Meld::valid() const {
    return allNonMagicCardsHaveTheSameValue() &&
           hasAtLeastTwoNonMagicCards() &&
           hasAtLeastThreeTotalCards() &&
           magicCardsNotExceedingNonMagicCards();
}

bool Meld::isMagicCard(CardValue cardValue) const {
    return cardValue == TWO || cardValue == JOKER;
}

int Meld::countMagicCards() const {
    int count = 0;
    for (auto& card : this->cards) {
        if (isMagicCard(card->value)) {
            count++;
        }
    }
    return count;
}

bool Meld::allNonMagicCardsHaveTheSameValue() const {
    CardValue baseValue = INVALID_CARD_VALUE;
    
    for (auto& card : this->cards) {
        if (!isMagicCard(card->value)) {
            if (baseValue == INVALID_CARD_VALUE) {
                baseValue = card->value;
            } else if (card->value != baseValue) {
                return false;
            }
        }
    }
    
    return true;
}

bool Meld::hasAtLeastTwoNonMagicCards() const {
    int nonMagicCardCount = 0;
    for (auto& card : this->cards) {
        if (!isMagicCard(card->value)) {
            nonMagicCardCount++;
        }
    }
    return nonMagicCardCount >= 2;
}

bool Meld::hasAtLeastThreeTotalCards() const {
    return this->cards.size() >= 3;
}

bool Meld::magicCardsNotExceedingNonMagicCards() const {
    int magicCardCount = countMagicCards();
    int nonMagicCardCount = static_cast<int>(this->cards.size()) - magicCardCount;
    return magicCardCount <= nonMagicCardCount;
}
