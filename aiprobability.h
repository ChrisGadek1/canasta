#ifndef AIPROBABILITY_H
#define AIPROBABILITY_H

#include "aiweights.h"
#include "game.h"
#include <array>

class AiProbability
{
    static constexpr int RED_THREES_COUNT = 4;
    Game& game;
    int playerNumber;
    const AiWeights& weights;
    int memorySinceTurn;
    std::array<int, CardMemory::CARD_VALUES_COUNT> unseenCounts;
    int totalUnseenCount;
    int unseenRedThreesCount;
    int countNaturalCardsInHand(CardValue value) const;
public:
    AiProbability(Game& game, int playerNumber, const AiWeights& weights, int memorySinceTurn);
    int getUnseenCount(CardValue value) const { return unseenCounts[value]; }
    int getTotalUnseenCount() const { return totalUnseenCount; }
    double drawProbability(CardValue value) const;
    double redThreeDrawProbability() const;
    double opponentHasPairProbability(int opponentNumber, CardValue value) const;
    int cardGain(CardValue value) const;
    double expectedDrawGain() const;
    bool isDeadValue(CardValue value) const;
};

#endif // AIPROBABILITY_H
