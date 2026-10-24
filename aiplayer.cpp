#include "aiplayer.h"
#include <algorithm>
#include <cstdio>

void AiPlayer::startTurn(Game& game) {
    this->phase = AiTurnPhase::DRAW;
    this->chooseStrategy(game);
    printf("P%d strategy: %s\n", this->playerNumber, strategyName(this->strategy));
}

void AiPlayer::step(Game& game) {
    if (this->phase == AiTurnPhase::DRAW) {
        this->playDrawPhase(game);
        this->phase = AiTurnPhase::MELD;
    } else if (this->phase == AiTurnPhase::MELD && !this->playMeldStep(game)) {
        this->phase = AiTurnPhase::DISCARD;
    }
    if (this->phase == AiTurnPhase::DISCARD) {
        this->playDiscardPhase(game);
        this->phase = AiTurnPhase::DONE;
    }
    if (game.isRoundFinished() || game.getCurrentPlayerIndex() != this->playerNumber) {
        this->phase = AiTurnPhase::DONE;
    }
}

void AiPlayer::chooseStrategy(Game& game) {
    bool opponentAlmostOut = false;
    for (int opponentNumber : {(this->playerNumber + 1) % 4, (this->playerNumber + 3) % 4}) {
        if (static_cast<int>(game.getPlayerDeck(opponentNumber).getCards().size()) <= this->weights.defensiveOpponentHandSize) {
            opponentAlmostOut = true;
        }
    }
    int partnerHandSize = static_cast<int>(game.getPlayerDeck((this->playerNumber + 2) % 4).getCards().size());
    bool partnerAlmostOut = partnerHandSize <= this->weights.defensivePartnerHandSize && game.hasTeamCanasta(this->playerNumber);
    int handSize = static_cast<int>(this->getHandCards(game).size());
    if (opponentAlmostOut || partnerAlmostOut || static_cast<int>(game.getHiddenDeck().getCards().size()) < this->weights.defensiveHiddenDeckSize) {
        this->strategy = AiStrategy::DEFENSIVE;
    } else if (!game.hasTeamInitialMeld(this->playerNumber)) {
        this->strategy = AiStrategy::OPENING;
    } else if (game.hasTeamCanasta(this->playerNumber) && handSize <= this->weights.goingOutHandSize) {
        this->strategy = AiStrategy::GOING_OUT;
    } else {
        this->strategy = AiStrategy::BUILDING;
    }
}

void AiPlayer::playDrawPhase(Game& game) {
    AiProbability probability(game, this->playerNumber, this->weights, this->getMemorySinceTurn(game));
    double expectedDrawGain = probability.expectedDrawGain();
    std::vector<Card*> exposedDeckCards;
    double takeScore = 0.0;
    bool canTake = this->chooseExposedDeckCards(game, probability, exposedDeckCards, takeScore);
    int exposedDeckSize = static_cast<int>(game.getExposedDeck().getCards().size());
    if (canTake && (exposedDeckSize >= this->weights.alwaysTakeExposedDeckSize || takeScore > expectedDrawGain)) {
        game.takeExposedDeck(this->playerNumber, exposedDeckCards);
        printf("P%d takes the exposed deck of %d cards with %d own cards (take=%.1f ev=%.1f)\n", this->playerNumber, exposedDeckSize, static_cast<int>(exposedDeckCards.size()), takeScore, expectedDrawGain);
    } else {
        game.drawFromHiddenDeck(this->playerNumber);
        if (game.hasCurrentPlayerDrawnThisTurn()) {
            printf("P%d draws from the hidden deck (can take=%d take=%.1f ev=%.1f)\n", this->playerNumber, canTake, takeScore, expectedDrawGain);
        } else if (!game.isRoundFinished()) {
            printf("P%d could not draw a card\n", this->playerNumber);
        }
    }

    this->plannedOpeningMelds.clear();
    if (game.isRoundFinished() || game.getCurrentPlayerIndex() != this->playerNumber) return;
    AiStrategy previousStrategy = this->strategy;
    this->chooseStrategy(game);
    if (this->strategy != previousStrategy) {
        printf("P%d strategy after drawing: %s\n", this->playerNumber, strategyName(this->strategy));
    }
    if (!this->isInitialMeldFulfilled(game)) {
        this->planOpeningMelds(game);
    }
}

