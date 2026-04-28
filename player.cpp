#include "player.h"
#include <string>


std::string Player::getName() {
    return this->name;
}

int Player::getNumber() const {
    return this->number;
}

bool Player::getIsHumanControlled() {
    return this->isHumanControlled;
}
