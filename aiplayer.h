#ifndef AIPLAYER_H
#define AIPLAYER_H

#include "aiprobability.h"
#include "game.h"
#include <map>
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

    static constexpr int DEFENSIVE_OPPONENT_HAND_SIZE = 3;
    static constexpr int DEFENSIVE_PARTNER_HAND_SIZE = 2;
    static constexpr int DEFENSIVE_HIDDEN_DECK_SIZE = 10;
    static constexpr int GOING_OUT_HAND_SIZE = 6;

    static constexpr int ALWAYS_TAKE_EXPOSED_DECK_SIZE = 7;
    static constexpr int PILE_CARD_SCORE = 15;
    static constexpr int USEFUL_PILE_CARD_SCORE = 40;
    static constexpr int MAGIC_PILE_CARD_SCORE = 25;
    static constexpr int INITIAL_MELD_SCORE = 60;
    static constexpr int PILE_CANASTA_SCORE = 50;
    static constexpr int DEAD_PILE_CARD_PENALTY = 10;
    static constexpr int ADDED_MAGIC_CARD_PENALTY = 30;
    static constexpr double JUNK_PILE_FACTOR = 0.5;

    static constexpr int MAX_OPENING_MAGIC_CARDS = 2;
    static constexpr int MELD_SIZE_SCORE = 10;
    static constexpr int MAGIC_CARD_MELD_PENALTY = 20;
    static constexpr int DEFENSIVE_POINTS_FACTOR = 2;
    static constexpr int MAGIC_CARDS_CANASTA_SCORE = 700;
    static constexpr int NATURAL_CANASTA_SCORE = 500;
    static constexpr int MIXED_CANASTA_SCORE = 300;

    static constexpr int MAGIC_CARD_KEEP_VALUE = 100;
    static constexpr int FREEZING_MAGIC_CARD_KEEP_VALUE = -50;
    static constexpr int FREEZE_EXPOSED_DECK_SIZE = 5;
    static constexpr int FREEZE_DANGER = 50;
    static constexpr int PAIR_KEEP_VALUE = 30;
    static constexpr int EXTRA_CARD_KEEP_VALUE = 15;
    static constexpr int SINGLE_KEEP_VALUE = 10;
    static constexpr int OPPONENT_MELD_DANGER = 200;
    static constexpr int KNOWN_PAIR_DANGER = 150;
    static constexpr int KNOWN_CARD_DANGER = 60;
    static constexpr int PAIR_PROBABILITY_DANGER = 100;
    static constexpr int DISCARDED_VALUE_SAFETY = 30;
    static constexpr double FROZEN_DANGER_FACTOR = 0.5;
    static constexpr double PILE_SIZE_DANGER_DIVISOR = 5.0;

    int playerNumber;
    AiTurnPhase phase;
    AiStrategy strategy;
    std::vector<std::vector<Card*>> plannedOpeningMelds;
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
    explicit AiPlayer(int playerNumber) : playerNumber(playerNumber), phase(AiTurnPhase::DONE), strategy(AiStrategy::OPENING) {}
    int getPlayerNumber() const { return playerNumber; }
    bool isPlayingTurn() const { return phase != AiTurnPhase::DONE; }
    void startTurn(Game& game);
    void step(Game& game);
};

#endif // AIPLAYER_H
