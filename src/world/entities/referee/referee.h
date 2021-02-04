#ifndef REFEREE_H
#define REFEREE_H

#include <QUdpSocket>
#include <QSignalMapper>

#include <src/world/entities/entity.h>
#include <src/world/entities/referee/checkers/halftime/checker_halftime.h>

class Referee : public Entity
{
    Q_OBJECT
public:
    Referee(Vision *vision, Constants *constants);

    QPair<VSSRef::Foul, VSSRef::Color> getLastPenaltyInfo();

private:
    // Entity inherited methods
    void initialization();
    void loop();
    void finalization();

    // Vision
    Vision *_vision;

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
    QMutex _foulMutex;
    void updatePenaltiesInfo(VSSRef::Foul foul, VSSRef::Color foulTeam, VSSRef::Quadrant foulQuadrant);
    void sendPenaltiesToNetwork();

    // Checker management
    QSignalMapper *_mapper;
    void addChecker(Checker *checker, int priority);
    void resetCheckers();
    void deleteCheckers();

    // Checkers
    QHash<int, QVector<Checker*>*> _checkers;

    // Half and timestamp management
    Checker_HalfTime *_halfChecker;
    VSSRef::Half _gameHalf;
    float _timeStamp;

    // Foul transition management
    Timer _transitionTimer;
    QMutex _transitionMutex;
    bool _resetedTimer;
    bool _isStopped;
    bool _teamsPlaced;

signals:
    void sendFoul(VSSRef::Foul foul);

public slots:
    void processChecker(QObject *checker);
    void halfPassed();
    void teamsPlaced();
};

#endif // REFEREE_H
