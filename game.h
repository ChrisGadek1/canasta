#ifndef GAME_H
#define GAME_H

#include "cardmemory.h"
#include "cardsdeck.h"
#include "player.h"
#include <array>
#include <vector>
#include "meld.h"
#include "roundresult.h"
#include "teamroundscore.h"

class Game
{
    static constexpr float exposedBottomPos = 0.65f;
    static constexpr float exposedLeftPos = 0.65f;
    std::vector<Player> players;
    std::vector<CardsDeck> decks;
    std::vector<Meld> melds;
    CardMemory cardMemory;
    int currentPlayerIndex;
    int turnNumber;
    bool currentPlayerHasDrawnThisTurn;
    std::array<bool, 2> teamHasInitialMeld;
    std::array<int, 2> teamInitialMeldMinimumPoints;
    std::array<bool, 2> teamHasMeldedInPreviousTurns;
    int roundFinishingPlayerNumber;
    bool roundFinishedConcealed;
    bool roundFinishedByEmptyHiddenDeck;
    int humanPlayerCardPage;
    static int getTeamIndex(int playerNumber) { return playerNumber % 2; }
    std::vector<Player> getMeldOwners(int playerNumber) const;
    void generatePlayers();
    void generateDecks();
    void meldRedThrees();
    Meld& getRedThreesMeld(const Player& player);
    Meld& getTeamMeld(CardValue value, const Player& player);
    Meld* findTeamMeld(CardValue value, const Player& player);
    int calculateInitialMeldMinimumPoints(const Player& player) const;
    void finishCurrentPlayerTurn();
    void finishTurnIfCurrentPlayerHasNoCards();
    std::vector<Card*> getHumanPlayerChosenCards();
    static Position exposedDeckPosition(bool rotated = false);
    static Position hiddenDeckPosition();
    void initializeHumanPlayerPageCardsPositions(const std::vector<Card*>& cards);
    void initializeTeamMeldCardPositions(const Player& player);
    void deleteCards();
    void saveRoundResult(const RoundResult& roundResult) const;
    void clearRoundResults() const;
    void restartRound();
public:
    Game(std::vector<Player> players, std::vector<CardsDeck> decks, int currentPlayerIndex, bool currentPlayerHasDrawnThisTurn, int humanPlayerCardPage): players(players), decks(decks), currentPlayerIndex(currentPlayerIndex), turnNumber(0), currentPlayerHasDrawnThisTurn(currentPlayerHasDrawnThisTurn), teamHasInitialMeld(), teamInitialMeldMinimumPoints(), teamHasMeldedInPreviousTurns(), roundFinishingPlayerNumber(-1), roundFinishedConcealed(false), roundFinishedByEmptyHiddenDeck(false), humanPlayerCardPage(humanPlayerCardPage) {}
    Game() : currentPlayerIndex(0), turnNumber(0), currentPlayerHasDrawnThisTurn(false), teamHasInitialMeld(), teamInitialMeldMinimumPoints(), teamHasMeldedInPreviousTurns(), roundFinishingPlayerNumber(-1), roundFinishedConcealed(false), roundFinishedByEmptyHiddenDeck(false), humanPlayerCardPage(0) {}
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
    ~Game();
    void initialize();
    void initializeCardsPositions();
    void initializeMeldCardPositions();
    std::vector<CardsDeck>& getDecks();
    std::vector<Player>& getPlayers();
    std::vector<Meld>& getMelds();
    bool hasMeldWithValue(CardValue value, const Player& player) const;
    bool hasFullMeldWithValue(CardValue value, const Player& player) const;
    CardsDeck& getHiddenDeck();
    CardsDeck& getExposedDeck();
    CardsDeck& getPlayerDeck(int playerNumber);
    CardsDeck& getHumanPlayerDeck();
    bool isExposedDeckFrozen();

