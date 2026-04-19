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

std::vector<Player>& Game::getPlayers() {
    return this->players;
}

std::vector<CardsDeck>& Game::getDecks() {
    return this->decks;
}

void Game::initialize() {
    this->generatePlayers();
    this->generateDecks();
    this->initializeCardsPositions();
}

void Game::generatePlayers() {
    Player p1(true, "Marek", 0);
    Player p2(false, "Jedrek", 1);
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

    for(int suitId = CardSuit::SPADES; suitId < CardSuit::CLUBS; suitId++) {
        for (int cardValueId = CardValue::TWO; cardValueId < CardValue::ACE; cardValueId++) {
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
    while(initialCard->value == CardValue::THREE && (initialCard->suit == CardSuit::DIAMONDS || initialCard->suit == CardSuit::HEARTS)) {
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

CardsDeck& Game::getHiddenDeck() {
    auto hiddenDeck = std::find_if(this->getDecks().begin(), this->getDecks().end(), [](CardsDeck deck) { return deck.getType() == HIDDEN; });
    if (hiddenDeck != this->getDecks().end()) {
        return *hiddenDeck;
    }
}

std::vector<Meld>& Game::getMelds() {
    return this->melds;
}

void Game::addMeld(Meld& meld) {
    this->melds.push_back(meld);
}

void Game::handleClickOnHiddenDeck() {
    if (currentPlayerIndex != 0) return;
    if (humanPlayerHasDrawnThisTurn) return;

    CardsDeck& hiddenDeck = this->getHiddenDeck();
    if (hiddenDeck.getCards().size() > 0) {
        Card* firstCard = hiddenDeck.getCards().back();
        hiddenDeck.getCards().pop_back();
        CardsDeck& humanPlayerDeck = this->getHumanPlayerDeck();
        if (humanPlayerDeck.getCards().size() > 0) {
            Card* lastPlayersCard = humanPlayerDeck.getCards().back();
            firstCard->behind = lastPlayersCard;
            lastPlayersCard->above = firstCard;
            humanPlayerDeck.addCard(firstCard);
            initializeCardsPositions();
        }
    }
    humanPlayerHasDrawnThisTurn = true;
}

void Game::resetHumanPlayerDrawFlag() {
    humanPlayerHasDrawnThisTurn = false;
}

CardsDeck& Game::getHumanPlayerDeck() {
    auto humanPlayerDec = std::find_if(this->getDecks().begin(), this->getDecks().end(), [](CardsDeck deck) { return deck.getType() == PLAYERS_OWNED && deck.getPlayer().getIsHumanControlled(); });
    if (humanPlayerDec != this->getDecks().end()) {
        return *humanPlayerDec;
    }
}

CardsDeck& Game::getExposedDeck() {
    auto exposedDeck = std::find_if(this->getDecks().begin(), this->getDecks().end(), [](CardsDeck deck) { return deck.getType() == EXPOSED; });
    if (exposedDeck != this->getDecks().end()) {
        return *exposedDeck;
    }
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
            for (auto &card : deck.getCards()) {
                if (card->isChosen()) {
                    selectedCount++;
                }
            }
        }
    }
    return selectedCount == 1;
}

void Game::handleClickOnExposedDeck() {
    if (currentPlayerIndex != 0) return;
    if (!hasExactlyOneCardSelected()) return;

    CardsDeck& humanPlayerDeck = this->getHumanPlayerDeck();
    Card* selectedCard = nullptr;

    for (auto &card : humanPlayerDeck.getCards()) {
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
        initializeCardsPositions();
    }
}

bool Game::mouseOverHiddenDeck(int vw, int vh, int x, int y) {
    CardsDeck& hiddenDeck = this->getHiddenDeck();
    Card* firstCard = hiddenDeck.getCards()[0];
    Position hiddenTopCardPos = Position::toPx(firstCard->position.left, firstCard->position.down, firstCard->position.top, firstCard->position.right, vw, vh);
    return hiddenTopCardPos.down > y && hiddenTopCardPos.top < y && hiddenTopCardPos.right > x && hiddenTopCardPos.left < x;
}

void Game::initializeMeldCardPositions() {
    float margin = 0.4f;
    float cardHeight = Card::cardHeight;
    float cardWidth = cardHeight * Card::ratio;
    float leftX = -0.9f + margin;
    float rightX = leftX + cardWidth;
    float offsetX = (2.0 + leftX * 2 - cardWidth) / (this->melds.size() - 1);

    for (auto &meld : this->melds) {
        Player meldPlayer = meld.getPlayer();
        float startingY = meldPlayer.getNumber() == 0 || meldPlayer.getNumber() == 2 ? 0 : (1 - margin);
        float endY = meldPlayer.getNumber() == 0 || meldPlayer.getNumber() == 2 ? - (1 - margin) : 0;
        float offsetY = (startingY - endY - cardHeight) / std::max(1, int(meld.getCards().size() - 1));

        for(auto &card: meld.getCards()) {
            card->position.setLeft(leftX);
            card->position.setRight(rightX);
            card->position.setDown(startingY - cardHeight);
            card->position.setTop(startingY);
            startingY -= offsetY;
        }

        leftX += offsetX;
        rightX = leftX + cardWidth;
    }
}

bool Game::canPlayerMeld(Player* player) {
    for (auto &deck: this->decks) {
        if (deck.getType() == PLAYERS_OWNED && deck.getPlayer().getNumber() == player->getNumber()) {
            Meld tempMeld(deck.getCards()[0]->value, *player);
            for (auto &card: deck.getCards()) {
                if (card->isChosen()) {
                    tempMeld.addCard(card);
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
            int n = cards.size();
            if (n == 0) continue;

            int playerNum = this->decks[deckI].getPlayer().getNumber();

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
                    startingX += offsetX;
                    if(cardI == 0) {
                        cards[cardI]->above = cards[cardI + 1];
                    }
                    else if(cardI > 0 && cardI < n - 1) {
                        cards[cardI]->above = cards[cardI + 1];
                        cards[cardI]->behind = cards[cardI - 1];
                    }
                    else {
                        cards[cardI]->behind = cards[cardI - 1];
                    }
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

