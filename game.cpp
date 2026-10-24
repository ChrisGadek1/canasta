#include "game.h"
#include <vector>
#include "player.h"
#include "cardsuit.h"
#include "cardvalue.h"
#include "card.h"
#include <algorithm>
#include <random>
#include "cardsdeck.h"
#include "cardsdecktype.h"
#include <chrono>
#include <stdexcept>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

Game::~Game() {
    this->deleteCards();
}

void Game::deleteCards() {
    for (auto &deck : this->decks) {
        for (auto &card : deck.getCards()) {
            delete card;
        }
    }
    for (auto &meld : this->melds) {
        for (auto &card : meld.getCards()) {
            delete card;
        }
    }
}

std::vector<Player>& Game::getPlayers() {
    return this->players;
}

std::vector<CardsDeck>& Game::getDecks() {
    return this->decks;
}

void Game::initialize() {
    this->currentPlayerIndex = 0;
    this->humanPlayerCardPage = 0;
    this->currentPlayerHasDrawnThisTurn = false;
    this->teamHasInitialMeld = {{false, false}};
    this->teamHasMeldedInPreviousTurns = {{false, false}};
    this->roundFinishingPlayerNumber = -1;
    this->roundFinishedConcealed = false;
    this->roundFinishedByEmptyHiddenDeck = false;
    this->cardMemory.reset();
    this->generatePlayers();
    for (int teamIndex = 0; teamIndex < 2; teamIndex++) {
        this->teamInitialMeldMinimumPoints[teamIndex] = this->calculateInitialMeldMinimumPoints(this->players[teamIndex]);
    }
    this->generateDecks();
    this->meldRedThrees();
    this->initializeCardsPositions();
}

void Game::generatePlayers() {
    Player p1(true, "Marek", 0);
    Player p2(false, "Marian", 1);
    Player p3(false, "Andrzej", 2);
    Player p4(false, "Mariusz", 3);

    this->players = std::vector<Player>();

    this->players.push_back(p1);
    this->players.push_back(p2);
    this->players.push_back(p3);
    this->players.push_back(p4);
}

void Game::generateDecks() {
    std::vector<Card*> initialCards;

    for(int suitId = CardSuit::SPADES; suitId <= CardSuit::CLUBS; suitId++) {
        for (int cardValueId = CardValue::TWO; cardValueId <= CardValue::ACE; cardValueId++) {
            initialCards.push_back(new Card(static_cast<CardValue>(cardValueId), static_cast<CardSuit>(suitId)));
            initialCards.push_back(new Card(static_cast<CardValue>(cardValueId), static_cast<CardSuit>(suitId)));
        }
    }

    initialCards.push_back(new Card(CardValue::JOKER, CardSuit::DIAMONDS));
    initialCards.push_back(new Card(CardValue::JOKER, CardSuit::DIAMONDS));
    initialCards.push_back(new Card(CardValue::JOKER, CardSuit::SPADES));
    initialCards.push_back(new Card(CardValue::JOKER, CardSuit::SPADES));

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine e(seed);
    std::shuffle(std::begin(initialCards), std::end(initialCards), e);

    std::vector<CardsDeck> playersDecks;

    for (auto &player : this->players) {
        playersDecks.push_back(CardsDeck(CardsDeckType::PLAYERS_OWNED, std::vector<Card*>(), player));
    }

    for(int i = 0; i < 11; i++) {
        for(auto &playersDeck : playersDecks) {
            playersDeck.addCard(initialCards.back());
            initialCards.pop_back();
        }
    }

    CardsDeck exposedDeck(CardsDeckType::EXPOSED, std::vector<Card*>());

    Card* initialCard = initialCards.back();
    initialCards.pop_back();
    initialCard->position = exposedDeckPosition(initialCard->freezesExposedDeck());
    while(initialCard->freezesExposedDeck()) {
        exposedDeck.addCard(initialCard);
        initialCard = initialCards.back();
        initialCard->position = exposedDeckPosition(initialCard->freezesExposedDeck());
        initialCards.pop_back();
    }
    exposedDeck.addCard(initialCard);

    for (auto &card: initialCards) {
        constexpr float hiddenBottomPos = 0.65f;
        constexpr float hiddenRightPos = -0.65f;
        card->position = Position(hiddenRightPos - Card::cardHeight * Card::ratio, hiddenBottomPos, hiddenBottomPos + Card::cardHeight, hiddenRightPos);
    }

    CardsDeck hiddenDeck(CardsDeckType::HIDDEN, initialCards);

    std::vector<CardsDeck> decks;
    decks.insert(std::end(decks), std::begin(playersDecks), std::end(playersDecks));
    decks.push_back(exposedDeck);
    decks.push_back(hiddenDeck);
    this->decks = decks;
}

void Game::meldRedThrees() {
    for (auto &deck : this->decks) {
        if (deck.getType() != PLAYERS_OWNED) continue;
        std::vector<Card*>& cards = deck.getCards();
        for (auto it = cards.begin(); it != cards.end();) {
            if ((*it)->isRedThree()) {
                this->getRedThreesMeld(deck.getPlayer()).addCard(*it);
                it = cards.erase(it);
            } else {
                ++it;
            }
        }
    }
    this->initializeMeldCardPositions();
}

