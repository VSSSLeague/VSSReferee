#ifndef CHECKER_HALFTIME_H
#define CHECKER_HALFTIME_H

#include <src/world/entities/referee/checkers/checker.h>

// Abstract Referee
class Referee;

class Checker_HalfTime : public Checker
{
    Q_OBJECT
public:
    // Using Foul constructor
    using Checker::Checker;

    // Foul inherited methods
    QString name();
    void configure();
    void run();

    // Referee set
    void setReferee(Referee *referee);

    // Get timestamp
    float getTimeStamp();

private:
    // Timer
    Timer _timer;

    // Seconds
    float _secondsPassed;

    // Referee
    Referee *_referee;

signals:
    void halfPassed();
};

#endif // CHECKER_HALFTIME_H
