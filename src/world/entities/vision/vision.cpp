#include "vision.h"

#include <include/packet.pb.h>

Vision::Vision(Constants *constants) : Entity(ENT_VISION) {
    // Taking constants
    _constants = constants;

    // Taking network data
    _visionAddress = getConstants()->visionAddress();
    _visionPort = getConstants()->visionPort();

    // Init objects
    initObjects();
}

Vision::~Vision() {
    deleteObjects();
}

void Vision::initialization() {
    // Binding and connecting in network
    bindAndConnect();

    std::cout << Text::blue("[VISION] ", true) + Text::bold("Module started at address '" + _visionAddress.toStdString() + "' and port '" + std::to_string(_visionPort) + "'.") + '\n';
}

void Vision::loop() {
    while(_visionClient->hasPendingDatagrams()) {
        // Creating auxiliary vars
        fira_message::sim_to_ref::Environment environmentData;
        QNetworkDatagram datagram;

        // Reading datagram and checking if it is valid
        datagram = _visionClient->receiveDatagram();
        if(!datagram.isValid()) {
            continue;
        }

        // Parsing datagram and checking if it worked properly
        if(environmentData.ParseFromArray(datagram.data().data(), datagram.data().size()) == false) {
            std::cout << Text::blue("[VISION] ", true) << Text::red("Wrapper packet parsing error.", true) + '\n';
            continue;
        }

        // Iterate received vision frame
        if(environmentData.has_frame()) {
            // Lock mutex for write
            _dataMutex.lockForWrite();

            // Clear objects control
            clearObjectsControl();

            // Take frame
            fira_message::Frame frame = environmentData.frame();

            // Parse ball
            if(frame.has_ball()) {
                _ballObject->updateObject(1.0f, Position(true, frame.ball().x(), frame.ball().y()));
            }
            else {
                _ballObject->updateObject(0.0f, Position(false, 0.0, 0.0));
            }

            // Parse blue robots
            for(int i = 0; i < frame.robots_blue_size(); i++) {
                // Take robot
                fira_message::Robot robot = frame.robots_blue(i);

                // Take id
                quint8 robotId = robot.robot_id();

                // Get object
                Object *robotObject = _objects.value(VSSRef::Color::BLUE)->value(robotId);
                robotObject->updateObject(1.0f, Position(true, robot.x(), robot.y()), Angle(true, robot.orientation()));

                // Update control to true
                _objectsControl.value(VSSRef::Color::BLUE)->insert(robotId, true);
            }

            // Parse yellow robots
            for(int i = 0; i < frame.robots_yellow_size(); i++) {
                // Take robot
                fira_message::Robot robot = frame.robots_yellow(i);

                // Take id
                quint8 robotId = robot.robot_id();

                // Get object
                Object *robotObject = _objects.value(VSSRef::Color::YELLOW)->value(robotId);
                robotObject->updateObject(1.0f, Position(true, robot.x(), robot.y()), Angle(true, robot.orientation()));

                // Update control to true
                _objectsControl.value(VSSRef::Color::YELLOW)->insert(robotId, true);
            }

            // Parse robots that didn't appeared
            for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
                // Take control hash
                QHash<quint8, bool> *idsControl = _objectsControl.value(VSSRef::Color(i));

                // Take ids list and iterate on it
                QList<quint8> idList = idsControl->keys();
                QList<quint8>::iterator it;

                for(it = idList.begin(); it != idList.end(); it++) {
                    // If not updated (== false)
                    if(idsControl->value((*it)) == false) {
                        // Take object
                        Object *robotObject = _objects.value(VSSRef::Color(i))->value((*it));

                        // Update it with invalid values
                        robotObject->updateObject(0.0f, Position(false, 0.0, 0.0), Angle(false, 0.0));
                    }
                }
            }

            // Release mutex
            _dataMutex.unlock();

            emit visionUpdated();
        }
    }
}

void Vision::finalization() {
    // Closing socket
    if(_visionClient->isOpen()) {
        _visionClient->close();
    }

    // Deleting vision client
    delete _visionClient;

    std::cout << Text::blue("[VISION] ", true) + Text::bold("Module finished.") + '\n';
}

void Vision::bindAndConnect() {
    // Creating socket
    _visionClient = new QUdpSocket();

    // Binding in defined network data
    if(_visionClient->bind(QHostAddress(_visionAddress), _visionPort, QUdpSocket::ShareAddress) == false) {
        std::cout << Text::blue("[VISION] " , true) << Text::red("Error while binding socket.", true) + '\n';
        return ;
    }

    // Joining multicast group
    if(_visionClient->joinMulticastGroup(QHostAddress(_visionAddress)) == false) {
        std::cout << Text::blue("[VISION] ", true) << Text::red("Error while joining multicast.", true) + '\n';
        return ;
    }
}

