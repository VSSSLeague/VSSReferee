#ifndef VSSREPLACER_H
#define VSSREPLACER_H

#include <QUdpSocket>

#include <src/entity/entity.h>
#include <include/vssclient/vssclient.h>
#include <include/vssref_placement.pb.h>
#include <include/replacement.pb.h>

#include <src/entity/refereeview/refereeview.h>
#include <src/entity/refereeview/soccerview/util/field_default_constants.h>
#include <src/entity/refereeview/soccerview/util/geometry.h>

#include <constants/constants.h>
#include <utils/utils.h>

#include <random>
#include <chrono>

class VSSReplacer : public Entity
{
    Q_OBJECT
public:
    VSSReplacer(const QString& refereeAddress, int replacerPort, const QString& firaSimAddress, int firaSimCommandPort, Constants* constants);
    ~VSSReplacer();
    QString name();

private:
    // Entity inherited methods
    void initialization();
    void loop();
    void finalization();

    // VSS Client to receive replacement data
    VSSClient *_vssClient;
    VSSRef::Frame frames[2];
    QString _refereeAddress;
    int _replacerPort;

    // Replacement data control
    bool _blueSentPacket;
    bool _yellowSentPacket;
    bool _awaitingPackets;
    int _packetsReceived;
    QMutex _mutex;

    VSSRef::Foul _foul;
    VSSRef::Color _color;
    VSSRef::Quadrant _quadrant;

    // Goal kick control
    bool _isGoaliePlacedAtTop;

    // Ball replacement
    vector2d getBallPlaceByFoul(VSSRef::Foul foul, VSSRef::Color color, VSSRef::Quadrant quadrant);

    // Socket to send replacement commands to firaSim
    QUdpSocket _socket;
    QString _firaSimAddress;
    int _firaSimCommandPort;
    bool connect(const QString &firaSimAddress, int firaSimCommandPort);
    bool isConnected() const;
    void sendPacket(fira_message::sim_to_ref::Packet command);
    void disconnect();
    void fillAndSendPacket(VSSRef::Frame *frame);
    void placeBall(double x, double y);
    void parseRobot(fira_message::sim_to_ref::Replacement *replacementPacket, VSSRef::Robot *robot, VSSRef::Color robotTeam);
    void debugFrame(VSSRef::Frame frame);

    // Team's goalie
    int goalie[2];
    QMutex _goalieMutex;

    // Constants
    Constants* _constants;
    Constants* getConstants();

    // Utils
    QString getFoulNameById(VSSRef::Foul foul);
    VSSRef::Frame* getPenaltyPlacement(VSSRef::Color color);
    VSSRef::Frame* getGoalKickPlacement(VSSRef::Color color);
    VSSRef::Frame* getFreeBallPlacement(VSSRef::Color color);
    VSSRef::Frame* getKickoffPlacement(VSSRef::Color color);

signals:
    void teamPlaced(VSSRef::Color color);
    void requestGoalie(VSSRef::Color team);

public slots:
    void takeGoalie(VSSRef::Color team, int id);
    void takeFoul(VSSRef::Foul foul, VSSRef::Color color, VSSRef::Quadrant quadrant);
    void stopWaiting();
};

#endif // VSSREPLACER_H
