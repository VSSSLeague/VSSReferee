#ifndef CHECKER_STUCKEDBALL_H
#define CHECKER_STUCKEDBALL_H

#include <src/world/entities/referee/checkers/checker.h>

class Checker_StuckedBall : public Checker
{
public:
    // Using Checker constructor
    using Checker::Checker;

    // Foul inherited methods
    QString name();
    void configure();
    void run();

private:
    // Timer
    Timer _timer;

    // Last stuck check
    bool _isLastStuckAtGoalArea;

    // Utils
    bool havePlayersNearlyBall(VSSRef::Color teamColor);
};

#endif // CHECKER_STUCKEDBALL_H
