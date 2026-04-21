#ifndef GAME_H
#define GAME_H

#include "cardsdeck.h"
#include "player.h"
#include <vector>
#include "meld.h"

class Game
{
    static constexpr float exposedBottomPos = 0.65f;
    static constexpr float exposedLeftPos = 0.65f;
    std::vector<Player> players;
    std::vector<CardsDeck> decks;
    std::vector<Meld> melds;
    int currentPlayerIndex;
    bool humanPlayerHasDrawnThisTurn;
    void generatePlayers();
    void generateDecks();
public:
    Game(std::vector<Player> players, std::vector<CardsDeck> decks, int currentPlayerIndex, bool humanPlayerHasDrawnThisTurn): players(players), decks(decks), currentPlayerIndex(currentPlayerIndex), humanPlayerHasDrawnThisTurn(humanPlayerHasDrawnThisTurn) {}
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
    CardsDeck& getExposedDeck();
    bool mouseOverHiddenDeck(int vw, int vh, int x, int y);
    bool mouseOverExposedDeck(int vw, int vh, int x, int y);
    bool hasExactlyOneCardSelected();
    void handleClickOnExposedDeck();
    Player* getCurrentPlayer() { return &players[currentPlayerIndex]; }
    int getCurrentPlayerIndex() { return currentPlayerIndex; }
    void setCurrentPlayer(int index) { currentPlayerIndex = index; }
    void resetHumanPlayerDrawFlag();
    bool hasHumanPlayerDrawnThisTurn() const { return humanPlayerHasDrawnThisTurn; }
};

#endif // GAME_H
