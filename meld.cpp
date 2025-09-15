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