bool AiPlayer::chooseExposedDeckCards(Game& game, const AiProbability& probability, std::vector<Card*>& chosenCards, double& score) {
    std::vector<Card*>& exposedCards = game.getExposedDeck().getCards();
    if (exposedCards.empty()) return false;
    Card* topCard = exposedCards.back();

    std::vector<std::vector<Card*>> candidates{std::vector<Card*>()};
    std::map<CardValue, std::vector<Card*>> groups = this->groupNaturalCardsByValue(game);
    auto group = groups.find(topCard->value);
    if (group != groups.end() && group->second.size() >= 2) {
        std::vector<Card*> cards = group->second;
        candidates.push_back(cards);
        if (!game.hasTeamInitialMeld(this->playerNumber)) {
            for (auto* magicCard : this->getMagicCards(game)) {
                if (countMagicCards(cards) >= this->weights.maxOpeningMagicCards) break;
                cards.push_back(magicCard);
                candidates.push_back(cards);
            }
        }
    }

    int pileCardsToHandCount = static_cast<int>(std::count_if(exposedCards.begin(), exposedCards.end() - 1, [](const Card* card) { return !card->isRedThree(); }));
    int handSize = static_cast<int>(this->getHandCards(game).size());
    bool found = false;
    for (const auto& cards : candidates) {
        int remainingCardsCount = handSize - static_cast<int>(cards.size()) + pileCardsToHandCount;
        if (remainingCardsCount < 2 && !game.hasTeamCanasta(this->playerNumber)) continue;
        if (!game.canTakeExposedDeck(this->playerNumber, cards) || game.wouldTakingExposedDeckFinishWithoutCanasta(this->playerNumber, cards)) continue;
        double candidateScore = this->noisy(this->scoreTakingExposedDeck(game, probability, cards));
        if (!found || candidateScore > score) {
            found = true;
            score = candidateScore;
            chosenCards = cards;
        }
    }
    return found;
}

double AiPlayer::scoreTakingExposedDeck(Game& game, const AiProbability& probability, const std::vector<Card*>& cards) {
    std::vector<Card*>& exposedCards = game.getExposedDeck().getCards();
    Card* topCard = exposedCards.back();
    std::map<CardValue, int> naturalCounts;
    for (auto* card : this->getHandCards(game)) {
        if (!card->isMagic() && std::find(cards.begin(), cards.end(), card) == cards.end()) naturalCounts[card->value]++;
    }
    for (auto* card : exposedCards) {
        if (!card->isMagic() && card != topCard) naturalCounts[card->value]++;
    }

    const Player& player = this->getPlayer(game);
    double score = 0.0;
    int pileCardsCount = 0;
    int junkCardsCount = 0;
    for (auto* card : exposedCards) {
        if (card->isRedThree()) {
            score += this->weights.usefulPileCardScore;
            continue;
        }
        pileCardsCount++;
        score += this->weights.pileCardScore;
        if (card->isMagic()) {
            score += this->weights.magicPileCardScore;
        } else if (card == topCard || (!card->isBlackThree() && (game.hasMeldWithValue(card->value, player) || naturalCounts[card->value] >= 2))) {
            score += this->weights.usefulPileCardScore;
        } else {
            junkCardsCount++;
            if (!card->isBlackThree() && probability.getUnseenCount(card->value) == 0) score -= this->weights.deadPileCardPenalty;
        }
    }
    if (!game.hasTeamInitialMeld(this->playerNumber)) score += this->weights.initialMeldScore;
    if (std::max(this->getTeamMeldSize(game, topCard->value), 0) + static_cast<int>(cards.size()) + 1 >= Meld::CANASTA_SIZE) score += this->weights.pileCanastaScore;
    score -= this->weights.addedMagicCardPenalty * countMagicCards(cards);
    if (this->strategy == AiStrategy::GOING_OUT && junkCardsCount * 2 > pileCardsCount) score *= this->weights.junkPileFactor;
    return score;
}

