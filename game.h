#ifndef GAME_H
#define GAME_H

#include "cardsdeck.h"
#include "player.h"
#include <vector>

class Game
{
    std::vector<Player> players;
    std::vector<CardsDeck> decks;
    void generatePlayers();
    void generateDecks();
public:
    Game(std::vector<Player> players, std::vector<CardsDeck> decks): players(players), decks(decks) {}
    Game() {}
    void initialize();
    std::vector<CardsDeck>& getDecks();
    std::vector<Player>& getPlayers();
};

#endif // GAME_H
