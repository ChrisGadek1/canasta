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
    this->humanPlayerHasDrawnThisTurn = false;
    this->humanPlayerHasDiscardedThisTurn = false;
    this->generatePlayers();
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
    initialCard->position = Position(Game::exposedLeftPos, Game::exposedBottomPos, Game::exposedBottomPos + Card::cardHeight, Game::exposedLeftPos + Card::cardHeight * Card::ratio);
    while(initialCard->isRedThree()) {
        exposedDeck.addCard(initialCard);
        initialCard = initialCards.back();
        initialCard->position = Position(Game::exposedLeftPos, Game::exposedBottomPos, Game::exposedBottomPos + Card::cardHeight, Game::exposedLeftPos + Card::cardHeight * Card::ratio);
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

Meld& Game::getRedThreesMeld(const Player& player) {
    for (auto &meld : this->melds) {
        if (meld.isRedThreesMeld() && meld.isOwnedBy(player)) return meld;
    }
    Player partner = this->players[(player.getNumber() + 2) % 4];
    std::vector<Player> meldOwners{player, partner};
    this->melds.insert(this->melds.begin(), Meld(CardValue::THREE, meldOwners, true));
    return this->melds.front();
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

void Game::addMeld(Meld& meld) {
    this->melds.push_back(meld);
}

bool Game::hasMeldWithValue(CardValue value, const Player& player) const {
    for (const auto& meld : melds) {
        if (!meld.isRedThreesMeld() && meld.getValue() == value && meld.isOwnedBy(player)) return true;
    }
    return false;
}

void Game::handleClickOnHiddenDeck() {
    if (currentPlayerIndex != 0) return;
    if (humanPlayerHasDrawnThisTurn) return;

    CardsDeck& hiddenDeck = this->getHiddenDeck();
    if (hiddenDeck.getCards().size() > 0) {
        Card* firstCard = hiddenDeck.getCards().back();
        hiddenDeck.getCards().pop_back();
        CardsDeck& humanPlayerDeck = this->getHumanPlayerDeck();
        firstCard->above = nullptr;
        firstCard->behind = nullptr;
        if (humanPlayerDeck.getCards().size() > 0) {
            Card* lastPlayersCard = humanPlayerDeck.getCards().back();
            firstCard->behind = lastPlayersCard;
            lastPlayersCard->above = firstCard;
        }
        humanPlayerDeck.addCard(firstCard);
        humanPlayerHasDrawnThisTurn = !firstCard->isRedThree();
        meldRedThrees();
        initializeCardsPositions();
    }
}

void Game::resetHumanPlayerDrawFlag() {
    humanPlayerHasDrawnThisTurn = false;
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
    if (exposedDeck.getCards().empty()) return false;
    Card* topCard = exposedDeck.getCards().back();
    Position exposedTopCardPos = Position::toPx(topCard->position.left, topCard->position.down, topCard->position.top, topCard->position.right, vw, vh);
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

bool Game::canHumanPlayerDiscard() {
    return currentPlayerIndex == 0 && humanPlayerHasDrawnThisTurn && !humanPlayerHasDiscardedThisTurn && hasExactlyOneCardSelected();
}

void Game::handleClickOnExposedDeck() {
    if (!canHumanPlayerDiscard()) return;

    CardsDeck& humanPlayerDeck = this->getHumanPlayerDeck();
    Card* selectedCard = nullptr;

    std::vector<Card*> allCards = humanPlayerDeck.getCards();
    for (auto &card : allCards) {
        if (card->isChosen()) {
            selectedCard = card;
            break;
        }
    }

    if (selectedCard) {
        std::vector<Card*>& cards = humanPlayerDeck.getCards();
        cards.erase(std::find(cards.begin(), cards.end(), selectedCard));

        CardsDeck& exposedDeck = this->getExposedDeck();
        selectedCard->setChosen(0);

        if (exposedDeck.getCards().size() > 0) {
            Card* previousTopCard = exposedDeck.getCards().back();
            selectedCard->behind = previousTopCard;
            previousTopCard->above = selectedCard;
        } else {
            selectedCard->behind = nullptr;
            selectedCard->above = nullptr;
        }

        selectedCard->position = Position(Game::exposedLeftPos, Game::exposedBottomPos, Game::exposedBottomPos + Card::cardHeight, Game::exposedLeftPos + Card::cardHeight * Card::ratio);

        exposedDeck.addCard(selectedCard);
        humanPlayerHasDiscardedThisTurn = true;

        int totalPages = getTotalPages(humanPlayerDeck.getCards());
        if (humanPlayerCardPage >= totalPages) {
            humanPlayerCardPage = std::max(0, totalPages - 1);
        }
        initializeCardsPositions();
    }
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

bool Game::canAddCardsToMeld(int meldIndex) const {
    if (meldIndex < 0 || meldIndex >= static_cast<int>(this->melds.size())) {
        printf("Cannot add card to meld - bad meldIndex: %d\n", meldIndex);
        return false;
    }
    if (this->melds[meldIndex].isRedThreesMeld()) {
        printf("Cannot add card to meld - red threes meld\n");
        return false;
    }
    std::vector<Card*> chosenCards;
    for (auto& deck : this->decks) {
        if (deck.getType() == PLAYERS_OWNED && deck.getPlayer().getNumber() == 0) {
            for (auto* card : deck.getCards()) {
                if (card->isChosen()) {
                    chosenCards.push_back(card);
                }
            }
        }
    }
    if (chosenCards.empty()) {
        printf("Cannot add card to meld - no cards chosen\n");
        return false;
    }

    return !chosenCards.empty() && this->melds[meldIndex].canAddCards(chosenCards);
}

void Game::addCardsToMeld(int meldIndex) {
    std::vector<Card*> chosenCards;
    std::vector<CardsDeck*> chosenDecks;
    for (auto& deck : this->decks) {
        if (deck.getType() == PLAYERS_OWNED && deck.getPlayer().getNumber() == 0) {
            for (auto* card : deck.getCards()) {
                if (card->isChosen()) {
                    chosenCards.push_back(card);
                    chosenDecks.push_back(&deck);
                }
            }
        }
    }
    if (chosenCards.empty() || meldIndex < 0 || meldIndex >= static_cast<int>(this->melds.size())) {
        return;
    }
    Meld& meld = this->melds[meldIndex];
    if (!meld.canAddCards(chosenCards)) return;
    for (size_t i = 0; i < chosenDecks.size(); i++) {
        meld.addCard(chosenCards[i]);
        std::vector<Card*>& cards = chosenDecks[i]->getCards();
        cards.erase(std::find(cards.begin(), cards.end(), chosenCards[i]));
    }
    initializeMeldCardPositions();
    initializeCardsPositions();
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

bool Game::canPlayerMeld(Player* player) {
    for (auto &deck: this->decks) {
        if (deck.getType() == PLAYERS_OWNED && deck.getPlayer().getNumber() == player->getNumber()) {
            std::vector<Card*> cardsToCheck = deck.getCards();
            if (cardsToCheck.empty()) {
                printf("cannot meld - cards to check are empty \n");
            }
            if (cardsToCheck.empty()) return false;
            Player partner = this->players[(player->getNumber() + 2) % 4];
            std::vector<Player> meldOwners{(*player), partner};
            Meld tempMeld(cardsToCheck[0]->value, meldOwners);
            for (auto &card: cardsToCheck) {
                if (card->isChosen()) {
                    tempMeld.addCard(card);
                }
            }
            if (tempMeld.valid()) {
                for (auto &card: cardsToCheck) {
                    if (card->isChosen()) {
                        if (hasMeldWithValue(card->value, *player)) {
                            printf("cannot meld - meld with such value already exists\n");
                            return false;
                        }
                    }
                }
            }
            return tempMeld.valid();
        }
    }
    return false;
}

bool Game::canHumanPlayerMeld() {
    auto it = std::find_if(players.begin(), players.end(),[](Player& p){ return p.getNumber() == 0; });
    if (it != players.end()) {
        Player& currentPlayerRef = *it;
        return canPlayerMeld(&currentPlayerRef);
    }
    return false;
}

void Game::initializeCardsPositions() {
    float margin = 0.4f;
    float cardHeight = Card::cardHeight;
    for (unsigned short deckI = 0; deckI < this->decks.size(); deckI++) {
        if (this->decks[deckI].getType() == PLAYERS_OWNED) {
            std::vector<Card *> cards = this->decks[deckI].getCards();
            int playerNum = this->decks[deckI].getPlayer().getNumber();

            // For human player, only position visible cards
            if (playerNum == 0) {
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
