#ifndef VSSVISIONCLIENT_H
#define VSSVISIONCLIENT_H

#include <QReadWriteLock>

#include <src/entity/entity.h>
#include <include/timer.h>
#include <include/packet.pb.h>
#include <include/vssclient/vssclient.h>

#define CLIENT_DISCONNECTED_TIME 500 // ms

class VSSVisionClient : public Entity
{
public:
    VSSVisionClient(const QString &visionAddress = "224.5.23.2", int visionPort = 10002);
    ~VSSVisionClient();

    bool hasDetectionUpdate();
    bool hasGeometryUpdate();
    fira_message::Frame getDetectionData();
    fira_message::Field getGeometryData();

    int visionPort() { return _visionPort; }
    void setVisionPort (int visionPort);

    QString name();

private:
    // Inherited methods from Entity
    void initialization();
    void loop();
    void finalization();

    // Vision system client (receive camera data)
    VSSClient *_vssClient;
    int _visionPort;
    QString _visionAddress;
    QMutex _portMutex;

    // Packets info
    QReadWriteLock _packetsMutex;
    fira_message::Frame _detectionPacket;
    fira_message::Field _geometryPacket;

    // Flags
    bool _hasDetectionUpdate;
    bool _hasGeometryUpdate;
};

#endif // VSSVISIONCLIENT_H
