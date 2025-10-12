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
    void initializeMeldCardPositions();
    std::vector<CardsDeck>& getDecks();
    std::vector<Player>& getPlayers();
    std::vector<Meld>& getMelds();
    void addMeld(Meld& meld);
    bool canPlayerMeld(Player* player);
    bool canHumanPlayerMeld();
    CardsDeck& getHiddenDeck();
    void handleClickOnHiddenDeck();
    CardsDeck& getHumanPlayerDeck();
    bool mouseOverHiddenDeck(int vw, int vh, int x, int y);
};

#endif // GAME_H
