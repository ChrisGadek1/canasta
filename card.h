#ifndef CARD_H
#define CARD_H

#include "player.h"
#include "cardsuit.h"
#include "cardvalue.h"
#include <string>
#include <QtOpenGL>

class Card
{
    GLuint textureId;
public:
    CardValue value;
    CardSuit suit;
    Card(CardValue value, CardSuit suit): textureId(0), value(value), suit(suit) {}
    ~Card() {
        glDeleteTextures(1, &textureId);
    }

    std::string texturePath() const;
    void loadTexture();
    void draw(float x, float y, float w, float h) const;
};

#endif // CARD_H
