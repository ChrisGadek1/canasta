#ifndef CARD_H
#define CARD_H

#include "player.h"
#include "cardsuit.h"
#include "cardvalue.h"
#include <string>

class Card
{
    CardValue value;
    CardSuit suit;
    Player player;
public:
    Card(CardValue value, CardSuit suit, Player player): value(value), suit(suit), player(player) {}

    std::string texturePath();
};

#endif // CARD_H
