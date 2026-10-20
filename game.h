#ifndef GAME_H
#define GAME_H

#include "cardsdeck.h"
#include "player.h"
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
    int currentPlayerIndex;
    bool humanPlayerHasDrawnThisTurn;
    bool humanTeamHasInitialMeld;
    int humanTeamInitialMeldMinimumPoints;
    bool humanTeamHasMeldedInPreviousTurns;
    int roundFinishingPlayerNumber;
    bool roundFinishedConcealed;
    int humanPlayerCardPage;
    void generatePlayers();
    void generateDecks();
    void meldRedThrees();
    Meld& getRedThreesMeld(const Player& player);
    Meld& getTeamMeld(CardValue value, const Player& player);
    Meld* findTeamMeld(CardValue value, const Player& player);
    int calculateInitialMeldMinimumPoints(const Player& player) const;
    int getHumanTeamMeldsPoints() const;
    bool hasHumanTeamCanasta() const;
    bool wouldHumanPlayerFinishWithoutCanasta(int remainingCardsCount, int resultingMeldSize) const;
    void finishCurrentPlayerTurn();
    std::vector<Card*> getHumanPlayerChosenCards();
    static Position exposedDeckPosition(bool rotated = false);
    void discardChosenCard();
    void takeExposedDeck();
    void initializeTeamMeldCardPositions(const Player& player);
    void deleteCards();
    void saveRoundResult(const RoundResult& roundResult) const;
public:
    Game(std::vector<Player> players, std::vector<CardsDeck> decks, int currentPlayerIndex, bool humanPlayerHasDrawnThisTurn, int humanPlayerCardPage): players(players), decks(decks), currentPlayerIndex(currentPlayerIndex), humanPlayerHasDrawnThisTurn(humanPlayerHasDrawnThisTurn), humanTeamHasInitialMeld(false), humanTeamInitialMeldMinimumPoints(0), humanTeamHasMeldedInPreviousTurns(false), roundFinishingPlayerNumber(-1), roundFinishedConcealed(false), humanPlayerCardPage(humanPlayerCardPage) {}
    Game() : currentPlayerIndex(0), humanPlayerHasDrawnThisTurn(false), humanTeamHasInitialMeld(false), humanTeamInitialMeldMinimumPoints(0), humanTeamHasMeldedInPreviousTurns(false), roundFinishingPlayerNumber(-1), roundFinishedConcealed(false), humanPlayerCardPage(0) {}
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
    ~Game();
    void initialize();
    void initializeCardsPositions();
    void initializeMeldCardPositions();
    std::vector<CardsDeck>& getDecks();
    std::vector<Player>& getPlayers();
    std::vector<Meld>& getMelds();
    void addMeld(Meld& meld);
    bool hasMeldWithValue(CardValue value, const Player& player) const;
    bool canPlayerMeld(Player* player);
    bool canHumanPlayerMeld();
    CardsDeck& getHiddenDeck();
    void handleClickOnHiddenDeck();
    CardsDeck& getHumanPlayerDeck();
    CardsDeck& getExposedDeck();
    bool mouseOverHiddenDeck(int vw, int vh, int x, int y);
    bool mouseOverExposedDeck(int vw, int vh, int x, int y);
    bool hasExactlyOneCardSelected();
    bool canHumanPlayerDiscard();
    bool canHumanPlayerTakeExposedDeck();
    bool isExposedDeckFrozen();
    int getHumanTeamInitialMeldMinimumPoints() const { return humanTeamInitialMeldMinimumPoints; }
    bool hasHumanTeamInsufficientInitialMelds() const;
    void removeHumanTeamMelds();
    bool wouldMeldFinishWithoutCanasta();
    bool wouldAddingCardsToMeldFinishWithoutCanasta(int meldIndex);
    bool wouldClickOnExposedDeckFinishWithoutCanasta();
    bool wouldMeldingChosenCardsFinishWithInsufficientInitialMelds();
    void finishTurnIfHumanPlayerHasNoCards();
    void handleClickOnExposedDeck();
    bool mouseOverMeld(int vw, int vh, int x, int y, int& meldIndex) const;
    bool canAddCardsToMeld(int meldIndex) const;
    void addCardsToMeld(int meldIndex);
    Player* getCurrentPlayer() { return &players[currentPlayerIndex]; }
    int getCurrentPlayerIndex() { return currentPlayerIndex; }
    void setCurrentPlayer(int index) { currentPlayerIndex = index; }
    bool hasHumanPlayerDrawnThisTurn() const { return humanPlayerHasDrawnThisTurn; }
    std::vector<RoundResult> loadRoundResults() const;
    bool isRoundFinished() const { return roundFinishingPlayerNumber != -1; }
    TeamRoundScore calculateTeamRoundScore(const Player& player) const;
    void finishRound();

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
