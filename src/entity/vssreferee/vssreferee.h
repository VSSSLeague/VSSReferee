#ifndef VSSREFEREE_H
#define VSSREFEREE_H

#include <src/entity/vssvisionclient/vssvisionclient.h>

#include <utils/utils.h>

#include <include/vssref_command.pb.h>
#include <include/vssref_placement.pb.h>

#include <QtCore>
#include <QUdpSocket>
#include <include/timer.h>

#include <constants/constants.h>

class VSSReferee : public Entity
{
    Q_OBJECT
public:
    VSSReferee(VSSVisionClient *visionClient, const QString& refereeAddress, int refereePort, Constants *constants);
    ~VSSReferee();
    QString name();

    // Network
    bool connect(const QString& refereeAddress, int refereePort);
    void disconnect();
    bool isConnected() const;

private:
    void initialization();
    void loop();
    void finalization();

    // Constants
    Constants *_constants;
    Constants* getConstants();

    // Socket to send foul data
    QUdpSocket _socket;
    QString _refereeAddress;
    int _refereePort;
    VSSRef::ref_to_team::VSSRef_Command _refereeCommand;
    void sendPacket(VSSRef::ref_to_team::VSSRef_Command command, bool isStop = false);

    // VSSVisionClient to receive data from FIRASim
    VSSVisionClient *_visionClient;

    // Foul name by id
    QString getFoulNameById(VSSRef::Foul foul);

    // Foul placement analysis
    Timer _placementTimer;
    bool _yellowSent;
    bool _blueSent;
    bool _placementIsSet;
    QMutex _placementMutex;

    // Stop after placement analysis
    bool _stopEnabled;
    bool _manualStop;
    bool _manualGameOn;
    Timer _stopTimer;

    // Game time analysis
    Timer _gameTimer;
    int timePassed;
    VSSRef::Half _gameHalf;

    /// FOULS

    // Reset all timers
    void resetFoulTimers();

    // Two players inside goal area with ball
    bool checkTwoPlayersInsideGoalAreaWithBall();
    bool _startedPenaltyTimer;
    Timer _penaltyTimer;

    // Two players inside enemy goal area with ball
    bool checkTwoPlayersAttackingAtGoalArea();

    // GK alone with ball analysis
    bool checkGKTakeoutTimeout();
    Timer _gkTimer;
    bool startedGKTimer;

    // Ball stuck
    bool checkBallStucked();
    Timer _ballStuckTimer;
    Timer _ballVelTimer;
    vector2d lastBallPos;
    bool startedStuckTimer;

    // Goal check
    bool checkGoal();

    // Foul setter
    void setTeamFoul(VSSRef::Foul foul, VSSRef::Color forTeam, VSSRef::Quadrant foulQuadrant = VSSRef::Quadrant::NO_QUADRANT, bool isStop = false);

    // Goalie check
    void updateGoalieTimers();
    float **time;
    Timer **timers;

signals:
    void setFoul(VSSRef::Foul foul, VSSRef::Color color, VSSRef::Quadrant quadrant); // send foul to replacer (reset it vars)
    void halfPassed();                      // when half passed
    void goalMarked(VSSRef::Color team);    // when an goal occurred
    void stopReplacerWaiting();             // stop when reached timeout or both teams placed
    void sendGoalie(VSSRef::Color team, int playerId);

public slots:
    void teamSent(VSSRef::Color color);     // if a team sent it packet
    void requestGoalie(VSSRef::Color team); // request team goalie
    void takeManualCommand(VSSRef::Foul foul, VSSRef::Color color, VSSRef::Quadrant quadrant); // manual referee commands from UI
};

#endif // VSSREFEREE_H
