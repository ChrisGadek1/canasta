#ifndef CARD_H
#define CARD_H

#include "player.h"
#include "cardsuit.h"
#include "cardvalue.h"
#include "position.h"
#include <string>
#include <unordered_map>
#include <QtOpenGL>

class Card
{
    GLuint textureId;
    bool chosen = false;
    Position drawPosition;
    Position animationStart;
    Position animationTarget;
    float animationProgress = 1.0f;
    const void* animationContainer = nullptr;
    bool animationChangesContainer = false;
public:
    CardValue value;
    CardSuit suit;
    Position position;
    Card* above = nullptr;
    Card* behind = nullptr;
    static GLuint reverseTextureId;
    Card(CardValue value, CardSuit suit): textureId(0), drawPosition(Position(0, 0, 0, 0)), animationStart(Position(0, 0, 0, 0)), animationTarget(Position(0, 0, 0, 0)), value(value), suit(suit), position(Position(0, 0, 0, 0)) {}
    ~Card() {
        glDeleteTextures(1, &textureId);
    }
    std::string texturePath() const;
    void loadTexture();
    void draw(bool reverse, float angleDegrees) const;
    bool isAnimating() const { return animationProgress < 1.0f; }
    bool isMovingBetweenContainers() const { return isAnimating() && animationChangesContainer; }
    bool advanceAnimation(const Position& target, const void* container, float progressDelta);
    void snapToPosition() { drawPosition = animationStart = animationTarget = position; animationProgress = 1.0f; }
    static constexpr float ratio = 0.68870523416;
    static constexpr float cardHeight = 0.3;
    static const std::unordered_map<CardValue, int, std::hash<int>> pointsByValue;
    int getPoints() const { return pointsByValue.at(value); }
    static void loadReverseTexture();
    void setChosen(int playerNumber);
    bool isChosen();
    void resetChosen() { chosen = false; }
    bool isMagic() const { return value == CardValue::TWO || value == CardValue::JOKER; }
    bool isRedThree() const { return value == CardValue::THREE && (suit == CardSuit::DIAMONDS || suit == CardSuit::HEARTS); }
    bool freezesExposedDeck() const { return isMagic() || isRedThree(); }
    bool isBlackThree() const { return value == CardValue::THREE && (suit == CardSuit::SPADES || suit == CardSuit::CLUBS); }

    bool operator==(const Card& b) const {
        if (value != b.value || suit != b.suit) return false;
        if (above == nullptr && b.above == nullptr) return true;
        if (above == nullptr || b.above == nullptr) return false;
        return above->value == b.above->value && above->suit == b.above->suit;
    }
};

#endif // CARD_H
