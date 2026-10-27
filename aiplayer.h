#ifndef AIPLAYER_H
#define AIPLAYER_H

#include "aiprobability.h"
#include "aiweights.h"
#include "game.h"
#include <chrono>
#include <map>
#include <random>
#include <vector>

enum class AiTurnPhase { DRAW, MELD, DISCARD, DONE };
enum class AiStrategy { OPENING, BUILDING, GOING_OUT, DEFENSIVE };

class AiPlayer
{
    struct MeldAction {
        int meldIndex;
        std::vector<Card*> cards;
        double score;
    };

    static constexpr int MAGIC_CARDS_CANASTA_SCORE = 700;
    static constexpr int NATURAL_CANASTA_SCORE = 500;
    static constexpr int MIXED_CANASTA_SCORE = 300;

    int playerNumber;
    AiWeights weights;
    std::default_random_engine randomEngine;
    AiTurnPhase phase;
    AiStrategy strategy;
    std::vector<std::vector<Card*>> plannedOpeningMelds;
    int getMemorySinceTurn(Game& game) const { return weights.memoryTurns < 0 ? 0 : game.getTurnNumber() - weights.memoryTurns; }
    double noisy(double score);
    const Player& getPlayer(Game& game) const { return game.getPlayers()[playerNumber]; }
    std::vector<Card*>& getHandCards(Game& game) const { return game.getPlayerDeck(playerNumber).getCards(); }
    bool canDumpCards() const { return strategy == AiStrategy::DEFENSIVE || strategy == AiStrategy::GOING_OUT; }
    void chooseStrategy(Game& game);
    void playDrawPhase(Game& game);
    bool chooseExposedDeckCards(Game& game, const AiProbability& probability, std::vector<Card*>& chosenCards, double& score);
    double scoreTakingExposedDeck(Game& game, const AiProbability& probability, const std::vector<Card*>& cards);
    bool playMeldStep(Game& game);
    void planOpeningMelds(Game& game);
    std::vector<MeldAction> findMeldActions(Game& game);
    double scoreMeldAction(Game& game, int meldIndex, const std::vector<Card*>& cards) const;
    void playDiscardPhase(Game& game);
    bool shouldFreezeExposedDeck(Game& game, const AiProbability& probability);
    double getKeepValue(Game& game, const AiProbability& probability, const Card* card, bool freezeExposedDeck);
    double getDanger(Game& game, const AiProbability& probability, const Card* card);
    std::map<CardValue, std::vector<Card*>> groupNaturalCardsByValue(Game& game);
    std::vector<Card*> getMagicCards(Game& game);
    int getTeamMeldSize(Game& game, CardValue value);
    bool isInitialMeldFulfilled(Game& game) const;
    int getMinimumRemainingCardsCount(Game& game) const;
    static int countPoints(const std::vector<Card*>& cards);
    static int countMagicCards(const std::vector<Card*>& cards);
    static int getCanastaScore(const std::vector<Card*>& cards);
    static const char* strategyName(AiStrategy strategy);
public:
    AiPlayer(int playerNumber, const AiWeights& weights) : playerNumber(playerNumber), weights(weights), randomEngine(static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count()) + playerNumber), phase(AiTurnPhase::DONE), strategy(AiStrategy::OPENING) {}
    int getPlayerNumber() const { return playerNumber; }
    void setWeights(const AiWeights& weights) { this->weights = weights; }
    bool isPlayingTurn() const { return phase != AiTurnPhase::DONE; }
    void startTurn(Game& game);
    void step(Game& game);
};

#endif // AIPLAYER_H
