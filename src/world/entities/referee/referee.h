#ifndef REFEREE_H
#define REFEREE_H

#include <QUdpSocket>
#include <QSignalMapper>

#include <src/world/entities/entity.h>
#include <src/world/entities/replacer/replacer.h>
#include <src/world/entities/referee/checkers/checkers.h>

// Abstract SoccerView
class SoccerView;

class Referee : public Entity
{
    Q_OBJECT
public:
    Referee(Vision *vision, Replacer *replacer, SoccerView *soccerView, Constants *constants);
    bool isGameOn();

private:
    // Entity inherited methods
    void initialization();
    void loop();
    void finalization();

    // Vision
    Vision *_vision;

    // Replacer
    Replacer *_replacer;

    // SoccerView
    SoccerView *_soccerView;

    // Referee client
    QUdpSocket *_refereeClient;
    QString _refereeAddress;
    quint16 _refereePort;
    void connectClient();
    void disconnectClient();

    // Constants
    Constants *_constants;
    Constants* getConstants();

    // Penalties info
    VSSRef::Foul _lastFoul;
    VSSRef::Color _lastFoulTeam;
    VSSRef::Quadrant _lastFoulQuadrant;
    bool _isToPlaceOutside;
    QMutex _foulMutex;
    void updatePenaltiesInfo(VSSRef::Foul foul, VSSRef::Color foulTeam, VSSRef::Quadrant foulQuadrant, bool isManual = false);
    void sendPenaltiesToNetwork();

    // Checker management
    QSignalMapper *_mapper;
    void addChecker(Checker *checker, int priority);
    void resetCheckers();
    void deleteCheckers();

    // Checkers
    QHash<int, QVector<Checker*>*> _checkers;
    QMutex _checkerMutex;

    // Stucked ball checker
    Checker_StuckedBall *_stuckedBallChecker;

    // Half and timestamp management
    Checker_HalfTime *_halfChecker;
    VSSRef::Half _gameHalf;
    VSSRef::Color _halfKickoff;
    float _timeStamp;

    // Goalie management
    Checker_Goalie *_goalieChecker;

    // Atk and def checkers
    Checker_TwoAttackers *_twoAtkChecker;
    Checker_TwoDefenders *_twoDefChecker;
    Checker_BallPlay *_ballPlayChecker;

    // Foul transition management
    Timer _transitionTimer;
    QMutex _transitionMutex;
    bool _resetedTimer;
    bool _isStopped;
    bool _teamsPlaced;
    bool _isEndGame;
    bool _isPenaltyShootout;
    void resetTransitionVars();

    // Collision
    void checkIfTeamsAreColliding();

    // Game control
    void sendControlFoul(VSSRef::Foul foul);
    bool _gameHalted;
    bool _longStop;
    bool _goalOccurred;

    // Placement collision control
    bool _forceDefault;
    VSSRef::Foul _collisionFoul;
    VSSRef::Color _collisionColor;
    VSSRef::Quadrant _collisionQuadrant;

    // Halt placement
    Position _lastBallPosition;
    Velocity _lastBallVelocity;
    bool _placedLast;

signals:
    void sendFoul(VSSRef::Foul foul, VSSRef::Color foulColor, VSSRef::Quadrant foulQuadrant);
    void sendTimestamp(float halftime, float timestamp, VSSRef::Half half, bool isEndGame);
    void callReplacer(bool forceDefault, bool isToPlaceOutside);
    void saveFrame();
    void placeFrame();
    void placeBall(Position position, Velocity velocity);
    void emitSuggestion(QString suggestion, VSSRef::Color forTeam = VSSRef::Color::NONE, VSSRef::Quadrant atQuadrant = VSSRef::Quadrant::NO_QUADRANT);

public slots:
    void processChecker(QObject *checker);
    void halfPassed();
    void teamsPlaced();
    void goalOccurred(VSSRef::Color);
    void takeManualFoul(VSSRef::Foul foul, VSSRef::Color foulColor, VSSRef::Quadrant foulQuadrant, bool isToPlaceOutside = false);
    void takeStuckedTime(float time);
    void processCollision(VSSRef::Foul foul, VSSRef::Color foulColor, VSSRef::Quadrant foulQuadrant, bool isToPlaceOutside);
    void processCollisionDecision();
};

#endif // REFEREE_H
