#ifndef CHECKER_STUCKEDBALL_H
#define CHECKER_STUCKEDBALL_H

#include <src/world/entities/referee/checkers/checker.h>

class Checker_StuckedBall : public Checker
{
    Q_OBJECT
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

signals:
    void sendStuckedTime(float time);
};

#endif // CHECKER_STUCKEDBALL_H
