#ifndef CARD_H
#define CARD_H

#include "player.h"
#include "cardsuit.h"
#include "cardvalue.h"
#include <string>
#include <QtOpenGL>

class Card
{
    CardValue value;
    CardSuit suit;
    Player player;
    GLuint textureId;
public:
    Card(CardValue value, CardSuit suit, Player player): value(value), suit(suit), player(player), textureId(0) {}
    ~Card() {
        glDeleteTextures(1, &textureId);
    }

    std::string texturePath() const;
    void loadTexture();
    void draw(float x, float y, float w, float h) const;
};

#endif // CARD_H