void Vision::initObjects() {
    // Init ball object
    _ballObject = new Object(getConstants()->useKalman());

    // Init robot objects
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        // Init objects
        QHash<quint8, Object*> *teamObjects = new QHash<quint8, Object*>();
        _objects.insert(VSSRef::Color(i), teamObjects);
        for(int j = 0; j < getConstants()->qtPlayers(); j++) {
            teamObjects->insert(j, new Object(getConstants()->useKalman()));
        }

        // Init objects control
        QHash<quint8, bool> *teamObjectsControl = new QHash<quint8, bool>();
        _objectsControl.insert(VSSRef::Color(i), teamObjectsControl);
        for(int j = 0; j < getConstants()->qtPlayers(); j++) {
            teamObjectsControl->insert(j, false);
        }
    }
}

void Vision::deleteObjects() {
    // Deleting ball object
    delete _ballObject;

    // Deleting player objects
    QList<VSSRef::Color> teamColors = _objects.keys();
    QList<VSSRef::Color>::iterator it;

    for(it = teamColors.begin(); it != teamColors.end(); it++) {
        // Take team objects control and delete it
        QHash<quint8, bool> *teamObjectsControl = _objectsControl.take((*it));
        delete teamObjectsControl;

        // Take team objects
        QHash<quint8, Object*> *teamObjects = _objects.take((*it));

        // Take registered ids and iterate on it
        QList<quint8> idList = teamObjects->keys();
        QList<quint8>::iterator itId;

        for(itId = idList.begin(); itId != idList.end(); itId++) {
            // Take object
            Object *playerObject = teamObjects->take((*itId));
            // Delete it
            delete playerObject;
        }

        // delete teamObjects
        delete teamObjects;
    }
}

void Vision::clearObjectsControl() {
    QList<VSSRef::Color> teamColors = _objects.keys();

    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        QHash<quint8, bool> *teamObjectsControl = _objectsControl.value(VSSRef::Color(i));

        // Take registered ids
        QList<quint8> objectsId = teamObjectsControl->keys();
        for(int j = 0; j < objectsId.size(); j++) {
            teamObjectsControl->insert(objectsId.at(j), false);
        }
    }
}

QList<quint8> Vision::getAvailablePlayers(VSSRef::Color teamColor) {
    _dataMutex.lockForRead();
    QList<quint8> availableList;

    // Take object list and iterate it
    QHash<quint8, Object*> *objectList = _objects.value(teamColor);
    QList<quint8> idList = objectList->keys();
    QList<quint8>::iterator it;

    for(it = idList.begin(); it != idList.end(); it++) {
        // If object position isn't invalid, add it id to list
        if(!objectList->value((*it))->getPosition().isInvalid()) {
            availableList.push_back((*it));
        }
    }
    _dataMutex.unlock();

    return availableList;
}

Position Vision::getPlayerPosition(VSSRef::Color teamColor, quint8 playerId) {
    _dataMutex.lockForRead();
    Object *playerObject = _objects.value(teamColor)->value(playerId);
    Position pos = playerObject->getPosition();
    _dataMutex.unlock();

    return pos;
}

Velocity Vision::getPlayerVelocity(VSSRef::Color teamColor, quint8 playerId) {
    _dataMutex.lockForRead();
    Object *playerObject = _objects.value(teamColor)->value(playerId);
    Velocity vel = playerObject->getVelocity();
    _dataMutex.unlock();

    return vel;
}

Angle Vision::getPlayerOrientation(VSSRef::Color teamColor, quint8 playerId) {
    _dataMutex.lockForRead();
    Object *playerObject = _objects.value(teamColor)->value(playerId);
    Angle ori = playerObject->getOrientation();
    _dataMutex.unlock();

    return ori;
}

Position Vision::getBallPosition() {
    _dataMutex.lockForRead();
    Position pos = _ballObject->getPosition();
    _dataMutex.unlock();

    return pos;
}

Velocity Vision::getBallVelocity() {
    _dataMutex.lockForRead();
    Velocity vel = _ballObject->getVelocity();
    _dataMutex.unlock();

    return vel;
}

Constants* Vision::getConstants() {
    if(_constants == nullptr) {
        std::cout << Text::red("[ERROR] ", true) << Text::bold("Constants with nullptr value at Vision") + '\n';
    }
    else {
        return _constants;
    }

    return nullptr;
}