std::vector<Player> Game::getMeldOwners(int playerNumber) const {
    return {this->players[playerNumber], this->players[(playerNumber + 2) % 4]};
}

Meld& Game::getRedThreesMeld(const Player& player) {
    for (auto &meld : this->melds) {
        if (meld.isRedThreesMeld() && meld.isOwnedBy(player)) return meld;
    }
    this->melds.insert(this->melds.begin(), Meld(CardValue::THREE, this->getMeldOwners(player.getNumber()), true));
    return this->melds.front();
}

Meld* Game::findTeamMeld(CardValue value, const Player& player) {
    for (auto &meld : this->melds) {
        if (!meld.isRedThreesMeld() && meld.getValue() == value && meld.isOwnedBy(player)) return &meld;
    }
    return nullptr;
}

Meld& Game::getTeamMeld(CardValue value, const Player& player) {
    Meld* teamMeld = this->findTeamMeld(value, player);
    if (teamMeld) return *teamMeld;
    this->melds.push_back(Meld(value, this->getMeldOwners(player.getNumber())));
    return this->melds.back();
}

std::vector<Card*> Game::getHumanPlayerChosenCards() {
    std::vector<Card*> chosenCards;
    for (auto* card : this->getHumanPlayerDeck().getCards()) {
        if (card->isChosen()) {
            chosenCards.push_back(card);
        }
    }
    return chosenCards;
}

Position Game::exposedDeckPosition(bool rotated) {
    float cardWidth = Card::cardHeight * Card::ratio;
    if (!rotated) {
        return Position(Game::exposedLeftPos, Game::exposedBottomPos, Game::exposedBottomPos + Card::cardHeight, Game::exposedLeftPos + cardWidth);
    }
    float centerX = Game::exposedLeftPos + cardWidth / 2;
    float centerY = Game::exposedBottomPos + Card::cardHeight / 2;
    return Position(centerX - Card::cardHeight / 2, centerY - cardWidth / 2, centerY + cardWidth / 2, centerX + Card::cardHeight / 2);
}

CardsDeck& Game::getHiddenDeck() {
    auto hiddenDeck = std::find_if(this->getDecks().begin(), this->getDecks().end(), [](const CardsDeck& deck) { return deck.getType() == HIDDEN; });
    if (hiddenDeck != this->getDecks().end()) {
        return *hiddenDeck;
    }
    throw std::logic_error("hidden deck not found");
}

std::vector<Meld>& Game::getMelds() {
    return this->melds;
}

bool Game::hasMeldWithValue(CardValue value, const Player& player) const {
    for (const auto& meld : melds) {
        if (!meld.isRedThreesMeld() && meld.getValue() == value && meld.isOwnedBy(player)) return true;
    }
    return false;
}

void Game::drawFromHiddenDeck(int playerNumber) {
    if (currentPlayerIndex != playerNumber || currentPlayerHasDrawnThisTurn) return;

    CardsDeck& hiddenDeck = this->getHiddenDeck();
    if (hiddenDeck.getCards().empty()) return;
    CardsDeck& playerDeck = this->getPlayerDeck(playerNumber);
    while (!currentPlayerHasDrawnThisTurn && !hiddenDeck.getCards().empty()) {
        Card* firstCard = hiddenDeck.getCards().back();
        hiddenDeck.getCards().pop_back();
        firstCard->above = nullptr;
        firstCard->behind = nullptr;
        if (playerDeck.getCards().size() > 0) {
            Card* lastPlayersCard = playerDeck.getCards().back();
            firstCard->behind = lastPlayersCard;
            lastPlayersCard->above = firstCard;
        }
        playerDeck.addCard(firstCard);
        currentPlayerHasDrawnThisTurn = !firstCard->isRedThree();
    }
    roundFinishedByEmptyHiddenDeck = !currentPlayerHasDrawnThisTurn;
    Card* drawnCard = playerDeck.getCards().back();
    meldRedThrees();
    playerDeck.sortCards();
    if (playerNumber == 0) {
        std::vector<Card*>& handCards = playerDeck.getCards();
        auto drawnCardIt = std::find(handCards.begin(), handCards.end(), drawnCard);
        if (drawnCardIt != handCards.end()) {
            humanPlayerCardPage = static_cast<int>(drawnCardIt - handCards.begin()) / CARDS_PER_PAGE;
        }
    }
    initializeCardsPositions();
}

void Game::handleClickOnHiddenDeck() {
    this->drawFromHiddenDeck(0);
}

CardsDeck& Game::getPlayerDeck(int playerNumber) {
    auto playerDeck = std::find_if(this->getDecks().begin(), this->getDecks().end(), [playerNumber](const CardsDeck& deck) { return deck.getType() == PLAYERS_OWNED && deck.getPlayer().getNumber() == playerNumber; });
    if (playerDeck != this->getDecks().end()) {
        return *playerDeck;
    }
    throw std::logic_error("player deck not found");
}