bool AiPlayer::playMeldStep(Game& game) {
    std::vector<Card*>& handCards = this->getHandCards(game);
    int handSizeBefore = static_cast<int>(handCards.size());
    if (!this->plannedOpeningMelds.empty()) {
        std::vector<Card*> cards = this->plannedOpeningMelds.front();
        this->plannedOpeningMelds.erase(this->plannedOpeningMelds.begin());
        game.createMeld(this->playerNumber, cards);
        if (static_cast<int>(handCards.size()) < handSizeBefore) {
            printf("P%d melds %d cards of value %d (opening)\n", this->playerNumber, static_cast<int>(cards.size()), Meld::baseValue(cards));
            return true;
        }
        printf("P%d could not create a planned opening meld\n", this->playerNumber);
        this->plannedOpeningMelds.clear();
        return false;
    }
    if (!this->isInitialMeldFulfilled(game)) return false;

    std::vector<MeldAction> actions = this->findMeldActions(game);
    std::stable_sort(actions.begin(), actions.end(), [](const MeldAction& a, const MeldAction& b) { return a.score > b.score; });
    for (const auto& action : actions) {
        if (action.score <= 0) break;
        if (action.meldIndex < 0) {
            game.createMeld(this->playerNumber, action.cards);
        } else {
            game.addCardsToMeld(this->playerNumber, action.meldIndex, action.cards);
        }
        if (game.getCurrentPlayerIndex() != this->playerNumber || static_cast<int>(handCards.size()) < handSizeBefore) {
            printf("P%d %s %d cards of value %d (score %.1f)\n", this->playerNumber, action.meldIndex < 0 ? "melds" : "adds", static_cast<int>(action.cards.size()), Meld::baseValue(action.cards), action.score);
            return true;
        }
    }
    return false;
}

void AiPlayer::planOpeningMelds(Game& game) {
    const Player& player = this->getPlayer(game);
    int capacity = static_cast<int>(this->getHandCards(game).size()) - this->getMinimumRemainingCardsCount(game);
    std::vector<std::vector<Card*>> triples;
    std::vector<std::vector<Card*>> pairs;
    for (const auto& group : this->groupNaturalCardsByValue(game)) {
        if (game.hasMeldWithValue(group.first, player)) continue;
        if (group.second.size() >= 3) triples.push_back(group.second);
        else if (group.second.size() == 2) pairs.push_back(group.second);
    }
    auto byPointsDescending = [](const std::vector<Card*>& a, const std::vector<Card*>& b) { return countPoints(a) > countPoints(b); };
    std::stable_sort(triples.begin(), triples.end(), byPointsDescending);
    std::stable_sort(pairs.begin(), pairs.end(), byPointsDescending);

    std::vector<Card*> magicCards = this->getMagicCards(game);
    int usedMagicCardsCount = 0;
    int points = game.getTeamMeldsPoints(this->playerNumber);
    int minimumPoints = game.getTeamInitialMeldMinimumPoints(this->playerNumber);
    std::vector<std::vector<Card*>> melds;
    for (const auto& cards : triples) {
        if (points >= minimumPoints) break;
        if (static_cast<int>(cards.size()) > capacity) continue;
        melds.push_back(cards);
        capacity -= static_cast<int>(cards.size());
        points += countPoints(cards);
    }
    for (const auto& cards : pairs) {
        if (points >= minimumPoints || usedMagicCardsCount >= this->weights.maxOpeningMagicCards || usedMagicCardsCount >= static_cast<int>(magicCards.size())) break;
        if (capacity < 3) continue;
        std::vector<Card*> meldCards = cards;
        meldCards.push_back(magicCards[usedMagicCardsCount++]);
        melds.push_back(meldCards);
        capacity -= 3;
        points += countPoints(meldCards);
    }
    for (auto& meldCards : melds) {
        while (points < minimumPoints && usedMagicCardsCount < this->weights.maxOpeningMagicCards && usedMagicCardsCount < static_cast<int>(magicCards.size()) && capacity > 0
               && 2 * countMagicCards(meldCards) < static_cast<int>(meldCards.size())) {
            Card* magicCard = magicCards[usedMagicCardsCount++];
            meldCards.push_back(magicCard);
            capacity--;
            points += magicCard->getPoints();
        }
    }
    if (points < minimumPoints) {
        printf("P%d cannot open yet (%d of %d points)\n", this->playerNumber, points, minimumPoints);
        return;
    }
    this->plannedOpeningMelds = melds;
    printf("P%d plans an opening of %d melds (%d of %d points)\n", this->playerNumber, static_cast<int>(melds.size()), points, minimumPoints);
}

