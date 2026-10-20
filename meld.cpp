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

const std::vector<Player>& Meld::getOwners() const {
    return this->owners;
}

bool Meld::isOwnedBy(const Player& player) const {
    for (const auto& owner : this->owners) {
        if (owner.getNumber() == player.getNumber()) {
            return true;
        }
    }
    return false;
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
                printf("cannot meld - all non magic cards don't have the same value \n");
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
    if (nonMagicCardCount < 2) {
        printf("cannot meld - not enough non magic cards \n");
    }
    return nonMagicCardCount >= 2;
}

bool Meld::hasAtLeastThreeTotalCards() const {
    if (this->cards.size() < 3) {
        printf("cannot meld - not enough cards \n");
    }
    return this->cards.size() >= 3;
}

bool Meld::magicCardsNotExceedingNonMagicCards() const {
    int magicCardCount = countMagicCards();
    int nonMagicCardCount = static_cast<int>(this->cards.size()) - magicCardCount;
    if (magicCardCount > nonMagicCardCount) {
        printf("cannot meld - non magic cards not exceeded magic cards number \n");
    }
    return magicCardCount <= nonMagicCardCount;
}

bool Meld::canAddCards(const std::vector<Card*>& cards) const {
    Meld tempMeld(this->value, this->owners);
    for (auto& card : this->cards) {
        tempMeld.addCard(card);
    }
    for (auto& card : cards) {
        tempMeld.addCard(card);
    }
    if (!tempMeld.valid()) {
        printf("cannot add cards to meld - resulting meld would be invalid\n");
        return false;
    }
    return true;
}

int Meld::getPoints() const {
    int points = 0;
    for (auto& card : this->cards) {
        points += card->getPoints();
    }
    return points;
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