    // Actions of any player
    bool hasCurrentPlayerDrawnThisTurn() const { return currentPlayerHasDrawnThisTurn; }
    bool hasTeamInitialMeld(int playerNumber) const { return teamHasInitialMeld[getTeamIndex(playerNumber)]; }
    int getTeamInitialMeldMinimumPoints(int playerNumber) const { return teamInitialMeldMinimumPoints[getTeamIndex(playerNumber)]; }
    int getTeamMeldsPoints(int playerNumber) const;
    bool hasTeamCanasta(int playerNumber) const;
    bool hasTeamInsufficientInitialMelds(int playerNumber) const;
    bool wouldFinishWithoutCanasta(int playerNumber, int remainingCardsCount, int resultingMeldSize) const;
    bool wouldMeldingCardsFinishWithInsufficientInitialMelds(int playerNumber, const std::vector<Card*>& cards);
    bool wouldTakingExposedDeckFinishWithoutCanasta(int playerNumber, const std::vector<Card*>& cards);
    void drawFromHiddenDeck(int playerNumber);
    bool canTakeExposedDeck(int playerNumber, const std::vector<Card*>& cards);
    void takeExposedDeck(int playerNumber, const std::vector<Card*>& cards);
    bool canCreateMeld(int playerNumber, const std::vector<Card*>& cards);
    void createMeld(int playerNumber, const std::vector<Card*>& cards);
    bool canAddCardsToMeld(int playerNumber, int meldIndex, const std::vector<Card*>& cards) const;
    void addCardsToMeld(int playerNumber, int meldIndex, const std::vector<Card*>& cards);
    bool canDiscard(int playerNumber, Card* card);
    void discard(int playerNumber, Card* card);
    const CardMemory& getCardMemory() const { return cardMemory; }
    int getTurnNumber() const { return turnNumber; }
    std::array<int, CardMemory::CARD_VALUES_COUNT> getUnseenCardsCounts(int playerNumber, int memorySinceTurn = 0);

    // Actions of the human player, based on the chosen cards
    bool canHumanPlayerDraw() const { return currentPlayerIndex == 0 && !currentPlayerHasDrawnThisTurn; }
    void handleClickOnHiddenDeck();
    bool mouseOverHiddenDeck(int vw, int vh, int x, int y);
    bool mouseOverExposedDeck(int vw, int vh, int x, int y);
    bool hasExactlyOneCardSelected();
    bool canHumanPlayerMeld();
    void meldHumanPlayerChosenCards();
    bool canHumanPlayerDiscard();
    bool canHumanPlayerTakeExposedDeck();
    int getHumanTeamInitialMeldMinimumPoints() const { return getTeamInitialMeldMinimumPoints(0); }
    bool hasHumanTeamInsufficientInitialMelds() const { return hasTeamInsufficientInitialMelds(0); }
    void removeHumanTeamMelds();
    bool wouldMeldFinishWithoutCanasta();
    bool wouldAddingCardsToMeldFinishWithoutCanasta(int meldIndex);
    bool wouldClickOnExposedDeckFinishWithoutCanasta();
    bool wouldMeldingChosenCardsFinishWithInsufficientInitialMelds();
    void handleClickOnExposedDeck();
    bool mouseOverMeld(int vw, int vh, int x, int y, int& meldIndex) const;
    bool canAddCardsToMeld(int meldIndex);
    void addCardsToMeld(int meldIndex);

    Player* getCurrentPlayer() { return &players[currentPlayerIndex]; }
    int getCurrentPlayerIndex() { return currentPlayerIndex; }
    void setCurrentPlayer(int index) { currentPlayerIndex = index; }
    std::vector<RoundResult> loadRoundResults() const;
    bool isRoundFinished() const { return roundFinishingPlayerNumber != -1 || roundFinishedByEmptyHiddenDeck; }
    TeamRoundScore calculateTeamRoundScore(const Player& player) const;
    void finishRound();
    void startNewGame();

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