CardsDeck& Game::getHumanPlayerDeck() {
    auto humanPlayerDec = std::find_if(this->getDecks().begin(), this->getDecks().end(), [](const CardsDeck& deck) { return deck.getType() == PLAYERS_OWNED && deck.getPlayer().getIsHumanControlled(); });
    if (humanPlayerDec != this->getDecks().end()) {
        return *humanPlayerDec;
    }
    throw std::logic_error("human player deck not found");
}

CardsDeck& Game::getExposedDeck() {
    auto exposedDeck = std::find_if(this->getDecks().begin(), this->getDecks().end(), [](const CardsDeck& deck) { return deck.getType() == EXPOSED; });
    if (exposedDeck != this->getDecks().end()) {
        return *exposedDeck;
    }
    throw std::logic_error("exposed deck not found");
}

bool Game::mouseOverExposedDeck(int vw, int vh, int x, int y) {
    CardsDeck& exposedDeck = this->getExposedDeck();
    Position topCardPos = exposedDeck.getCards().empty() ? exposedDeckPosition() : exposedDeck.getCards().back()->position;
    Position exposedTopCardPos = Position::toPx(topCardPos.left, topCardPos.down, topCardPos.top, topCardPos.right, vw, vh);
    return exposedTopCardPos.down > y && exposedTopCardPos.top < y && exposedTopCardPos.right > x && exposedTopCardPos.left < x;
}

bool Game::hasExactlyOneCardSelected() {
    int selectedCount = 0;
    for (auto &deck : this->getDecks()) {
        if (deck.getType() == PLAYERS_OWNED && deck.getPlayer().getNumber() == 0) {
            // Check all cards (not just visible) to support multi-page selection
            std::vector<Card*> cardsToCheck = deck.getCards();
            for (auto &card : cardsToCheck) {
                if (card->isChosen()) {
                    selectedCount++;
                }
            }
        }
    }
    return selectedCount == 1;
}

bool Game::canDiscard(int playerNumber, Card* card) {
    std::vector<Card*>& handCards = this->getPlayerDeck(playerNumber).getCards();
    return currentPlayerIndex == playerNumber && currentPlayerHasDrawnThisTurn && std::find(handCards.begin(), handCards.end(), card) != handCards.end();
}

bool Game::canHumanPlayerDiscard() {
    return hasExactlyOneCardSelected() && canDiscard(0, this->getHumanPlayerChosenCards().front());
}

bool Game::canTakeExposedDeck(int playerNumber, const std::vector<Card*>& cards) {
    if (currentPlayerIndex != playerNumber || currentPlayerHasDrawnThisTurn) return false;
    std::vector<Card*>& exposedCards = this->getExposedDeck().getCards();
    if (exposedCards.empty()) return false;
    Card* topCard = exposedCards.back();
    if (topCard->isMagic() || topCard->isBlackThree()) return false;
    Meld* teamMeld = this->findTeamMeld(topCard->value, this->players[playerNumber]);
    if (cards.empty()) return !isExposedDeckFrozen() && teamMeld != nullptr;
    long matchingCardsCount = std::count_if(cards.begin(), cards.end(), [topCard](const Card* card) { return card->value == topCard->value; });
    if (matchingCardsCount < 2) return false;
    std::vector<Card*> meldCards = cards;
    meldCards.push_back(topCard);
    if (teamMeld) return teamMeld->canAddCards(meldCards);

    Meld newMeld(topCard->value, this->getMeldOwners(playerNumber));
    for (auto* card : meldCards) {
        newMeld.addCard(card);
    }
    return newMeld.valid() && (hasTeamInitialMeld(playerNumber) || newMeld.getPoints() >= getTeamInitialMeldMinimumPoints(playerNumber));
}

bool Game::canHumanPlayerTakeExposedDeck() {
    return canTakeExposedDeck(0, this->getHumanPlayerChosenCards());
}

int Game::calculateInitialMeldMinimumPoints(const Player& player) const {
    int teamPoints = 0;
    for (const auto& roundResult : this->loadRoundResults()) {
        teamPoints += player.getNumber() % 2 == 0 ? roundResult.getTeam02Points() : roundResult.getTeam13Points();
    }
    if (teamPoints < 0) return 15;
    if (teamPoints < 1500) return 50;
    if (teamPoints < 3000) return 90;
    return 120;
}

int Game::getTeamMeldsPoints(int playerNumber) const {
    int points = 0;
    for (const auto& meld : this->melds) {
        if (!meld.isRedThreesMeld() && meld.isOwnedBy(this->players[playerNumber])) {
            points += meld.getPoints();
        }
    }
    return points;
}

bool Game::hasTeamInsufficientInitialMelds(int playerNumber) const {
    int points = this->getTeamMeldsPoints(playerNumber);
    return !hasTeamInitialMeld(playerNumber) && points > 0 && points < getTeamInitialMeldMinimumPoints(playerNumber);
}

bool Game::hasTeamCanasta(int playerNumber) const {
    return std::any_of(this->melds.begin(), this->melds.end(), [this, playerNumber](const Meld& meld) { return meld.isOwnedBy(this->players[playerNumber]) && meld.isCanasta(); });
}