std::vector<AiPlayer::MeldAction> AiPlayer::findMeldActions(Game& game) {
    std::vector<MeldAction> actions;
    int capacity = static_cast<int>(this->getHandCards(game).size()) - this->getMinimumRemainingCardsCount(game);
    if (capacity <= 0) return actions;

    const Player& player = this->getPlayer(game);
    std::map<CardValue, std::vector<Card*>> groups = this->groupNaturalCardsByValue(game);
    std::vector<Card*> magicCards = this->getMagicCards(game);
    std::vector<Meld>& melds = game.getMelds();
    for (int meldIndex = 0; meldIndex < static_cast<int>(melds.size()); meldIndex++) {
        const Meld& meld = melds[meldIndex];
        if (meld.isRedThreesMeld() || !meld.isOwnedBy(player)) continue;
        int meldSize = static_cast<int>(meld.getCards().size());
        int meldMagicCardsCount = countMagicCards(meld.getCards());
        bool magicCardsMeld = meldMagicCardsCount == meldSize;

        auto group = groups.find(meld.getValue());
        if (!magicCardsMeld && group != groups.end()) {
            std::vector<Card*> cards = group->second;
            if (static_cast<int>(cards.size()) > capacity) cards.resize(capacity);
            actions.push_back(MeldAction{meldIndex, cards, this->noisy(this->scoreMeldAction(game, meldIndex, cards))});
        }

        int magicSlots = magicCardsMeld ? static_cast<int>(magicCards.size()) : meldSize - 2 * meldMagicCardsCount;
        int magicCardsCount = std::min(std::min(magicSlots, static_cast<int>(magicCards.size())), capacity);
        int missingForCanasta = Meld::CANASTA_SIZE - meldSize;
        if (!this->canDumpCards() && missingForCanasta > 0 && missingForCanasta <= magicCardsCount) {
            magicCardsCount = missingForCanasta;
        } else if (!this->canDumpCards()) {
            magicCardsCount = 0;
        }
        if (magicCardsCount > 0) {
            std::vector<Card*> cards(magicCards.begin(), magicCards.begin() + magicCardsCount);
            actions.push_back(MeldAction{meldIndex, cards, this->noisy(this->scoreMeldAction(game, meldIndex, cards))});
        }
    }

    for (const auto& group : groups) {
        if (game.hasMeldWithValue(group.first, player)) continue;
        std::vector<Card*> cards = group.second;
        if (cards.size() >= 3 && capacity >= 3) {
            if (static_cast<int>(cards.size()) > capacity) cards.resize(capacity);
        } else if (cards.size() == 2 && this->canDumpCards() && !magicCards.empty() && capacity >= 3) {
            cards.push_back(magicCards.front());
        } else {
            continue;
        }
        actions.push_back(MeldAction{-1, cards, this->noisy(this->scoreMeldAction(game, -1, cards))});
    }

    if (static_cast<int>(magicCards.size()) >= Meld::CANASTA_SIZE && static_cast<int>(magicCards.size()) <= capacity && !game.hasMeldWithValue(CardValue::JOKER, player)) {
        actions.push_back(MeldAction{-1, magicCards, this->noisy(this->scoreMeldAction(game, -1, magicCards))});
    }
    return actions;
}

