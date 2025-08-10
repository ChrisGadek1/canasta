#ifndef GAME_H
#define GAME_H

#include "card.h"
#include "player.h"
#include <vector>

class Game
{
    std::vector<Player> players;
    std::vector<Card> cards;
public:
    Game(std::vector<Player> players, std::vector<Card> cards): players(players), cards(cards) {}
};

#endif // GAME_H
