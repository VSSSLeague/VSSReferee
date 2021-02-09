#ifndef CHECKER_HALFTIME_H
#define CHECKER_HALFTIME_H

#include <src/world/entities/referee/checkers/checker.h>

// Abstract Referee
class Referee;

class Checker_HalfTime : public Checker
{
    Q_OBJECT
public:
    // Using Checker constructor
    using Checker::Checker;

    // Foul inherited methods
    QString name();
    void configure();
    void run();

    // Referee set
    void setReferee(Referee *referee);

    // Get timestamp
    float getTimeStamp();

    // Overtime control
    bool isOvertime();
    void setIsOvertime(bool isOvertime);
    void setIsPenaltyShootout(bool isPenaltyShootout);

private:
    // Timer
    Timer _timer;

    // Seconds
    float _secondsPassed;
    QMutex _secondsMutex;
    bool _isOvertime;
    bool _isPenaltyShootout;

    // Referee
    Referee *_referee;

signals:
    void halfPassed();

public slots:
    void receiveTime(int seconds);
};

#endif // CHECKER_HALFTIME_H