double AiPlayer::scoreMeldAction(Game& game, int meldIndex, const std::vector<Card*>& cards) const {
    std::vector<Card*> meldCards = cards;
    int meldSizeBefore = 0;
    if (meldIndex >= 0) {
        const std::vector<Card*>& existingCards = game.getMelds()[meldIndex].getCards();
        meldCards.insert(meldCards.end(), existingCards.begin(), existingCards.end());
        meldSizeBefore = static_cast<int>(existingCards.size());
    }
    int pointsFactor = this->strategy == AiStrategy::DEFENSIVE ? this->weights.defensivePointsFactor : 1;
    double score = countPoints(cards) * pointsFactor + this->weights.meldSizeScore * static_cast<int>(meldCards.size());
    if (meldSizeBefore < Meld::CANASTA_SIZE && static_cast<int>(meldCards.size()) >= Meld::CANASTA_SIZE) {
        score += getCanastaScore(meldCards);
    }
    if (this->strategy == AiStrategy::BUILDING) {
        score -= this->weights.magicCardMeldPenalty * countMagicCards(cards);
    }
    return score;
}

void AiPlayer::playDiscardPhase(Game& game) {
    AiProbability probability(game, this->playerNumber, this->weights, this->getMemorySinceTurn(game));
    bool freezeExposedDeck = this->shouldFreezeExposedDeck(game, probability);
    std::vector<std::pair<double, Card*>> cardsByCost;
    for (auto* card : this->getHandCards(game)) {
        double cost = this->noisy(this->getKeepValue(game, probability, card, freezeExposedDeck) + this->getDanger(game, probability, card));
        cardsByCost.push_back(std::make_pair(cost, card));
    }
    std::stable_sort(cardsByCost.begin(), cardsByCost.end(), [](const std::pair<double, Card*>& a, const std::pair<double, Card*>& b) { return a.first < b.first; });
    for (const auto& costAndCard : cardsByCost) {
        Card* card = costAndCard.second;
        if (!game.canDiscard(this->playerNumber, card)) continue;
        game.discard(this->playerNumber, card);
        if (game.isRoundFinished() || game.getCurrentPlayerIndex() != this->playerNumber) {
            printf("P%d discards card of value %d (cost %.1f%s)\n", this->playerNumber, card->value, costAndCard.first, freezeExposedDeck && card->isMagic() ? ", freezing" : "");
            return;
        }
    }
    printf("P%d could not discard any card\n", this->playerNumber);
}

bool AiPlayer::shouldFreezeExposedDeck(Game& game, const AiProbability& probability) {
    if (!this->weights.canFreezeExposedDeck) return false;
    if (static_cast<int>(game.getExposedDeck().getCards().size()) < this->weights.freezeExposedDeckSize || game.isExposedDeckFrozen()) return false;
    bool hasNaturalCards = false;
    for (auto* card : this->getHandCards(game)) {
        if (card->isBlackThree()) return false;
        if (card->isMagic()) continue;
        hasNaturalCards = true;
        if (this->getDanger(game, probability, card) < this->weights.freezeDanger) return false;
    }
    return hasNaturalCards;
}

double AiPlayer::getKeepValue(Game& game, const AiProbability& probability, const Card* card, bool freezeExposedDeck) {
    if (card->isBlackThree()) return 0.0;
    double keepValue;
    if (card->isMagic() && freezeExposedDeck) {
        keepValue = this->weights.freezingMagicCardKeepValue;
    } else if (card->isMagic()) {
        keepValue = this->weights.magicCardKeepValue;
    } else {
        std::vector<Card*>& handCards = this->getHandCards(game);
        int sameValueCount = static_cast<int>(std::count_if(handCards.begin(), handCards.end(), [card](const Card* handCard) { return !handCard->isMagic() && handCard->value == card->value; }));
        if (sameValueCount >= 2) {
            keepValue = this->weights.pairKeepValue + this->weights.extraCardKeepValue * (sameValueCount - 2);
        } else {
            keepValue = probability.isDeadValue(card->value) ? 0.0 : this->weights.singleKeepValue;
        }
    }
    if (this->strategy == AiStrategy::DEFENSIVE) {
        keepValue -= card->getPoints() / 2.0;
    }
    return keepValue;
}

