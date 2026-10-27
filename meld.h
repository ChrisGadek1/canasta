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
    bool hasNoBlackThrees() const;
    bool hasAtMostMaxSizeCards() const;
    bool isMagicCard(CardValue cardValue) const;
    int countMagicCards() const;
    void moveMagicCardsToFront();
 public:
    Meld(CardValue value, const std::vector<Player>& owners, bool redThrees = false) : value(value), owners(owners), redThrees(redThrees) {}
    static constexpr int CANASTA_SIZE = 7;
    static constexpr int MAX_SIZE = 8;
    bool isRedThreesMeld() const { return redThrees; }
    bool isFull() const { return !redThrees && static_cast<int>(cards.size()) >= MAX_SIZE; }
    bool isCanasta() const { return !redThrees && static_cast<int>(cards.size()) >= CANASTA_SIZE; }
    bool isNaturalCanasta() const { return isCanasta() && countMagicCards() == 0; }
    bool isMagicCardsCanasta() const { return isCanasta() && countMagicCards() == static_cast<int>(cards.size()); }
    static CardValue baseValue(const std::vector<Card*>& cards);
    CardValue getValue() const;
    void addCard(Card* card);
    const std::vector<Card*>& getCards() const;
    const std::vector<Player>& getOwners() const;
    bool isOwnedBy(const Player& player) const;
    bool valid() const;
    bool canAddCards(const std::vector<Card*>& cards) const;
    int getPoints() const;
};

#endif // MELD_H
