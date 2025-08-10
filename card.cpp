#include "card.h"
#include "player.h"
#include <string>


std::string Card::texturePath() {
    if(this->value == CardValue::JOKER) {
        if(this->suit == CardSuit::SPADES || this->suit == CardSuit::CLUBS) {
            return "black_joker.svg";
        }
        else {
            return "red_joker.svg";
        }
    }

    std::string cardValueToInt;
    switch (this->value) {
        case CardValue::TWO:
            cardValueToInt = "2";
            break;
        case CardValue::THREE:
            cardValueToInt = "3";
            break;
        case CardValue::FOUR:
            cardValueToInt = "4";
            break;
        case CardValue::FIVE:
            cardValueToInt = "5";
            break;
        case CardValue::SIX:
            cardValueToInt = "6";
            break;
        case CardValue::SEVEN:
            cardValueToInt = "7";
            break;
        case CardValue::EIGHT:
            cardValueToInt = "8";
            break;
        case CardValue::NINE:
            cardValueToInt = "9";
            break;
        case CardValue::TEN:
            cardValueToInt = "10";
            break;
        case CardValue::JACK:
            cardValueToInt = "jack";
            break;
        case CardValue::QUEEN:
            cardValueToInt = "queen";
            break;
        case CardValue::KING:
            cardValueToInt = "king";
            break;
        case CardValue::ACE:
            cardValueToInt = "ace";
            break;
        default:
            cardValueToInt = "joker";
    }

    std::string suitToString;

    switch(this->suit) {
        case CardSuit::DIAMONDS:
            suitToString = "diamonds";
            break;
        case CardSuit::CLUBS:
            suitToString = "clubs";
            break;
        case CardSuit::SPADES:
            suitToString = "spades";
            break;
        case CardSuit::HEARTS:
            suitToString = "hearts";
            break;
    }

    return cardValueToInt + "_of_" + suitToString + ".svg";
}
