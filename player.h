#ifndef PLAYER_H
#define PLAYER_H

#include <string>

class Player
{
    bool isHumanControlled;
    std::string name;
public:
    Player(bool isHumanControlled, std::string name): isHumanControlled(isHumanControlled), name(name) {}
};

#endif // PLAYER_H
