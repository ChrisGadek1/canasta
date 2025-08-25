#include "cardsdeck.h"
#include "card.h"

void CardsDeck::addCard(Card card) {
     this->cards.push_back(card);
}

std::vector<Card>& CardsDeck::getCards() {
    return this->cards;
}

Player& CardsDeck::getPlayer() {
    return this->player;
}

CardsDeckType& CardsDeck::getType() {
    return this->type;
}
