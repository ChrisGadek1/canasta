#ifndef CARD_H
#define CARD_H

#include "player.h"
#include "cardsuit.h"
#include "cardvalue.h"
#include "position.h"
#include <string>
#include <QtOpenGL>

class Card
{
    GLuint textureId;
public:
    CardValue value;
    CardSuit suit;
    Position position;
    static GLuint reverseTextureId;
    Card(CardValue value, CardSuit suit): textureId(0), value(value), suit(suit), position(Position(0, 0, 0, 0)) {}
    ~Card() {
        glDeleteTextures(1, &textureId);
    }

    std::string texturePath() const;
    void loadTexture();
    void draw(bool reverse, float angleDegrees) const;
    static constexpr float ratio = 0.68870523416;
    static void loadReverseTexture();
};

#endif // CARD_H
