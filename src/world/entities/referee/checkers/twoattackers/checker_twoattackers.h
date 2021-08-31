#ifndef CHECKER_TWOATTACKERS_H
#define CHECKER_TWOATTACKERS_H

#include <src/world/entities/referee/checkers/checker.h>

class Checker_TwoAttackers : public Checker
{
public:
    // Using Checker constructor
    using Checker::Checker;

    // Foul inherited methods
    QString name();
    void configure();
    void run();

    // Flag getter
    bool isAnyTeamAttackingWithMoreThanPossible();
    bool isTwoPlayersAttackingAtGoalArea(VSSRef::Color teamColor);
    bool isFourPlayersAttackingAtBigArea(VSSRef::Color teamColor);
    VSSRef::Color attackingTeam();
    float getTimer();

private:
    // Timers
    QHash<VSSRef::Color, Timer*> _timers;
};

#endif // CHECKER_TWOATTACKERS_H
