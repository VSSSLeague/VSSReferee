#ifndef VSSREPLACER_H
#define VSSREPLACER_H

#include <QUdpSocket>

#include <src/entity/entity.h>
#include <include/vssclient/vssclient.h>
#include <include/vssref_placement.pb.h>
#include <include/replacement.pb.h>

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

    // Timer
    Timer _timer;

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
    void takeFoul(VSSRef::Foul foul);
};

#endif // VSSREPLACER_H
