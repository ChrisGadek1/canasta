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
    std::vector<Card> initialCards;

    for(int suitId = CardSuit::SPADES; suitId < CardSuit::CLUBS; suitId++) {
        for (int cardValueId = CardValue::TWO; cardValueId < CardValue::ACE; cardValueId++) {
            initialCards.push_back(Card(static_cast<CardValue>(cardValueId), static_cast<CardSuit>(suitId)));
            initialCards.push_back(Card(static_cast<CardValue>(cardValueId), static_cast<CardSuit>(suitId)));
        }
    }

    initialCards.push_back(Card(CardValue::JOKER, CardSuit::DIAMONDS));
    initialCards.push_back(Card(CardValue::JOKER, CardSuit::DIAMONDS));
    initialCards.push_back(Card(CardValue::JOKER, CardSuit::SPADES));
    initialCards.push_back(Card(CardValue::JOKER, CardSuit::SPADES));

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine e(seed);
    std::shuffle(std::begin(initialCards), std::end(initialCards), e);

    std::vector<CardsDeck> playersDecks;

    for (auto &player : this->players) {
        playersDecks.push_back(CardsDeck(CardsDeckType::PLAYERS_OWNED, std::vector<Card>(), player));
    }

    for(int i = 0; i < 11; i++) {
        for(auto &playersDeck : playersDecks) {
            playersDeck.addCard(initialCards.back());
            initialCards.pop_back();
        }
    }

    CardsDeck exposedDeck(CardsDeckType::EXPOSED, std::vector<Card>());

    Card initialCard = initialCards.back();
    initialCards.pop_back();
    while(initialCard.value == CardValue::THREE && (initialCard.suit == CardSuit::DIAMONDS || initialCard.suit == CardSuit::HEARTS)) {
        exposedDeck.addCard(initialCard);
        initialCard = initialCards.back();
        initialCards.pop_back();
    }

    exposedDeck.addCard(initialCard);

    CardsDeck hiddenDeck(CardsDeckType::HIDDEN, initialCards);

    std::vector<CardsDeck> decks;
    decks.insert(std::end(decks), std::begin(playersDecks), std::end(playersDecks));
    decks.push_back(exposedDeck);
    decks.push_back(hiddenDeck);
    this->decks = decks;
}

void Game::initializeCardsPositions() {
    float margin = 0.4f;
    float cardHeight = 0.3f;
    for (unsigned short deckI = 0; deckI < this->decks.size(); deckI++) {
        if (this->decks[deckI].getType() == PLAYERS_OWNED) {
            auto &cards = this->decks[deckI].getCards();
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
                    cards[cardI].position.setLeft(startingX);
                    cards[cardI].position.setRight(startingX + cardWidth);
                    cards[cardI].position.setTop(topY);
                    cards[cardI].position.setDown(bottomY);
                    startingX += offsetX;
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
                    cards[cardI].position.setLeft(startingX);
                    cards[cardI].position.setRight(startingX + cardWidth);
                    cards[cardI].position.setTop(topY);
                    cards[cardI].position.setDown(bottomY);
                    startingX += offsetX;
                }
            }
            else if (playerNum == 1) {
                // lewo (układ pionowy)
                float leftX = -0.9f;
                float rightX = leftX + cardHeight * Card::ratio;
                float startingY = -(1 - margin);
                float endY = (1 - margin);
                float offsetY = (endY - startingY - cardHeight) / std::max(1, n - 1);

                for (int cardI = 0; cardI < n; cardI++) {
                    cards[cardI].position.setLeft(leftX);
                    cards[cardI].position.setRight(rightX);
                    cards[cardI].position.setDown(startingY);
                    cards[cardI].position.setTop(startingY + cardHeight);
                    startingY += offsetY;
                }
            }
            else if (playerNum == 3) {
                // prawo (układ pionowy)
                float rightX = 0.9f;
                float leftX = rightX - cardHeight * Card::ratio;
                float startingY = -(1 - margin);
                float endY = (1 - margin);
                float offsetY = (endY - startingY - cardHeight) / std::max(1, n - 1);

                for (int cardI = 0; cardI < n; cardI++) {
                    cards[cardI].position.setLeft(leftX);
                    cards[cardI].position.setRight(rightX);
                    cards[cardI].position.setDown(startingY);
                    cards[cardI].position.setTop(startingY + cardHeight);
                    startingY += offsetY;
                }
            }
        }
    }
}

