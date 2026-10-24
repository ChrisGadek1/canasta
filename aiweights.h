#ifndef AIWEIGHTS_H
#define AIWEIGHTS_H

struct AiWeights
{
    // Difficulty
    int memoryTurns = -1;
    double scoreNoise = 0.0;
    bool canFreezeExposedDeck = true;

    // Strategy
    int defensiveOpponentHandSize = 3;
    int defensivePartnerHandSize = 2;
    int defensiveHiddenDeckSize = 10;
    int goingOutHandSize = 6;

    // Drawing
    int extendingMeldGain = 40;
    int makingPairGain = 30;
    int magicCardGain = 25;
    int singleCardGain = 5;
    int redThreeGain = 50;
    double discardedValuePairFactor = 0.3;

    // Taking the exposed deck
    int alwaysTakeExposedDeckSize = 7;
    int pileCardScore = 15;
    int usefulPileCardScore = 40;
    int magicPileCardScore = 25;
    int initialMeldScore = 60;
    int pileCanastaScore = 50;
    int deadPileCardPenalty = 10;
    int addedMagicCardPenalty = 30;
    double junkPileFactor = 0.5;

    // Melding
    int maxOpeningMagicCards = 2;
    int meldSizeScore = 10;
    int magicCardMeldPenalty = 20;
    int defensivePointsFactor = 2;

    // Discarding
    int magicCardKeepValue = 100;
    int freezingMagicCardKeepValue = -50;
    int freezeExposedDeckSize = 5;
    int freezeDanger = 50;
    int pairKeepValue = 30;
    int extraCardKeepValue = 15;
    int singleKeepValue = 10;
    int opponentMeldDanger = 200;
    int knownPairDanger = 150;
    int knownCardDanger = 60;
    int pairProbabilityDanger = 100;
    int discardedValueSafety = 30;
    double frozenDangerFactor = 0.5;
    double pileSizeDangerDivisor = 5.0;

    static AiWeights easy() {
        AiWeights weights;
        weights.memoryTurns = 12;
        weights.scoreNoise = 0.3;
        weights.canFreezeExposedDeck = false;
        return weights;
    }
    static AiWeights normal() {
        AiWeights weights;
        weights.memoryTurns = 24;
        weights.scoreNoise = 0.1;
        return weights;
    }
    static AiWeights hard() { return AiWeights(); }
};

#endif // AIWEIGHTS_H
