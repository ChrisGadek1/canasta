#ifndef CARDSDECK_H
#define CARDSDECK_H

#include "cardsdecktype.h"
#include "card.h"
#include "player.h"
#include <vector>

class CardsDeck {
    CardsDeckType type;
    std::vector<Card*> cards;
    Player player;
public:
    CardsDeck(CardsDeckType type, std::vector<Card*> cards, Player player): type(type), cards(cards), player(player) {}
    CardsDeck(CardsDeckType type, std::vector<Card*> cards): type(type), cards(cards) {}
    void addCard(Card* card);
    void sortCards();
    std::vector<Card*>& getCards();
    const std::vector<Card*>& getCards() const;
    Player& getPlayer();
    const Player& getPlayer() const;
    CardsDeckType& getType();
    CardsDeckType getType() const;
};

#endif // CARDSDECK_H