bool Game::wouldFinishWithoutCanasta(int playerNumber, int remainingCardsCount, int resultingMeldSize) const {
    return remainingCardsCount == 0 && resultingMeldSize < Meld::CANASTA_SIZE && !hasTeamCanasta(playerNumber);
}

bool Game::wouldMeldFinishWithoutCanasta() {
    int handSize = static_cast<int>(this->getHumanPlayerDeck().getCards().size());
    int chosenCount = static_cast<int>(this->getHumanPlayerChosenCards().size());
    return wouldFinishWithoutCanasta(0, handSize - chosenCount, chosenCount);
}

bool Game::wouldAddingCardsToMeldFinishWithoutCanasta(int meldIndex) {
    int handSize = static_cast<int>(this->getHumanPlayerDeck().getCards().size());
    int chosenCount = static_cast<int>(this->getHumanPlayerChosenCards().size());
    int meldSize = static_cast<int>(this->melds[meldIndex].getCards().size());
    return wouldFinishWithoutCanasta(0, handSize - chosenCount, meldSize + chosenCount);
}

bool Game::wouldTakingExposedDeckFinishWithoutCanasta(int playerNumber, const std::vector<Card*>& cards) {
    std::vector<Card*>& exposedCards = this->getExposedDeck().getCards();
    if (exposedCards.empty()) return false;
    int handSize = static_cast<int>(this->getPlayerDeck(playerNumber).getCards().size());
    int pileCardsToHandCount = static_cast<int>(std::count_if(exposedCards.begin(), exposedCards.end() - 1, [](const Card* card) { return !card->isRedThree(); }));
    int cardsCount = static_cast<int>(cards.size());
    Meld* teamMeld = this->findTeamMeld(exposedCards.back()->value, this->players[playerNumber]);
    int meldSize = teamMeld ? static_cast<int>(teamMeld->getCards().size()) : 0;
    return wouldFinishWithoutCanasta(playerNumber, handSize - cardsCount + pileCardsToHandCount, meldSize + cardsCount + 1);
}

bool Game::wouldClickOnExposedDeckFinishWithoutCanasta() {
    int handSize = static_cast<int>(this->getHumanPlayerDeck().getCards().size());
    if (canHumanPlayerDiscard()) return wouldFinishWithoutCanasta(0, handSize - 1, 0);
    if (!canHumanPlayerTakeExposedDeck()) return false;
    return wouldTakingExposedDeckFinishWithoutCanasta(0, this->getHumanPlayerChosenCards());
}

void Game::removeHumanTeamMelds() {
    if (hasTeamInitialMeld(0)) return;
    CardsDeck& humanPlayerDeck = this->getHumanPlayerDeck();
    for (auto it = this->melds.begin(); it != this->melds.end();) {
        if (!it->isRedThreesMeld() && it->isOwnedBy(this->players[0])) {
            for (auto* card : it->getCards()) {
                humanPlayerDeck.addCard(card);
            }
            this->cardMemory.onCardsTakenToHand(0, it->getCards());
            it = this->melds.erase(it);
        } else {
            ++it;
        }
    }
    initializeMeldCardPositions();
    initializeCardsPositions();
}

bool Game::isExposedDeckFrozen() {
    std::vector<Card*>& exposedCards = this->getExposedDeck().getCards();
    return std::any_of(exposedCards.begin(), exposedCards.end(), [](const Card* card) { return card->freezesExposedDeck(); });
}

void Game::handleClickOnExposedDeck() {
    if (wouldClickOnExposedDeckFinishWithoutCanasta()) return;
    if (canHumanPlayerDiscard()) {
        discard(0, this->getHumanPlayerChosenCards().front());
    } else if (canHumanPlayerTakeExposedDeck()) {
        takeExposedDeck(0, this->getHumanPlayerChosenCards());
    }
}

void Game::takeExposedDeck(int playerNumber, const std::vector<Card*>& cards) {
    if (!canTakeExposedDeck(playerNumber, cards) || wouldTakingExposedDeckFinishWithoutCanasta(playerNumber, cards)) return;
    CardsDeck& playerDeck = this->getPlayerDeck(playerNumber);
    std::vector<Card*>& handCards = playerDeck.getCards();
    std::vector<Card*>& exposedCards = this->getExposedDeck().getCards();

    std::vector<Card*> meldCards = cards;
    for (auto* card : meldCards) {
        handCards.erase(std::find(handCards.begin(), handCards.end(), card));
    }
    this->cardMemory.onCardsMelded(playerNumber, meldCards);
    meldCards.push_back(exposedCards.back());
    exposedCards.pop_back();
    this->cardMemory.onCardsTakenToHand(playerNumber, exposedCards);

    Meld& meld = this->getTeamMeld(meldCards.back()->value, this->players[playerNumber]);
    for (auto* card : meldCards) {
        card->resetChosen();
        meld.addCard(card);
    }
    for (auto* card : exposedCards) {
        playerDeck.addCard(card);
    }
    exposedCards.clear();

    currentPlayerHasDrawnThisTurn = true;
    teamHasInitialMeld[getTeamIndex(playerNumber)] = true;
    meldRedThrees();
    initializeCardsPositions();
    finishTurnIfCurrentPlayerHasNoCards();
}

