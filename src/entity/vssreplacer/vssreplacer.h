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

class VSSReplacer : public Entity
{
    Q_OBJECT
public:
    VSSReplacer(const QString& refereeAddress, int replacerPort, const QString& firaSimAddress, int firaSimCommandPort);
    ~VSSReplacer();
    QString name();

private:
    // Entity inherited methods
    void initialization();
    void loop();
    void finalization();

    // VSS Client to receive replacement data
    VSSClient *_vssClient;
    VSSRef::Frame frames[3];
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

    // Ball replacement
    vector2d getBallPlaceByFoul(VSSRef::Foul foul, VSSRef::Color color, VSSRef::Quadrant quadrant);

    // Socket to send replacement commands to firaSim
    QUdpSocket _socket;
    QString _firaSimAddress;
    fira_message::sim_to_ref::Replacement *_replacementCommand;
    int _firaSimCommandPort;
    bool connect(const QString &firaSimAddress, int firaSimCommandPort);
    bool isConnected() const;
    void sendPacket(fira_message::sim_to_ref::Packet command);
    void disconnect();
    void fillPacket(VSSRef::Frame frameBlue, VSSRef::Frame frameYellow);
    void placeBall(double x, double y);
    void parseRobot(VSSRef::Robot *robot, VSSRef::Color robotTeam);
    void debugFrame(VSSRef::Frame frame);

    // Utils
    QString getFoulNameById(VSSRef::Foul foul);

signals:
    void teamPlaced(VSSRef::Color color);

public slots:
    void takeFoul(VSSRef::Foul foul, VSSRef::Color color, VSSRef::Quadrant quadrant);
    void stopWaiting();
};

#endif // VSSREPLACER_H
