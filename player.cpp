#include "player.h"
#include <string>


std::string Player::getName() const {
    return this->name;
}

int Player::getNumber() const {
    return this->number;
}

bool Player::getIsHumanControlled() const {
    return this->isHumanControlled;
}