void Game::discard(int playerNumber, Card* card) {
    if (!canDiscard(playerNumber, card) || hasTeamInsufficientInitialMelds(playerNumber)) return;

    std::vector<Card*>& handCards = this->getPlayerDeck(playerNumber).getCards();
    if (wouldFinishWithoutCanasta(playerNumber, static_cast<int>(handCards.size()) - 1, 0)) return;
    handCards.erase(std::find(handCards.begin(), handCards.end(), card));
    card->resetChosen();
    this->cardMemory.onCardDiscarded(playerNumber, card);

    CardsDeck& exposedDeck = this->getExposedDeck();
    if (exposedDeck.getCards().size() > 0) {
        Card* previousTopCard = exposedDeck.getCards().back();
        card->behind = previousTopCard;
        previousTopCard->above = card;
    } else {
        card->behind = nullptr;
        card->above = nullptr;
    }

    card->position = exposedDeckPosition(card->freezesExposedDeck());

    exposedDeck.addCard(card);
    initializeCardsPositions();
    finishCurrentPlayerTurn();
}

std::array<int, CardMemory::CARD_VALUES_COUNT> Game::getUnseenCardsCounts(int playerNumber) {
    constexpr int cardsPerValue = 8;
    constexpr int jokersCount = 4;
    std::array<int, CardMemory::CARD_VALUES_COUNT> unseenCounts;
    unseenCounts.fill(cardsPerValue);
    unseenCounts[CardValue::JOKER] = jokersCount;
    std::vector<Card*> seenCards = this->getPlayerDeck(playerNumber).getCards();
    std::vector<Card*>& exposedCards = this->getExposedDeck().getCards();
    seenCards.insert(seenCards.end(), exposedCards.begin(), exposedCards.end());
    for (const auto& meld : this->melds) {
        seenCards.insert(seenCards.end(), meld.getCards().begin(), meld.getCards().end());
    }
    for (const auto* card : seenCards) {
        unseenCounts[card->value]--;
    }
    for (int otherPlayerNumber = 0; otherPlayerNumber < CardMemory::PLAYERS_COUNT; otherPlayerNumber++) {
        if (otherPlayerNumber == playerNumber) continue;
        for (int value = 0; value < CardMemory::CARD_VALUES_COUNT; value++) {
            unseenCounts[value] -= this->cardMemory.getKnownInHand(otherPlayerNumber, static_cast<CardValue>(value));
        }
    }
    for (auto& unseenCount : unseenCounts) {
        unseenCount = std::max(unseenCount, 0);
    }
    return unseenCounts;
}

void Game::finishCurrentPlayerTurn() {
    int teamIndex = getTeamIndex(currentPlayerIndex);
    if (this->getPlayerDeck(currentPlayerIndex).getCards().empty()) {
        roundFinishingPlayerNumber = currentPlayerIndex;
        roundFinishedConcealed = !teamHasMeldedInPreviousTurns[teamIndex];
        return;
    }
    if (this->getHiddenDeck().getCards().empty()) {
        roundFinishedByEmptyHiddenDeck = true;
        return;
    }
    if (getTeamMeldsPoints(currentPlayerIndex) > 0) {
        teamHasInitialMeld[teamIndex] = true;
        teamHasMeldedInPreviousTurns[teamIndex] = true;
    }
    currentPlayerIndex = (currentPlayerIndex + 1) % static_cast<int>(this->players.size());
    currentPlayerHasDrawnThisTurn = false;
}

void Game::finishTurnIfCurrentPlayerHasNoCards() {
    if (this->getPlayerDeck(currentPlayerIndex).getCards().empty()) {
        finishCurrentPlayerTurn();
    }
}

bool Game::wouldMeldingCardsFinishWithInsufficientInitialMelds(int playerNumber, const std::vector<Card*>& cards) {
    if (cards.size() != this->getPlayerDeck(playerNumber).getCards().size() || hasTeamInitialMeld(playerNumber)) return false;
    int cardsPoints = 0;
    for (auto* card : cards) {
        cardsPoints += card->getPoints();
    }
    return getTeamMeldsPoints(playerNumber) + cardsPoints < getTeamInitialMeldMinimumPoints(playerNumber);
}

bool Game::wouldMeldingChosenCardsFinishWithInsufficientInitialMelds() {
    return wouldMeldingCardsFinishWithInsufficientInitialMelds(0, this->getHumanPlayerChosenCards());
}

bool Game::mouseOverHiddenDeck(int vw, int vh, int x, int y) {
    CardsDeck& hiddenDeck = this->getHiddenDeck();
    if (hiddenDeck.getCards().empty()) return false;
    Card* topCard = hiddenDeck.getCards().back();
    Position hiddenTopCardPos = Position::toPx(topCard->position.left, topCard->position.down, topCard->position.top, topCard->position.right, vw, vh);
    return hiddenTopCardPos.down > y && hiddenTopCardPos.top < y && hiddenTopCardPos.right > x && hiddenTopCardPos.left < x;
}

