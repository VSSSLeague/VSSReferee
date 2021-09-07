#ifndef REPLACER_H
#define REPLACER_H

#include <src/utils/types/field/field.h>
#include <src/world/entities/entity.h>
#include <src/world/entities/vision/vision.h>
#include <include/vssref_placement.pb.h>
#include <include/packet.pb.h>
#include <src/utils/types/placedata/placedata.h>

class Replacer : public Entity
{
    Q_OBJECT
public:
    Replacer(QString replaceFileName, Vision *vision, Field *field, Constants *constants);

protected:
    QVariantMap documentMap() { return _documentMap; }

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

    // Field
    Field *_field;
    Field* getField();

    // Constants
    Constants *_constants;
    Constants* getConstants();

    // Placements <Foul, <Category, <Role, QVector<PlaceData>>>>
    QMap<QString, QMap<QString, QMap<QString, QVector<PlaceData>>*>*> placements;
    QMap<QString, QVector<PlaceData>> getPlacementsByFoul(QString foul);
    VSSRef::Frame getPlacementFrameByFoul(QString foul, VSSRef::Quadrant foulQuadrant, VSSRef::Color teamColor);

    // Internal placements file management
    QString _replaceFileName;
    QString _fileBuffer;
    QFile _file;

    // Internal json parse vars
    QJsonDocument _document;
    QVariantMap _documentMap;
    void parsePlacements();

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
    void placeFrame(VSSRef::Frame frame);

    // Last data (maintain ball stopped)
    Position _lastBallPosition;
    Velocity _lastBallVelocity;
    bool _placedLastPosition;
    QHash<VSSRef::Color, QHash<quint8, fira_message::Robot*>*> _lastFrame;
    QMutex _lastDataMutex;
    void clearLastData();

    // Default placement utils
    Position getBallPlaceByFoul(VSSRef::Foul foul, VSSRef::Color color, VSSRef::Quadrant quadrant);
    VSSRef::Frame getOutsideFieldPlacement(VSSRef::Color teamColor);
    VSSRef::Frame getPenaltyShootoutPlacement(VSSRef::Color teamColor, bool placeAttacker);

signals:
    void teamsPlaced();

public slots:
    void takeGoalie(VSSRef::Color color, quint8 playerId);
    void takeFoul(VSSRef::Foul foul, VSSRef::Color foulColor, VSSRef::Quadrant foulQuadrant);
    void placeTeams();
    void placeOutside(VSSRef::Foul foul, VSSRef::Color oppositeTeam);
    void saveFrameAndBall();
    void placeLastFrameAndBall();
    void placeBall(Position ballPos, Velocity ballVelocity = Velocity(true, 0.0, 0.0));
};

#endif // REPLACER_H
