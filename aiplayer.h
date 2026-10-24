#ifndef AIPLAYER_H
#define AIPLAYER_H

#include "game.h"
#include <map>
#include <vector>

enum class AiTurnPhase { DRAW, MELD, DISCARD, DONE };
enum class AiStrategy { OPENING, BUILDING, GOING_OUT, DEFENSIVE };

class AiPlayer
{
    static constexpr int DEFENSIVE_OPPONENT_HAND_SIZE = 3;
    static constexpr int DEFENSIVE_HIDDEN_DECK_SIZE = 10;
    static constexpr int GOING_OUT_HAND_SIZE = 6;
    static constexpr int WILD_CARD_DISCARD_COST = 1000;
    static constexpr int PAIR_DISCARD_COST = 100;
    int playerNumber;
    AiTurnPhase phase;
    AiStrategy strategy;
    void chooseStrategy(Game& game);
    void playDrawPhase(Game& game);
    bool playMeldStep(Game& game);
    void playDiscardPhase(Game& game);
    std::map<CardValue, std::vector<Card*>> groupNaturalCardsByValue(Game& game);
    std::vector<std::vector<Card*>> chooseNewMelds(Game& game);
    int getMinimumRemainingCardsCount(Game& game) const;
    int getDiscardCost(Game& game, Card* card);
    static const char* strategyName(AiStrategy strategy);
public:
    explicit AiPlayer(int playerNumber) : playerNumber(playerNumber), phase(AiTurnPhase::DONE), strategy(AiStrategy::OPENING) {}
    int getPlayerNumber() const { return playerNumber; }
    bool isPlayingTurn() const { return phase != AiTurnPhase::DONE; }
    void startTurn(Game& game);
    void step(Game& game);
};

#endif // AIPLAYER_H
