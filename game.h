#ifndef GAME_H
#define GAME_H

#include "cardsdeck.h"
#include "player.h"
#include <vector>
#include "meld.h"

class Game
{
    std::vector<Player> players;
    std::vector<CardsDeck> decks;
    std::vector<Meld> melds;
    void generatePlayers();
    void generateDecks();
public:
    Game(std::vector<Player> players, std::vector<CardsDeck> decks): players(players), decks(decks) {}
    Game() {}
    void initialize();
    void initializeCardsPositions();
    void initializeMeldCardPositions(Meld* meld);
    std::vector<CardsDeck>& getDecks();
    std::vector<Player>& getPlayers();
    std::vector<Meld>& getMelds();
    void addMeld(Meld& meld);
};

#endif // GAME_H