double AiPlayer::getDanger(Game& game, const AiProbability& probability, const Card* card) {
    if (card->isBlackThree() || card->isMagic()) return 0.0;
    int nextPlayerNumber = (this->playerNumber + 1) % 4;
    const CardMemory& cardMemory = game.getCardMemory();
    bool exposedDeckFrozen = game.isExposedDeckFrozen();
    int knownCount = cardMemory.getKnownInHand(nextPlayerNumber, card->value, this->getMemorySinceTurn(game));
    double danger;
    if (!exposedDeckFrozen && game.hasMeldWithValue(card->value, game.getPlayers()[nextPlayerNumber])) {
        danger = this->weights.opponentMeldDanger;
    } else if (knownCount >= 2) {
        danger = this->weights.knownPairDanger;
    } else if (knownCount == 1) {
        danger = this->weights.knownCardDanger;
    } else {
        danger = probability.opponentHasPairProbability(nextPlayerNumber, card->value) * this->weights.pairProbabilityDanger;
    }
    if (cardMemory.hasDiscarded(nextPlayerNumber, card->value, this->getMemorySinceTurn(game))) danger -= this->weights.discardedValueSafety;
    if (exposedDeckFrozen) danger *= this->weights.frozenDangerFactor;
    int exposedDeckSize = static_cast<int>(game.getExposedDeck().getCards().size());
    return std::max(danger, 0.0) * (1.0 + exposedDeckSize / this->weights.pileSizeDangerDivisor);
}

std::map<CardValue, std::vector<Card*>> AiPlayer::groupNaturalCardsByValue(Game& game) {
    std::map<CardValue, std::vector<Card*>> groups;
    for (auto* card : this->getHandCards(game)) {
        if (card->isMagic() || card->value == CardValue::THREE) continue;
        groups[card->value].push_back(card);
    }
    return groups;
}

std::vector<Card*> AiPlayer::getMagicCards(Game& game) {
    std::vector<Card*> magicCards;
    for (auto* card : this->getHandCards(game)) {
        if (card->isMagic()) magicCards.push_back(card);
    }
    std::stable_sort(magicCards.begin(), magicCards.end(), [](const Card* a, const Card* b) { return a->getPoints() < b->getPoints(); });
    return magicCards;
}

int AiPlayer::getTeamMeldSize(Game& game, CardValue value) {
    const Player& player = this->getPlayer(game);
    for (const auto& meld : game.getMelds()) {
        if (!meld.isRedThreesMeld() && meld.getValue() == value && meld.isOwnedBy(player)) return static_cast<int>(meld.getCards().size());
    }
    return -1;
}

bool AiPlayer::isInitialMeldFulfilled(Game& game) const {
    return game.hasTeamInitialMeld(this->playerNumber) || game.getTeamMeldsPoints(this->playerNumber) >= game.getTeamInitialMeldMinimumPoints(this->playerNumber);
}

int AiPlayer::getMinimumRemainingCardsCount(Game& game) const {
    return game.hasTeamCanasta(this->playerNumber) && this->canDumpCards() ? 1 : 2;
}

double AiPlayer::noisy(double score) {
    if (this->weights.scoreNoise <= 0.0) return score;
    std::uniform_real_distribution<double> noise(-this->weights.scoreNoise, this->weights.scoreNoise);
    return score * (1.0 + noise(this->randomEngine));
}

int AiPlayer::countPoints(const std::vector<Card*>& cards) {
    int points = 0;
    for (auto* card : cards) {
        points += card->getPoints();
    }
    return points;
}

int AiPlayer::countMagicCards(const std::vector<Card*>& cards) {
    return static_cast<int>(std::count_if(cards.begin(), cards.end(), [](const Card* card) { return card->isMagic(); }));
}

int AiPlayer::getCanastaScore(const std::vector<Card*>& cards) {
    int magicCardsCount = countMagicCards(cards);
    if (magicCardsCount == static_cast<int>(cards.size())) return MAGIC_CARDS_CANASTA_SCORE;
    if (magicCardsCount == 0) return NATURAL_CANASTA_SCORE;
    return MIXED_CANASTA_SCORE;
}

const char* AiPlayer::strategyName(AiStrategy strategy) {
    switch (strategy) {
        case AiStrategy::OPENING: return "OPENING";
        case AiStrategy::BUILDING: return "BUILDING";
        case AiStrategy::GOING_OUT: return "GOING_OUT";
        case AiStrategy::DEFENSIVE: return "DEFENSIVE";
    }
    return "UNKNOWN";
}