bool Game::mouseOverMeld(int vw, int vh, int x, int y, int& meldIndex) const {
    for (int i = static_cast<int>(this->melds.size()) - 1; i >= 0; i--) {
        const Meld& meld = this->melds[i];
        Player humanPlayer = this->players[0];
        if (!meld.isOwnedBy(humanPlayer)) continue;
        std::vector<Card*> cards = meld.getCards();
        for (int ci = static_cast<int>(cards.size()) - 1; ci >= 0; ci--) {
            const auto& card = cards[ci];
            Position cardPos = Position::toPx(card->position.left, card->position.down, card->position.top, card->position.right, vw, vh);
            if (cardPos.down > y && cardPos.top < y && cardPos.left < x && cardPos.right > x) {
                meldIndex = i;
                return true;
            }
        }
    }
    return false;
}

bool Game::canAddCardsToMeld(int playerNumber, int meldIndex, const std::vector<Card*>& cards) const {
    if (currentPlayerIndex != playerNumber || !currentPlayerHasDrawnThisTurn) return false;
    if (meldIndex < 0 || meldIndex >= static_cast<int>(this->melds.size())) {
        printf("Cannot add card to meld - bad meldIndex: %d\n", meldIndex);
        return false;
    }
    if (this->melds[meldIndex].isRedThreesMeld()) {
        printf("Cannot add card to meld - red threes meld\n");
        return false;
    }
    if (!this->melds[meldIndex].isOwnedBy(this->players[playerNumber])) {
        printf("Cannot add card to meld - meld of the opponents\n");
        return false;
    }
    if (cards.empty()) {
        printf("Cannot add card to meld - no cards chosen\n");
        return false;
    }
    return this->melds[meldIndex].canAddCards(cards);
}

bool Game::canAddCardsToMeld(int meldIndex) {
    return canAddCardsToMeld(0, meldIndex, this->getHumanPlayerChosenCards());
}

void Game::addCardsToMeld(int playerNumber, int meldIndex, const std::vector<Card*>& cards) {
    if (!canAddCardsToMeld(playerNumber, meldIndex, cards) || wouldMeldingCardsFinishWithInsufficientInitialMelds(playerNumber, cards)) return;
    std::vector<Card*>& handCards = this->getPlayerDeck(playerNumber).getCards();
    Meld& meld = this->melds[meldIndex];
    int cardsCount = static_cast<int>(cards.size());
    if (wouldFinishWithoutCanasta(playerNumber, static_cast<int>(handCards.size()) - cardsCount, static_cast<int>(meld.getCards().size()) + cardsCount)) return;
    for (auto* card : cards) {
        card->resetChosen();
        meld.addCard(card);
        handCards.erase(std::find(handCards.begin(), handCards.end(), card));
    }
    this->cardMemory.onCardsMelded(playerNumber, cards);
    initializeMeldCardPositions();
    initializeCardsPositions();
    finishTurnIfCurrentPlayerHasNoCards();
}

void Game::addCardsToMeld(int meldIndex) {
    addCardsToMeld(0, meldIndex, this->getHumanPlayerChosenCards());
}

void Game::initializeMeldCardPositions() {
    this->initializeTeamMeldCardPositions(this->players[0]);
    this->initializeTeamMeldCardPositions(this->players[1]);
}

void Game::initializeTeamMeldCardPositions(const Player& player) {
    std::vector<Meld*> teamMelds;
    for (auto &meld : this->melds) {
        if (meld.isOwnedBy(player)) teamMelds.push_back(&meld);
    }

    float margin = 0.4f;
    float marginBottom = 0.5f;
    float cardHeight = Card::cardHeight;
    float cardWidth = cardHeight * Card::ratio;
    float leftX = -0.9f + margin;
    float rightX = leftX + cardWidth;
    float offsetX = (2.0 + leftX * 2 - cardWidth) / std::max(1, static_cast<int>(teamMelds.size()) - 1);

    constexpr float meldCardGap = 0.1f;

    for (auto &meld : teamMelds) {
        float startingY = -1 + cardHeight + marginBottom;
        float cardStep = meldCardGap;

        std::vector<Card*> meldCards = meld->getCards();
        int n = meldCards.size();
        for (int cardI = 0; cardI < n; cardI++) {
            meldCards[cardI]->position.setLeft(leftX);
            meldCards[cardI]->position.setRight(rightX);
            meldCards[cardI]->position.setDown(startingY - cardHeight);
            meldCards[cardI]->position.setTop(startingY);
            startingY += cardStep;
            meldCards[cardI]->above = cardI < n - 1 ? meldCards[cardI + 1] : nullptr;
            meldCards[cardI]->behind = cardI > 0 ? meldCards[cardI - 1] : nullptr;
        }

        leftX += offsetX;
        rightX = leftX + cardWidth;
    }
}

bool Game::canCreateMeld(int playerNumber, const std::vector<Card*>& cards) {
    if (currentPlayerIndex != playerNumber || !currentPlayerHasDrawnThisTurn || cards.empty()) return false;
    Meld meld(Meld::baseValue(cards), this->getMeldOwners(playerNumber));
    for (auto* card : cards) {
        meld.addCard(card);
    }
    if (meld.valid() && hasMeldWithValue(meld.getValue(), this->players[playerNumber])) {
        printf("cannot meld - meld with such value already exists\n");
        return false;
    }
    return meld.valid();
}

