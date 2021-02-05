#ifndef CHECKER_GOALIE_H
#define CHECKER_GOALIE_H

#include <src/world/entities/referee/checkers/checker.h>

class Checker_Goalie : public Checker
{
    Q_OBJECT
public:
    // Using Foul constructor
    using Checker::Checker;

    // Foul inherited methods
    QString name();
    void configure();
    void run();

private:
    // Player timers
    QHash<VSSRef::Color, QHash<quint8, Timer*>*> _timers;
    QHash<VSSRef::Color, QHash<quint8, float>*> _elapsedTimeInGoal;

    // Goalies
    void updateGoalies();

signals:
    void updateGoalie(VSSRef::Color color, quint8 playerId);
};

#endif // CHECKER_GOALIE_H
