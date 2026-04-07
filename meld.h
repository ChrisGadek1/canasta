#ifndef MELD_H
#define MELD_H
#include "cardvalue.h"
#include "player.h"
#include <vector>
#include "card.h"

class Meld
{
    CardValue value;
    std::vector<Card*> cards;
    Player player;
    bool allNonMagicCardsHaveTheSameValue() const;
    bool hasAtLeastTwoNonMagicCards() const;
    bool hasAtLeastThreeTotalCards() const;
    bool magicCardsNotExceedingNonMagicCards() const;
    bool isMagicCard(CardValue cardValue) const;
    int countMagicCards() const;
 public:
    Meld(CardValue value, Player player) : value(value), player(player) {}
    CardValue getValue();
    void addCard(Card* card);
    std::vector<Card*>& getCards();
    Player& getPlayer();
    bool valid() const;
};

#endif // MELD_H
