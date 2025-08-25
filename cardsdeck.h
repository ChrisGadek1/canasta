#ifndef CARDSDECK_H
#define CARDSDECK_H

#include "cardsdecktype.h"
#include "card.h"
#include "player.h"
#include <vector>

class CardsDeck {
    CardsDeckType type;
    std::vector<Card> cards;
    Player player;
public:
    CardsDeck(CardsDeckType type, std::vector<Card> cards, Player player): type(type), cards(cards), player(player) {}
    CardsDeck(CardsDeckType type, std::vector<Card> cards): type(type), cards(cards) {}
    void addCard(Card card);
    std::vector<Card>& getCards();
    Player& getPlayer();
    CardsDeckType& getType();
};

#endif // CARDSDECK_H
