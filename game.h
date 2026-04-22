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
    int humanPlayerCardPage;
    void generatePlayers();
    void generateDecks();
public:
    Game(std::vector<Player> players, std::vector<CardsDeck> decks, int currentPlayerIndex, bool humanPlayerHasDrawnThisTurn, int humanPlayerCardPage): players(players), decks(decks), currentPlayerIndex(currentPlayerIndex), humanPlayerHasDrawnThisTurn(humanPlayerHasDrawnThisTurn), humanPlayerCardPage(humanPlayerCardPage) {}
    Game() : humanPlayerCardPage(0) {}
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

    // Pagination methods
    static constexpr int CARDS_PER_PAGE = 15;
    int getHumanPlayerCardPage() const { return humanPlayerCardPage; }
    void setHumanPlayerCardPage(int page) { humanPlayerCardPage = page; }
    void nextPage();
    void previousPage();
    int getTotalPages(const std::vector<Card*>& cards) const;
    std::vector<Card*> getVisibleCards(const std::vector<Card*>& allCards) const;
    bool shouldShowArrows() const;
    bool canGoPrevious() const;
    bool canGoNext(const std::vector<Card*>& cards) const;
};

#endif // GAME_H
