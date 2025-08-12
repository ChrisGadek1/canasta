#include "cardsdeck.h"
#include "card.h"

void CardsDeck::addCard(Card card) {
     this->cards.push_back(card);
}

std::vector<Card>& CardsDeck::getCards() {
    return this->cards;
}
