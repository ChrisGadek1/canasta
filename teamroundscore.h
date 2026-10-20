#ifndef TEAMROUNDSCORE_H
#define TEAMROUNDSCORE_H

class TeamRoundScore
{
    int canastasPoints;
    int redThreesPoints;
    int goingOutPoints;
    int meldedCardsPoints;
    int handCardsPoints;
public:
    TeamRoundScore(int canastasPoints, int redThreesPoints, int goingOutPoints, int meldedCardsPoints, int handCardsPoints):
        canastasPoints(canastasPoints), redThreesPoints(redThreesPoints), goingOutPoints(goingOutPoints), meldedCardsPoints(meldedCardsPoints), handCardsPoints(handCardsPoints) {}
    int getCanastasPoints() const { return canastasPoints; }
    int getRedThreesPoints() const { return redThreesPoints; }
    int getGoingOutPoints() const { return goingOutPoints; }
    int getMeldedCardsPoints() const { return meldedCardsPoints; }
    int getHandCardsPoints() const { return handCardsPoints; }
    int getTotalPoints() const { return canastasPoints + redThreesPoints + goingOutPoints + meldedCardsPoints + handCardsPoints; }
};

#endif // TEAMROUNDSCORE_H
