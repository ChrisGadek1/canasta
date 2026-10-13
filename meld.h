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
    std::vector<Player> owners;
    bool redThrees;
    bool allNonMagicCardsHaveTheSameValue() const;
    bool hasAtLeastTwoNonMagicCards() const;
    bool hasAtLeastThreeTotalCards() const;
    bool magicCardsNotExceedingNonMagicCards() const;
    bool isMagicCard(CardValue cardValue) const;
    int countMagicCards() const;
    void moveMagicCardsToFront();
 public:
    Meld(CardValue value, const std::vector<Player>& owners, bool redThrees = false) : value(value), owners(owners), redThrees(redThrees) {}
    bool isRedThreesMeld() const { return redThrees; }
    CardValue getValue() const;
    void addCard(Card* card);
    const std::vector<Card*>& getCards() const;
    const std::vector<Player>& getOwners() const;
    bool isOwnedBy(const Player& player) const;
    bool valid() const;
    bool canAddCards(const std::vector<Card*>& cards) const;
};

#endif // MELD_H
