#ifndef CHECKER_BALLPLAY_H
#define CHECKER_BALLPLAY_H

#include <src/world/entities/referee/checkers/twoattackers/checker_twoattackers.h>
#include <src/world/entities/referee/checkers/twodefenders/checker_twodefenders.h>

class Checker_BallPlay : public Checker
{
    Q_OBJECT
public:
    // Using Checker constructor
    using Checker::Checker;

    // Foul inherited methods
    QString name();
    void configure();
    void run();

    // Set checkers
    void setAtkDefCheckers(Checker_TwoAttackers *twoAtk, Checker_TwoDefenders *twoDef);

private:
    // Flag control
    bool _isPlayRunning;
    bool _possiblePenalty;
    bool _possibleGoalKick;
    bool _possibleGoal;
    VSSRef::Color _possibleGoalTeam;

    // Ball in area management
    Timer _areaTimer;
    bool _areaTimerControl;

    // Checkers
    Checker_TwoAttackers *_checkerTwoAtk;
    Checker_TwoDefenders *_checkerTwoDef;

signals:
    void emitGoal(VSSRef::Color forTeam);
    void emitSuggestion(QString suggestion, VSSRef::Color forTeam = VSSRef::Color::NONE);
};

#endif // CHECKER_BALLPLAY_H
