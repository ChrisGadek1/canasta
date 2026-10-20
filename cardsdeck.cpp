#include "cardsdeck.h"
#include "card.h"
#include <algorithm>

void CardsDeck::addCard(Card* card) {
     this->cards.push_back(card);
}

void CardsDeck::sortCards() {
    auto sortRank = [](const Card* card) { return card->value == JOKER ? -1 : static_cast<int>(card->value); };
    std::stable_sort(this->cards.begin(), this->cards.end(), [sortRank](const Card* a, const Card* b) {
        if (sortRank(a) != sortRank(b)) return sortRank(a) > sortRank(b);
        return a->suit < b->suit;
    });
}

std::vector<Card*>& CardsDeck::getCards() {
    return this->cards;
}

const std::vector<Card*>& CardsDeck::getCards() const {
    return this->cards;
}

Player& CardsDeck::getPlayer() {
    return this->player;
}

const Player& CardsDeck::getPlayer() const {
    return this->player;
}

CardsDeckType& CardsDeck::getType() {
    return this->type;
}

CardsDeckType CardsDeck::getType() const {
    return this->type;
}