bool Game::canHumanPlayerMeld() {
    return canCreateMeld(0, this->getHumanPlayerChosenCards());
}

void Game::createMeld(int playerNumber, const std::vector<Card*>& cards) {
    if (!canCreateMeld(playerNumber, cards) || wouldMeldingCardsFinishWithInsufficientInitialMelds(playerNumber, cards)) return;
    std::vector<Card*>& handCards = this->getPlayerDeck(playerNumber).getCards();
    int cardsCount = static_cast<int>(cards.size());
    if (wouldFinishWithoutCanasta(playerNumber, static_cast<int>(handCards.size()) - cardsCount, cardsCount)) return;
    Meld meld(Meld::baseValue(cards), this->getMeldOwners(playerNumber));
    for (auto* card : cards) {
        card->resetChosen();
        meld.addCard(card);
        handCards.erase(std::find(handCards.begin(), handCards.end(), card));
    }
    this->melds.push_back(meld);
    this->cardMemory.onCardsMelded(playerNumber, cards);
    initializeMeldCardPositions();
    initializeCardsPositions();
    finishTurnIfCurrentPlayerHasNoCards();
}

void Game::meldHumanPlayerChosenCards() {
    createMeld(0, this->getHumanPlayerChosenCards());
}

void Game::initializeCardsPositions() {
    float margin = 0.4f;
    float cardHeight = Card::cardHeight;
    for (unsigned short deckI = 0; deckI < this->decks.size(); deckI++) {
        if (this->decks[deckI].getType() == PLAYERS_OWNED) {
            this->decks[deckI].sortCards();
            std::vector<Card *> cards = this->decks[deckI].getCards();
            int playerNum = this->decks[deckI].getPlayer().getNumber();

            // For human player, only position visible cards
            if (playerNum == 0) {
                humanPlayerCardPage = std::min(humanPlayerCardPage, std::max(0, getTotalPages(cards) - 1));
                cards = this->getVisibleCards(cards);
            }

            int n = cards.size();
            if (n == 0) continue;

            if (playerNum == 0) {
                // dół
                float startingX = - (1 - margin);
                float endX = (1 - margin);
                float bottomY = - 0.9f;
                float topY = - 0.9f + cardHeight;
                float cardWidth = (topY - bottomY) * Card::ratio;
                float offsetX = (endX - startingX - cardWidth) / std::max(1, n - 1);

                for (int cardI = 0; cardI < n; cardI++) {
                    cards[cardI]->position.setLeft(startingX);
                    cards[cardI]->position.setRight(startingX + cardWidth);
                    cards[cardI]->position.setTop(topY);
                    cards[cardI]->position.setDown(bottomY);
                    if (cards[cardI]->isChosen()) {
                        cards[cardI]->position.setTop(topY + 0.06f);
                        cards[cardI]->position.setDown(bottomY + 0.06f);
                    }
                    startingX += offsetX;
                    cards[cardI]->above = cardI < n - 1 ? cards[cardI + 1] : nullptr;
                    cards[cardI]->behind = cardI > 0 ? cards[cardI - 1] : nullptr;
                }
            }
            else if (playerNum == 2) {
                // góra
                float startingX = - (1 - margin);
                float endX = (1 - margin);
                float topY = 0.9f;
                float bottomY = 0.9f - cardHeight;
                float cardWidth = (topY - bottomY) * Card::ratio;
                float offsetX = (endX - startingX - cardWidth) / std::max(1, n - 1);

                for (int cardI = 0; cardI < n; cardI++) {
                    cards[cardI]->position.setLeft(startingX);
                    cards[cardI]->position.setRight(startingX + cardWidth);
                    cards[cardI]->position.setTop(topY);
                    cards[cardI]->position.setDown(bottomY);
                    startingX += offsetX;
                }
            }
            else if (playerNum == 1) {
                // lewo (układ poziomy)
                float leftX = -0.9f;
                float rightX = leftX + cardHeight;
                float startingY = -(1 - margin);
                float endY = (1 - margin);
                float cardWidth = (rightX - leftX) * Card::ratio;
                float offsetY = (endY - startingY - cardWidth) / std::max(1, n - 1);

                for (int cardI = 0; cardI < n; cardI++) {
                    cards[cardI]->position.setLeft(leftX);
                    cards[cardI]->position.setRight(rightX);
                    cards[cardI]->position.setDown(startingY);
                    cards[cardI]->position.setTop(startingY + cardWidth);
                    startingY += offsetY;
                }
            }
            else if (playerNum == 3) {
                // prawo (układ poziomy)
                float rightX = 0.9f;
                float leftX = rightX - cardHeight;
                float startingY = -(1 - margin);
                float endY = (1 - margin);
                float cardWidth = (rightX - leftX) * Card::ratio;
                float offsetY = (endY - startingY - cardWidth) / std::max(1, n - 1);

                for (int cardI = 0; cardI < n; cardI++) {
                    cards[cardI]->position.setLeft(leftX);
                    cards[cardI]->position.setRight(rightX);
                    cards[cardI]->position.setDown(startingY);
                    cards[cardI]->position.setTop(startingY + cardWidth);
                    startingY += offsetY;
                }
            }
        }
    }
}

