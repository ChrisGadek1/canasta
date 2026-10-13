#ifndef ROUNDRESULT_H
#define ROUNDRESULT_H

class RoundResult
{
    int team02Points;
    int team13Points;
public:
    RoundResult(int team02Points, int team13Points): team02Points(team02Points), team13Points(team13Points) {}
    int getTeam02Points() const { return team02Points; }
    int getTeam13Points() const { return team13Points; }
};

#endif // ROUNDRESULT_H
