#include "aiprobability.h"
#include <algorithm>
#include <cmath>

AiProbability::AiProbability(Game& game, int playerNumber) : game(game), playerNumber(playerNumber), unseenCounts(game.getUnseenCardsCounts(playerNumber)), totalUnseenCount(0), unseenRedThreesCount(RED_THREES_COUNT) {
    for (int unseenCount : this->unseenCounts) {
        this->totalUnseenCount += unseenCount;
    }
    std::vector<Card*> visibleCards = game.getExposedDeck().getCards();
    for (const auto& meld : game.getMelds()) {
        visibleCards.insert(visibleCards.end(), meld.getCards().begin(), meld.getCards().end());
    }
    this->unseenRedThreesCount -= static_cast<int>(std::count_if(visibleCards.begin(), visibleCards.end(), [](const Card* card) { return card->isRedThree(); }));
    this->unseenRedThreesCount = std::min(std::max(this->unseenRedThreesCount, 0), this->unseenCounts[CardValue::THREE]);
}

int AiProbability::countNaturalCardsInHand(CardValue value) const {
    std::vector<Card*>& handCards = this->game.getPlayerDeck(this->playerNumber).getCards();
    return static_cast<int>(std::count_if(handCards.begin(), handCards.end(), [value](const Card* card) { return !card->isMagic() && card->value == value; }));
}

double AiProbability::drawProbability(CardValue value) const {
    if (this->totalUnseenCount == 0) return 0.0;
    return static_cast<double>(this->unseenCounts[value]) / this->totalUnseenCount;
}

double AiProbability::redThreeDrawProbability() const {
    if (this->totalUnseenCount == 0) return 0.0;
    return static_cast<double>(this->unseenRedThreesCount) / this->totalUnseenCount;
}

double AiProbability::opponentHasPairProbability(int opponentNumber, CardValue value) const {
    const CardMemory& cardMemory = this->game.getCardMemory();
    int knownCount = cardMemory.getKnownInHand(opponentNumber, value);
    if (knownCount >= 2) return 1.0;

    int unknownCardsCount = static_cast<int>(this->game.getPlayerDeck(opponentNumber).getCards().size()) - cardMemory.getKnownInHandTotal(opponentNumber);
    double p = this->drawProbability(value);
    double noneProbability = std::pow(1.0 - p, unknownCardsCount);
    double probability = 1.0 - noneProbability;
    if (knownCount == 0) {
        double exactlyOneProbability = unknownCardsCount > 0 ? unknownCardsCount * p * std::pow(1.0 - p, unknownCardsCount - 1) : 0.0;
        probability -= exactlyOneProbability;
    }
    if (cardMemory.hasDiscarded(opponentNumber, value)) {
        probability *= DISCARDED_VALUE_PAIR_FACTOR;
    }
    return std::max(probability, 0.0);
}

int AiProbability::cardGain(CardValue value) const {
    if (value == CardValue::THREE) return 0;
    if (value == CardValue::TWO || value == CardValue::JOKER) return MAGIC_CARD_GAIN;
    if (this->game.hasMeldWithValue(value, this->game.getPlayers()[this->playerNumber])) return EXTENDING_MELD_GAIN;
    if (this->countNaturalCardsInHand(value) > 0) return MAKING_PAIR_GAIN;
    return SINGLE_CARD_GAIN;
}

double AiProbability::expectedDrawGain() const {
    double expectedGain = this->redThreeDrawProbability() * RED_THREE_GAIN;
    for (int value = 0; value < CardMemory::CARD_VALUES_COUNT; value++) {
        expectedGain += this->drawProbability(static_cast<CardValue>(value)) * this->cardGain(static_cast<CardValue>(value));
    }
    return expectedGain;
}

bool AiProbability::isDeadValue(CardValue value) const {
    return this->unseenCounts[value] == 0 && this->countNaturalCardsInHand(value) == 1;
}
