#include "aiplayer.h"
#include "aiprobability.h"
#include <algorithm>
#include <cstdio>

void AiPlayer::startTurn(Game& game) {
    this->phase = AiTurnPhase::DRAW;
    this->chooseStrategy(game);
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
        if (static_cast<int>(game.getPlayerDeck(opponentNumber).getCards().size()) <= DEFENSIVE_OPPONENT_HAND_SIZE) {
            opponentAlmostOut = true;
        }
    }
    int handSize = static_cast<int>(game.getPlayerDeck(this->playerNumber).getCards().size());
    if (opponentAlmostOut || static_cast<int>(game.getHiddenDeck().getCards().size()) < DEFENSIVE_HIDDEN_DECK_SIZE) {
        this->strategy = AiStrategy::DEFENSIVE;
    } else if (!game.hasTeamInitialMeld(this->playerNumber)) {
        this->strategy = AiStrategy::OPENING;
    } else if (game.hasTeamCanasta(this->playerNumber) && handSize <= GOING_OUT_HAND_SIZE) {
        this->strategy = AiStrategy::GOING_OUT;
    } else {
        this->strategy = AiStrategy::BUILDING;
    }
    printf("P%d strategy: %s\n", this->playerNumber, strategyName(this->strategy));
}

void AiPlayer::playDrawPhase(Game& game) {
    double expectedDrawGain = AiProbability(game, this->playerNumber).expectedDrawGain();
    game.drawFromHiddenDeck(this->playerNumber);
    if (game.hasCurrentPlayerDrawnThisTurn()) {
        printf("P%d draws from the hidden deck (ev=%.1f)\n", this->playerNumber, expectedDrawGain);
    } else if (!game.isRoundFinished()) {
        printf("P%d could not draw a card\n", this->playerNumber);
    }
}

bool AiPlayer::playMeldStep(Game& game) {
    std::vector<Card*>& handCards = game.getPlayerDeck(this->playerNumber).getCards();
    int handSizeBefore = static_cast<int>(handCards.size());
    int maxCardsToMeld = handSizeBefore - this->getMinimumRemainingCardsCount(game);
    if (maxCardsToMeld <= 0) return false;

    std::map<CardValue, std::vector<Card*>> groups = this->groupNaturalCardsByValue(game);
    std::vector<Meld>& melds = game.getMelds();
    const Player& player = game.getPlayers()[this->playerNumber];
    for (int meldIndex = 0; meldIndex < static_cast<int>(melds.size()); meldIndex++) {
        auto group = groups.find(melds[meldIndex].getValue());
        if (group == groups.end() || melds[meldIndex].isRedThreesMeld() || !melds[meldIndex].isOwnedBy(player)) continue;
        std::vector<Card*> cards = group->second;
        if (static_cast<int>(cards.size()) > maxCardsToMeld) cards.resize(maxCardsToMeld);
        if (!game.canAddCardsToMeld(this->playerNumber, meldIndex, cards)) continue;
        game.addCardsToMeld(this->playerNumber, meldIndex, cards);
        if (static_cast<int>(handCards.size()) < handSizeBefore) {
            printf("P%d adds %d cards to the meld of value %d\n", this->playerNumber, static_cast<int>(cards.size()), group->first);
            return true;
        }
    }

    for (const auto& cards : this->chooseNewMelds(game)) {
        game.createMeld(this->playerNumber, cards);
        if (static_cast<int>(handCards.size()) < handSizeBefore) {
            printf("P%d melds %d cards of value %d\n", this->playerNumber, static_cast<int>(cards.size()), cards.front()->value);
            return true;
        }
    }
    return false;
}

void AiPlayer::playDiscardPhase(Game& game) {
    std::vector<Card*> handCards = game.getPlayerDeck(this->playerNumber).getCards();
    std::vector<std::pair<int, Card*>> cardsByCost;
    for (auto* card : handCards) {
        cardsByCost.push_back(std::make_pair(this->getDiscardCost(game, card), card));
    }
    std::stable_sort(cardsByCost.begin(), cardsByCost.end(), [](const std::pair<int, Card*>& a, const std::pair<int, Card*>& b) { return a.first < b.first; });
    for (const auto& costAndCard : cardsByCost) {
        Card* card = costAndCard.second;
        if (!game.canDiscard(this->playerNumber, card)) continue;
        game.discard(this->playerNumber, card);
        if (game.isRoundFinished() || game.getCurrentPlayerIndex() != this->playerNumber) {
            printf("P%d discards card of value %d (cost %d)\n", this->playerNumber, card->value, costAndCard.first);
            return;
        }
    }
    printf("P%d could not discard any card\n", this->playerNumber);
}

std::map<CardValue, std::vector<Card*>> AiPlayer::groupNaturalCardsByValue(Game& game) {
    std::map<CardValue, std::vector<Card*>> groups;
    for (auto* card : game.getPlayerDeck(this->playerNumber).getCards()) {
        if (card->isMagic() || card->value == CardValue::THREE) continue;
        groups[card->value].push_back(card);
    }
    return groups;
}

std::vector<std::vector<Card*>> AiPlayer::chooseNewMelds(Game& game) {
    const Player& player = game.getPlayers()[this->playerNumber];
    std::vector<std::vector<Card*>> candidates;
    for (const auto& group : this->groupNaturalCardsByValue(game)) {
        if (group.second.size() >= 3 && !game.hasMeldWithValue(group.first, player)) {
            candidates.push_back(group.second);
        }
    }
    auto points = [](const std::vector<Card*>& cards) {
        int sum = 0;
        for (auto* card : cards) {
            sum += card->getPoints();
        }
        return sum;
    };
    std::stable_sort(candidates.begin(), candidates.end(), [points](const std::vector<Card*>& a, const std::vector<Card*>& b) { return points(a) > points(b); });

    int capacity = static_cast<int>(game.getPlayerDeck(this->playerNumber).getCards().size()) - this->getMinimumRemainingCardsCount(game);
    int chosenPoints = 0;
    std::vector<std::vector<Card*>> chosenMelds;
    for (const auto& cards : candidates) {
        if (static_cast<int>(cards.size()) > capacity) continue;
        capacity -= static_cast<int>(cards.size());
        chosenPoints += points(cards);
        chosenMelds.push_back(cards);
    }
    if (!game.hasTeamInitialMeld(this->playerNumber) && game.getTeamMeldsPoints(this->playerNumber) + chosenPoints < game.getTeamInitialMeldMinimumPoints(this->playerNumber)) {
        return {};
    }
    return chosenMelds;
}

int AiPlayer::getMinimumRemainingCardsCount(Game& game) const {
    return game.hasTeamCanasta(this->playerNumber) ? 1 : 2;
}

int AiPlayer::getDiscardCost(Game& game, Card* card) {
    if (card->isBlackThree()) return 0;
    if (card->isMagic()) return WILD_CARD_DISCARD_COST + card->getPoints();
    std::vector<Card*>& handCards = game.getPlayerDeck(this->playerNumber).getCards();
    int sameValueCount = static_cast<int>(std::count_if(handCards.begin(), handCards.end(), [card](const Card* handCard) { return handCard->value == card->value; }));
    return PAIR_DISCARD_COST * (sameValueCount - 1) + card->getPoints();
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
