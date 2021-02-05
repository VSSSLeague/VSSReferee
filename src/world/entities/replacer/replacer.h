#ifndef REPLACER_H
#define REPLACER_H

#include <src/world/entities/entity.h>
#include <src/world/entities/vision/vision.h>
#include <include/vssref_placement.pb.h>
#include <include/packet.pb.h>

class Replacer : public Entity
{
    Q_OBJECT
public:
    Replacer(Vision *vision, Constants *constants);

private:
    // Entity inherited methods
    void initialization();
    void loop();
    void finalization();

    // Replacer Network
    QUdpSocket *_replacerClient;
    QString _replacerAddress;
    quint16 _replacerPort;

    // FIRASim Network
    QUdpSocket *_firaClient;
    QString _firaAddress;
    quint16 _firaPort;

    // Network management
    void bindAndConnect();
    void disconnectClient();

    // Vision
    Vision *_vision;

    // Constants
    Constants *_constants;
    Constants* getConstants();

    // Goalies management
    QHash<VSSRef::Color, quint8> _goalies;
    quint8 getGoalie(VSSRef::Color color);
    QMutex _goalieMutex;

    // Fouls management
    VSSRef::Foul _foul;
    VSSRef::Color _foulColor;
    VSSRef::Quadrant _foulQuadrant;
    VSSRef::Foul getFoul();
    VSSRef::Color getFoulColor();
    VSSRef::Quadrant getFoulQuadrant();
    QMutex _foulMutex;
    bool _foulProcessed;

    // Placement management
    QHash<VSSRef::Color, VSSRef::Frame> _placement;
    QHash<VSSRef::Color, bool> _placementStatus;
    bool _isGoaliePlacedAtTop;
    Position getBallPlaceByFoul(VSSRef::Foul foul, VSSRef::Color color, VSSRef::Quadrant quadrant);
    VSSRef::Frame* getPenaltyPlacement(VSSRef::Color color);
    VSSRef::Frame* getGoalKickPlacement(VSSRef::Color color);
    VSSRef::Frame* getFreeBallPlacement(VSSRef::Color color);
    VSSRef::Frame* getKickoffPlacement(VSSRef::Color color);
    void placeFrame(VSSRef::Frame *frame);
    void placeBall();
    void stopRobots();

signals:
    void teamsPlaced();

public slots:
    void takeGoalie(VSSRef::Color color, quint8 playerId);
    void takeFoul(VSSRef::Foul foul, VSSRef::Color foulColor, VSSRef::Quadrant foulQuadrant);
    void placeTeams();

};

#endif // REPLACER_H
