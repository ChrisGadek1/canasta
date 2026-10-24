#ifndef AIPROBABILITY_H
#define AIPROBABILITY_H

#include "game.h"
#include <array>

class AiProbability
{
    static constexpr int EXTENDING_MELD_GAIN = 40;
    static constexpr int MAKING_PAIR_GAIN = 30;
    static constexpr int MAGIC_CARD_GAIN = 25;
    static constexpr int SINGLE_CARD_GAIN = 5;
    static constexpr int RED_THREE_GAIN = 50;
    static constexpr int RED_THREES_COUNT = 4;
    static constexpr double DISCARDED_VALUE_PAIR_FACTOR = 0.3;
    Game& game;
    int playerNumber;
    std::array<int, CardMemory::CARD_VALUES_COUNT> unseenCounts;
    int totalUnseenCount;
    int unseenRedThreesCount;
    int countNaturalCardsInHand(CardValue value) const;
public:
    AiProbability(Game& game, int playerNumber);
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
