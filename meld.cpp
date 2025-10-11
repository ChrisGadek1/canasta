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
    return allCardsHaveTheSameValue();
}

bool Meld::allCardsHaveTheSameValue() const {
    for (int i = 1; i < this->cards.size(); i++) {
        if (this->cards[i]->value != this->cards[i - 1]->value && this->cards[i]->value != JOKER && this->cards[i - 1]->value != JOKER) {
            return false;
        }
    }
    return this->cards.size() > 2;
}
