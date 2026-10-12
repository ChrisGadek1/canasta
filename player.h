#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <array>

class Player
{
    bool isHumanControlled;
    std::string name;
    int number;
public:
    Player(bool isHumanControlled, std::string name, int number): isHumanControlled(isHumanControlled), name(name), number(number) {}
    Player(): isHumanControlled(false), name("<placeholder>"), number(-1) {}
    std::string getName() const;
    int getNumber() const;
    bool getIsHumanControlled() const;
};

#endif // PLAYER_H
