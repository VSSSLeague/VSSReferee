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
    bool isTwoPlayersAttacking();
    VSSRef::Color attackingTeam();

private:
    // Timers
    QHash<VSSRef::Color, Timer*> _timers;

    // Flag control
    QHash<VSSRef::Color, bool> _twoAttacking;
};

#endif // CHECKER_TWOATTACKERS_H
