#ifndef CHECKER_TWODEFENDERS_H
#define CHECKER_TWODEFENDERS_H

#include <src/world/entities/referee/checkers/checker.h>

class Checker_TwoDefenders : public Checker
{
public:
    // Using Checker constructor
    using Checker::Checker;

    // Foul inherited methods
    QString name();
    void configure();
    void run();

    // Flag getter
    bool isAnyTeamDefendingWithMoreThanPossible();
    bool isTwoPlayersDefendingAtGoalArea(VSSRef::Color teamColor);
    bool isFivePlayersDefendingAtBigArea(VSSRef::Color teamColor);
    float getTimer();
    VSSRef::Color defendingTeam();

private:
    // Timers
    QHash<VSSRef::Color, Timer*> _timers;
};

#endif // CHECKER_TWODEFENDERS_H