int Game::getTotalPages(const std::vector<Card*>& cards) const {
    if (cards.empty()) return 0;
    return (static_cast<int>(cards.size()) + CARDS_PER_PAGE - 1) / CARDS_PER_PAGE;
}

std::vector<Card*> Game::getVisibleCards(const std::vector<Card*>& allCards) const {
    std::vector<Card*> visible;
    if (allCards.size() <= static_cast<size_t>(CARDS_PER_PAGE)) {
        return allCards;
    }
    size_t start = static_cast<size_t>(humanPlayerCardPage) * CARDS_PER_PAGE;
    size_t end = std::min(start + CARDS_PER_PAGE, allCards.size());
    for (size_t i = start; i < end; i++) {
        visible.push_back(allCards[i]);
    }
    return visible;
}

void Game::nextPage() {
    CardsDeck& humanDeck = getHumanPlayerDeck();
    int totalPages = getTotalPages(humanDeck.getCards());
    if (humanPlayerCardPage < totalPages - 1) {
        humanPlayerCardPage++;
    }
}

void Game::previousPage() {
    if (humanPlayerCardPage > 0) {
        humanPlayerCardPage--;
    }
}

bool Game::shouldShowArrows() const {
    CardsDeck& humanDeck = const_cast<Game*>(this)->getHumanPlayerDeck();
    return humanDeck.getCards().size() > static_cast<size_t>(CARDS_PER_PAGE);
}

bool Game::canGoPrevious() const {
    return humanPlayerCardPage > 0;
}

bool Game::canGoNext(const std::vector<Card*>& cards) const {
    int totalPages = getTotalPages(cards);
    return humanPlayerCardPage < totalPages - 1;
}

std::vector<RoundResult> Game::loadRoundResults() const {
    std::vector<RoundResult> roundResults;
    QFile resultsFile("results.json");
    if (!resultsFile.open(QIODevice::ReadOnly)) {
        printf("cannot open results.json\n");
        return roundResults;
    }
    QJsonArray rounds = QJsonDocument::fromJson(resultsFile.readAll()).object()["rounds"].toArray();
    for (const auto& round : rounds) {
        QJsonObject roundObject = round.toObject();
        roundResults.push_back(RoundResult(roundObject["team02"].toInt(), roundObject["team13"].toInt()));
    }
    return roundResults;
}

TeamRoundScore Game::calculateTeamRoundScore(const Player& player) const {
    int canastasPoints = 0;
    int meldedCardsPoints = 0;
    int redThreesCount = 0;
    bool hasMelds = false;
    for (const auto& meld : this->melds) {
        if (!meld.isOwnedBy(player)) continue;
        if (meld.isRedThreesMeld()) {
            redThreesCount += static_cast<int>(meld.getCards().size());
            continue;
        }
        hasMelds = true;
        meldedCardsPoints += meld.getPoints();
        if (meld.isCanasta()) {
            canastasPoints += meld.isMagicCardsCanasta() ? 700 : meld.isNaturalCanasta() ? 500 : 300;
        }
    }

    int redThreesPoints = redThreesCount == 4 ? 800 : redThreesCount * 100;
    if (!hasMelds) redThreesPoints = -redThreesPoints;

    int goingOutPoints = 0;
    if (roundFinishingPlayerNumber != -1 && roundFinishingPlayerNumber % 2 == player.getNumber() % 2) {
        goingOutPoints = roundFinishedConcealed ? 200 : 100;
    }

    int handCardsPoints = 0;
    for (const auto& deck : this->decks) {
        if (deck.getType() == PLAYERS_OWNED && deck.getPlayer().getNumber() % 2 == player.getNumber() % 2) {
            for (const auto* card : deck.getCards()) {
                handCardsPoints -= card->getPoints();
            }
        }
    }
    return TeamRoundScore(canastasPoints, redThreesPoints, goingOutPoints, meldedCardsPoints, handCardsPoints);
}

void Game::saveRoundResult(const RoundResult& roundResult) const {
    QFile resultsFile("results.json");
    QJsonObject results;
    if (resultsFile.open(QIODevice::ReadOnly)) {
        results = QJsonDocument::fromJson(resultsFile.readAll()).object();
        resultsFile.close();
    }
    QJsonObject round;
    round["team02"] = roundResult.getTeam02Points();
    round["team13"] = roundResult.getTeam13Points();
    QJsonArray rounds = results["rounds"].toArray();
    rounds.append(round);
    results["rounds"] = rounds;
    if (!resultsFile.open(QIODevice::WriteOnly)) {
        printf("cannot write results.json\n");
        return;
    }
    resultsFile.write(QJsonDocument(results).toJson());
}

void Game::finishRound() {
    this->saveRoundResult(RoundResult(this->calculateTeamRoundScore(this->players[0]).getTotalPoints(), this->calculateTeamRoundScore(this->players[1]).getTotalPoints()));
    this->deleteCards();
    this->decks.clear();
    this->melds.clear();
    this->initialize();
}
