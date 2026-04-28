#include "meld.h"
#include "cardvalue.h"

CardValue Meld::getValue() const {
    return this->value;
}

void Meld::addCard(Card* card) {
    this->cards.push_back(card);
    this->moveMagicCardsToFront();
}

const std::vector<Card*>& Meld::getCards() const {
    return this->cards;
}

const Player& Meld::getPlayer() const {
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

bool Meld::canAddCards(const std::vector<Card*>& cards) const {
    for (auto& card : cards) {
        if (!isMagicCard(card->value) && card->value != this->value) {
            return false;
        }
    }
    return true;
}

void Meld::moveMagicCardsToFront() {
    std::vector<Card*> magic;
    std::vector<Card*> nonMagic;
    for (auto& card : this->cards) {
        if (isMagicCard(card->value)) {
            magic.push_back(card);
        } else {
            nonMagic.push_back(card);
        }
    }
    magic.insert(magic.end(), nonMagic.begin(), nonMagic.end());
    this->cards = magic;
}
