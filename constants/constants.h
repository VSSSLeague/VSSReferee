#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QtCore>
#include <include/vssref_common.pb.h>

class Constants
{
public:
    Constants(QString fileName);
    QString _fileName;
    void loadConstants();

    /// Getters

    // Physic data
    int getQtPlayers() const;
    float getBallRadius() const;
    float getRobotLength() const;

    // Network
    QString getVisionAddress() const;
    int getVisionPort() const;
    QString getRefereeAddress() const;
    int getRefereePort() const;
    QString getReplacerAddress() const;
    int getReplacerPort() const;
    QString getFiraSimAddress() const;
    int getFiraSimPort();

    // Teams
    QString getLeftTeamName() const;
    VSSRef::Color getLeftTeamColor() const;
    QString getRightTeamName() const;
    VSSRef::Color getRightTeamColor() const;

    // Parameters
    float getBallMinimumVelocity() const;
    float getPlacementWaitTime() const;
    float getStopWaitTime() const;
    float getGKTakeoutTime() const;
    float getBallStuckTime() const;
    float getPenaltyTime() const;
    float getGameHalfTime() const;

private:
    // General informations about the robots and ball
    int _qtPlayers;
    float _ballRadius;
    float _robotLength;

    // Vision Network
    QString _visionAddress;
    int _visionPort;

    // Referee Network
    QString _refereeAddress;
    int _refereePort;

    // Replacer Network
    QString _replacerAddress;
    int _replacerPort;

    // FiraSim Network
    QString _firaSimAddress;
    int _firaSimPort;

    // Team left
    QString _teamLeftName;
    VSSRef::Color _teamLeftColor;

    // Team right
    QString _teamRightName;
    VSSRef::Color _teamRightColor;

    // Parameters
    float _ballMinimumVelocity;
    float _placementWaitTime;
    float _stopWaitTime;
    float _gkTakeoutTime;
    float _ballStuckTime;
    float _penaltyTime;
    float _gameHalfTime;
};

#endif // CONSTANTS_H
