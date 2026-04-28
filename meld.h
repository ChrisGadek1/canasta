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
    void moveMagicCardsToFront();
 public:
    Meld(CardValue value, Player player) : value(value), player(player) {}
    CardValue getValue() const;
    void addCard(Card* card);
    const std::vector<Card*>& getCards() const;
    const Player& getPlayer() const;
    bool valid() const;
    bool canAddCards(const std::vector<Card*>& cards) const;
};

#endif // MELD_H
