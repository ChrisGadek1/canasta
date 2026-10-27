#include "card.h"
#include "player.h"
#include <string>
#include <algorithm>
#include <QDebug>
#include <QImage>
#include <QtOpenGL>

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <stdlib.h>

GLuint Card::reverseTextureId = 0;

const std::unordered_map<CardValue, int, std::hash<int>> Card::pointsByValue = {
    {CardValue::TWO, 20},
    {CardValue::THREE, 5},
    {CardValue::FOUR, 5},
    {CardValue::FIVE, 5},
    {CardValue::SIX, 5},
    {CardValue::SEVEN, 5},
    {CardValue::EIGHT, 10},
    {CardValue::NINE, 10},
    {CardValue::TEN, 10},
    {CardValue::JACK, 10},
    {CardValue::QUEEN, 10},
    {CardValue::KING, 10},
    {CardValue::ACE, 20},
    {CardValue::JOKER, 50}
};

std::string Card::texturePath() const {
    std::string cardValueToInt;
    switch (value) {
        case CardValue::TWO:   cardValueToInt = "2"; break;
        case CardValue::THREE: cardValueToInt = "3"; break;
        case CardValue::FOUR:  cardValueToInt = "4"; break;
        case CardValue::FIVE:  cardValueToInt = "5"; break;
        case CardValue::SIX:   cardValueToInt = "6"; break;
        case CardValue::SEVEN: cardValueToInt = "7"; break;
        case CardValue::EIGHT: cardValueToInt = "8"; break;
        case CardValue::NINE:  cardValueToInt = "9"; break;
        case CardValue::TEN:   cardValueToInt = "10"; break;
        case CardValue::JACK:  cardValueToInt = "jack"; break;
        case CardValue::QUEEN: cardValueToInt = "queen"; break;
        case CardValue::KING:  cardValueToInt = "king"; break;
        case CardValue::ACE:   cardValueToInt = "ace"; break;
        case CardValue::JOKER: cardValueToInt = "joker"; break;
    }

    std::string suitToString;
    switch (suit) {
        case CardSuit::DIAMONDS: suitToString = "diamonds"; break;
        case CardSuit::CLUBS:    suitToString = "clubs"; break;
        case CardSuit::SPADES:   suitToString = "spades"; break;
        case CardSuit::HEARTS:   suitToString = "hearts"; break;
    }

    if (value == CardValue::JOKER) {
        return (suit == CardSuit::SPADES || suit == CardSuit::CLUBS)
            ? "black_joker.png" : "red_joker.png";
    }

    return cardValueToInt + "_of_" + suitToString + ".png";
}

void Card::loadTexture() {
    QString path = QString(":/images/images/cards/%1").arg(QString::fromStdString(texturePath()));
    QImage img;
    if (!img.load(path)) {
        qDebug() << "Nie udało się wczytać tekstury:" << path;
        return;
    }

    QImage glImg = QGLWidget::convertToGLFormat(img);

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 glImg.width(), glImg.height(),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, glImg.bits());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Card::draw(bool reverse, float angleDegrees) const
{
    GLuint texture = reverse ? Card::reverseTextureId : textureId;
    if (!texture) return;
    glBindTexture(GL_TEXTURE_2D, texture);

    float verts[4][2] = {
        { drawPosition.left,  drawPosition.down },
        { drawPosition.right, drawPosition.down },
        { drawPosition.right, drawPosition.top  },
        { drawPosition.left,  drawPosition.top  }
    };

    float tex[4][2] = {
        {0.f, 0.f}, {1.f, 0.f}, {1.f, 1.f}, {0.f, 1.f}
    };

    float rad = angleDegrees * M_PI / 180.0f;
    float c = cos(rad), s = sin(rad);

    for (int i = 0; i < 4; i++) {
        float u = tex[i][0] - 0.5f;
        float v = tex[i][1] - 0.5f;

        float u2 = c * u - s * v;
        float v2 = s * u + c * v;

        tex[i][0] = u2 + 0.5f;
        tex[i][1] = v2 + 0.5f;
    }

    glBegin(GL_QUADS);
    for (int i = 0; i < 4; i++) {
        glTexCoord2f(tex[i][0], tex[i][1]);
        glVertex2f(verts[i][0], verts[i][1]);
    }
    glEnd();
}


bool Card::advanceAnimation(const Position& target, const void* container, float progressDelta) {
    if (!(target == animationTarget)) {
        animationStart = drawPosition;
        animationTarget = target;
        animationProgress = 0.0f;
        animationChangesContainer = container != animationContainer;
    }
    animationContainer = container;
    if (!isAnimating()) return false;
    animationProgress = std::min(1.0f, animationProgress + progressDelta);
    float easedProgress = 1 - (1 - animationProgress) * (1 - animationProgress);
    drawPosition = Position::lerp(animationStart, animationTarget, easedProgress);
    return isAnimating();
}

void Card::loadReverseTexture() {
    QString path = QString(":/images/images/cards/card_reverse.png");
    QImage img;
    if (!img.load(path)) {
        qDebug() << "Nie udało się wczytać tekstury:" << path;
        return;
    }

    QImage glImg = QGLWidget::convertToGLFormat(img);

    glGenTextures(1, &Card::reverseTextureId);
    glBindTexture(GL_TEXTURE_2D, Card::reverseTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 glImg.width(), glImg.height(),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, glImg.bits());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

bool Card::isChosen() {
    return this->chosen;
}

void Card::setChosen(int playerNumber) {
    if(playerNumber == 0) {
        Position cardPos = this->position;
        if(this->chosen) {
            cardPos.down -= 0.06;
            cardPos.top -= 0.06;
            this->chosen = false;
        }
        else {
            cardPos.down += 0.06;
            cardPos.top += 0.06;
            this->chosen = true;
        }
        this->position = cardPos;
    }
}
