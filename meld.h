#ifndef MELD_H
#define MELD_H
#include "cardvalue.h"
#include "player.h"
#include <vector>
#include "card.h"

class Meld
{
    CardValue value;
    std::vector<Card*> cards;
    Player player;
public:
    Meld(CardValue value, Player player) : value(value), player(player) {}
    CardValue getValue();
    void addCard(Card* card);
    std::vector<Card*>& getCards();
    Player& getPlayer();
};

#endif // MELD_H
