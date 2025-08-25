#include "card.h"
#include "player.h"
#include <string>
#include <QDebug>
#include <QImage>
#include <QtOpenGL>

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

GLuint Card::reverseTextureId = 0;

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
    GLuint tex = reverse ? Card::reverseTextureId : textureId;
    if (!tex) return;
    glBindTexture(GL_TEXTURE_2D, tex);

    const float w  = position.right - position.left;
    const float h  = position.top   - position.down;
    const float cx = (position.left + position.right) / 2.0f;
    const float cy = (position.down + position.top)   / 2.0f;

    glPushMatrix();
    glTranslatef(cx, cy, 0.0f);
    glRotatef(angleDegrees, 0.0f, 0.0f, 1.0f);

    const float hw = w * 0.5f;
    const float hh = h * 0.5f;

    glBegin(GL_QUADS);
        glTexCoord2f(0.f, 0.f); glVertex2f(-hw, -hh);
        glTexCoord2f(1.f, 0.f); glVertex2f( hw, -hh);
        glTexCoord2f(1.f, 1.f); glVertex2f( hw,  hh);
        glTexCoord2f(0.f, 1.f); glVertex2f(-hw,  hh);
    glEnd();

    glPopMatrix();
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
