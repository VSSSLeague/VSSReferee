#include "replacer.h"

#include <random>
#include <chrono>

#include <src/utils/types/field/field.h>
#include <src/utils/utils.h>

#include <QNetworkInterface>

void movePlayerToPosition(VSSRef::Robot *bot, quint8 botId, Position position, Angle orientation) {
    bot->set_orientation(0.0);
    bot->set_robot_id(botId);
    bot->set_orientation(Angle::toDegrees(orientation.value()));
    bot->set_x(position.x());
    bot->set_y(position.y());
    return;
}

void debugReplacePosition(double x, double y, int num = 0, std::string side = "") {
    std::cout << Text::blue("[REPLACER] ", true) + Text::bold("Moving player " + std::to_string(num) + " from team \"" + side + "\" " + " to position ("+ std::to_string(x) + ", " + std::to_string(y) + ")") + '\n';
}

void debugTeleport(bool teleport) {
    std::string const str_teleport = (teleport ? "enabled." : "disabled.");
    std::cout << Text::yellow("[TELEPORT] ", true) + Text::bold("Teleport is " + str_teleport) + '\n';
}

Replacer::Replacer(QString replaceFileName, Vision *vision, Field *field, Constants *constants) : Entity(ENT_REPLACER){
    // Take pointers
    _vision = vision;
    _field = field;
    _constants = constants;
    _replaceFileName = replaceFileName;

    // Opening and reading file content in buffer
    _file.setFileName(_replaceFileName);
    _file.open(QIODevice::ReadOnly | QIODevice::Text);
    _fileBuffer = _file.readAll();
    _file.close();

    // Parsing buffer to json objects
    _document = QJsonDocument::fromJson(_fileBuffer.toUtf8());
    _documentMap = _document.object().toVariantMap();

    // Parse placements
    parsePlacements();

    // Taking network data
    _replacerAddress = getConstants()->replacerAddress();
    _replacerPort = getConstants()->replacerPort();
    _firaAddress = getConstants()->firaAddress();
    _firaPort = getConstants()->firaPort();
}

void Replacer::bindAndConnect() {
    // Creating sockets
    _replacerClient = new QUdpSocket();
    _firaClient = new QUdpSocket();

    // Connecting fira to host
    _firaClient->connectToHost(_firaAddress, _firaPort, QIODevice::WriteOnly, QAbstractSocket::IPv4Protocol);

    // Binding replacer in defined network data
    if(_replacerClient->bind(QHostAddress(_replacerAddress), _replacerPort, QUdpSocket::ShareAddress) == false) {
        std::cout << Text::blue("[REPLACER] " , true) << Text::red("Error while binding socket.", true) + '\n';
        return ;
    }

    // Joining multicast group
    if(_replacerClient->joinMulticastGroup(QHostAddress(_replacerAddress), QNetworkInterface::interfaceFromName(getConstants()->networkInterface())) == false) {
        std::cout << Text::blue("[REPLACER] ", true) << Text::red("Error while joining multicast.", true) + '\n';
        return ;
    }
}

void Replacer::disconnectClient() {
    // Closing replacer socket
    if(_replacerClient->isOpen()) {
        _replacerClient->close();
    }

    // Closing fira socket
    if(_firaClient->isOpen()) {
        _firaClient->close();
    }

    // Deleting replacer client
    delete _replacerClient;

    // Deleting fira client
    delete _firaClient;
}

void Replacer::initialization() {
    // Set initial goalie (id 0)
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        takeGoalie(VSSRef::Color(i), 0);
    }

    // Set initial frames as empty
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        setTeamFrame(VSSRef::Color(i), VSSRef::Frame());
    }

    // Set initial placed status as false
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        _placementStatus.insert(VSSRef::Color(i), false);
    }

    // Setup initial vars
    _foulProcessed = false;
    _isGoaliePlacedAtTop = false;

    // Connect to network
    bindAndConnect();

    // Debug network info
    std::cout << Text::blue("[REPLACER] ", true) + Text::bold("Module started at address '" + _replacerAddress.toStdString() + "' and port '" + std::to_string(_replacerPort) + "'.") + '\n';
}

void Replacer::loop() {
    while(_replacerClient->hasPendingDatagrams()) {
        QNetworkDatagram datagram;
        VSSRef::team_to_ref::VSSRef_Placement frame;

        // Reading datagram and checking if it is valid
        datagram = _replacerClient->receiveDatagram();
        if(!datagram.isValid()) {
            continue;
        }

        // Parsing datagram and checking if it worked properly
        if(frame.ParseFromArray(datagram.data().data(), datagram.data().size()) == false) {
            std::cout << Text::blue("[REPLACER] ", true) << Text::red("Frame packet parsing error.", true) + '\n';
            continue;
        }

        // Check if world has world (frame data)
        if(frame.has_world()) {
            // Take frame
            VSSRef::Frame frameData = frame.world();

            // Set frame
            setTeamFrame(frameData.teamcolor(), frameData);

            // Set that team placed
            _placementStatus.insert(frameData.teamcolor(), true);
        }

        // Check if both placed
        if(_placementStatus.value(VSSRef::Color::BLUE) == true && _placementStatus.value(VSSRef::Color::YELLOW) == true) {
            // If both placed send signal
            emit teamsPlaced();
        }

        // Check if placed ball
        if(!_placedLastPosition) {
            placeBall(_lastBallPosition);
            _placedLastPosition = true;
        }
    }

    if(_foulProcessed) {
        // Reset control vars
        for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
            setTeamFrame(VSSRef::Color(i), VSSRef::Frame());
            _placementStatus.insert(VSSRef::Color(i), false);
        }

        // Update last ball data
        _lastBallPosition = _vision->getBallPosition();
        _lastBallVelocity = _vision->getBallVelocity();

        _placedLastPosition = false;
    }
}

void Replacer::finalization() {
    disconnectClient();
    std::cout << Text::blue("[REPLACER] ", true) + Text::bold("Module finished.") + '\n';
}

void Replacer::parsePlacements() {
    // Take fouls listed in JSON
    QList<QString> fouls = documentMap().keys();

    // For each foul
    for(int i = 0; i < fouls.size(); i++) {
        // Take categories listed in each foul
        QMap<QString, QVariant> foulsMap = documentMap()[fouls.at(i)].toMap();
        QList<QString> categories = foulsMap.keys();

        // Register foul at placement map
        if(!placements.contains(fouls.at(i))) {
            placements.insert(fouls.at(i), new QMap<QString, QMap<QString, QVector<PlaceData>>*>());
        }
        QMap<QString, QMap<QString, QVector<PlaceData>>*>* foulPlacementMap = placements.value(fouls.at(i));

        // For each category
        for(int j = 0; j < categories.size(); j++) {
            // Take roles
            QMap<QString, QVariant> categoriesMap = foulsMap[categories.at(j)].toMap();
            QList<QString> roles = categoriesMap.keys();

            // Register category at placement map
            if(!foulPlacementMap->contains(categories.at(j))) {
                foulPlacementMap->insert(categories.at(j), new QMap<QString, QVector<PlaceData>>());
            }
            QMap<QString, QVector<PlaceData>>* categoriesPlacementMap = foulPlacementMap->value(categories.at(j));

            // For each role
            for(int k = 0; k < roles.size(); k++) {
                // Take placement types data
                QMap<QString, QVariant> rolesMap = categoriesMap[roles.at(k)].toMap();
                QList<QString> placementTypes = rolesMap.keys();
                QVector<PlaceData> placementsData;

                // For each placement type
                for(int l = 0; l < placementTypes.size(); l++) {
                    // Take placement data
                    QMap<QString, QVariant> placementInfo = rolesMap[placementTypes.at(l)].toMap();

                    // Get PlaceData
                    PlaceData placeData(Position(true, placementInfo["x"].toFloat(), placementInfo["y"].toFloat()), Angle(true, placementInfo["ori"].toFloat()));

                    // Insert into vector
                    placementsData.push_back(placeData);
                }

                // Insert placement vector in map
                categoriesPlacementMap->insert(roles.at(k), placementsData);
            }
        }
    }
}

QMap<QString, QVector<PlaceData>> Replacer::getPlacementsByFoul(QString foul) {
    // Check if placement map contains the foul
    if(!placements.contains(foul)) {
        std::cout << Text::red("[ERROR] ", true) + Text::bold("The foul '" + foul.toStdString() + "' cannot be found in placement list. Please check the JSON file to avoid mistakes.") + '\n';
        return QMap<QString, QVector<PlaceData>>();
    }

    // Take the map of placements by category
    QMap<QString, QMap<QString, QVector<PlaceData>>*>* placementsByCategory = placements.value(foul);

    // Check if the desired category is contained by the map
    QString category = (getConstants()->is5v5()) ? "5v5" : "3v3";
    if(!placementsByCategory->contains(category)) {
        std::cout << Text::red("[ERROR] ", true) + Text::bold("The category '" + category.toStdString() + "' cannot be found in foul '" + foul.toStdString() + "'placement categories list. Please check the JSON file to avoid mistakes.") + '\n';
        return QMap<QString, QVector<PlaceData>>();
    }

    // Take the map of placedata by roles
    QMap<QString, QVector<PlaceData>>* placementsByRole = placementsByCategory->value(category);

    // Return it
    return (*placementsByRole);
}

VSSRef::Frame Replacer::getPlacementFrameByFoul(QString foul, VSSRef::Quadrant foulQuadrant, VSSRef::Color teamColor) {
    // Take placementData
    QMap<QString, QVector<PlaceData>> placementData = getPlacementsByFoul(foul);
    QList<QString> placementRoles = placementData.keys();

    // If empty, probably not found the foul, so return an empty frame
    if(placementRoles.empty()) {
        return VSSRef::Frame();
    }

    // Create frame and set color
    VSSRef::Frame frame;
    frame.set_teamcolor(teamColor);

    // Check if teamColor is playing at left side (so, negative all x-axis position)
    bool teamIsAtLeft = (teamColor == VSSRef::Color::BLUE && getConstants()->blueIsLeftSide()) || (teamColor == VSSRef::Color::YELLOW && !getConstants()->blueIsLeftSide());

    // Check if team will make the kick
    bool teamWillMakeTheKick = (teamColor == getFoulColor()) || ((teamIsAtLeft && (foulQuadrant == VSSRef::Quadrant::QUADRANT_1 || foulQuadrant == VSSRef::Quadrant::QUADRANT_4))
                                                                 || (!teamIsAtLeft && (foulQuadrant == VSSRef::Quadrant::QUADRANT_2 || foulQuadrant == VSSRef::Quadrant::QUADRANT_3)));

    // Set players at frame
    // Get available players for teamColor
    QList<quint8> players = _vision->getAvailablePlayers(teamColor);

    // Get goalie and remove it from list
    quint8 goalieId = getGoalie(teamColor);
    players.removeOne(goalieId);
    placementRoles.removeOne("goalkeeper");

    // Get goalie PlaceData and move it to position
    PlaceData goalKeeperPlaceData = placementData["goalkeeper"][!teamWillMakeTheKick];
    if(teamIsAtLeft) goalKeeperPlaceData.reflect();

    // If is GOAL_KICK, randomly swap the y-axis
    if(foul == "GOAL_KICK" && getFoulColor() == teamColor) {
        // Random to choose GK position
        auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        std::mt19937 mt_rand(seed);
        _isGoaliePlacedAtTop = mt_rand() % 2;

        // Position and ori
        Position gkPosition = goalKeeperPlaceData.getPosition();
        gkPosition = Position(true, gkPosition.x(), (_isGoaliePlacedAtTop) ? gkPosition.y() : -gkPosition.y());

        // Future ball position
        Position futureBall = getBallPlaceByFoul(VSSRef::Foul::GOAL_KICK, teamColor, VSSRef::Quadrant::NO_QUADRANT);

        // Move
        movePlayerToPosition(frame.add_robots(), goalieId, gkPosition, Angle(true, Utils::getAngle(futureBall, gkPosition)));
    }
    else {
        movePlayerToPosition(frame.add_robots(), goalieId, goalKeeperPlaceData.getPosition(), goalKeeperPlaceData.getOrientation());
    }

    // Check if its FREE_BALL
    if(foul == "FREE_BALL") {
        // Take attacker
        quint8 attackerId = players.takeFirst();

        // Get free ball position
        Position fbPosition = getBallPlaceByFoul(VSSRef::Foul::FREE_BALL, VSSRef::Color::NONE, getFoulQuadrant());

        // Get x-axis diff
        float xAxisDiff = (getConstants()->is5v5()) ? 0.25 : 0.2;

        // Place attacker in correct position
        movePlayerToPosition(frame.add_robots(), attackerId, Position(true, fbPosition.x() + (teamIsAtLeft ? -xAxisDiff : xAxisDiff), fbPosition.y()), Angle(true, 0.0f));
    }

    // For each other player, set their position
    /// TODO: check how to set properly the player roles... but now it will set randomly
    for(int i = 0; i < players.size(); i++) {
        PlaceData playerPlacementData = placementData[placementRoles.at(i)][!teamWillMakeTheKick];
        Position placementPosition = playerPlacementData.getPosition();

        if(teamIsAtLeft) playerPlacementData.reflect();
        placementPosition = playerPlacementData.getPosition();

        // If is free ball foul, check if is needed to swap y-axis
        if(foul == "FREE_BALL") {
            if(foulQuadrant == VSSRef::Quadrant::QUADRANT_3 || foulQuadrant == VSSRef::Quadrant::QUADRANT_4) {
                placementPosition = Position(true, placementPosition.x(), -placementPosition.y());
            }
        }

        movePlayerToPosition(frame.add_robots(), players.at(i), placementPosition, playerPlacementData.getOrientation());
    }

    // Return frame
    return frame;
}

bool Replacer::checkIfCollides(VSSRef::Frame blueFrame, VSSRef::Frame yellowFrame) {
    // Check blue with yellow
    for(int i = 0; i < blueFrame.robots_size(); i++) {
        for(int j = 0; j < yellowFrame.robots_size(); j++) {
            Position blueRobotPosition = Position(true, blueFrame.robots(i).x(), blueFrame.robots(i).y());
            Position yellowRobotPosition = Position(true, yellowFrame.robots(j).x(), yellowFrame.robots(j).y());


            if(Utils::distance(blueRobotPosition, yellowRobotPosition) <= 1.1 * getConstants()->robotLength()) {
                std::cout << Text::yellow("[REPLACER] ", true) + Text::bold("Detected collision with blue and yellow frames:") + '\n';
                std::cout << "Collision with blue robot " + std::to_string(blueFrame.robots(i).robot_id()) + " at position x: " + std::to_string(blueRobotPosition.x()) + " y: " + std::to_string(blueRobotPosition.y()) + '\n';
                std::cout << "Collision with yellow robot " + std::to_string(yellowFrame.robots(j).robot_id()) + " at position x: " + std::to_string(yellowRobotPosition.x()) + " y: " + std::to_string(yellowRobotPosition.y()) + '\n';

                return true;
            }
        }
    }

    // Check yellow with yellow
    for(int i = 0; i < yellowFrame.robots_size(); i++) {
        for(int j = 0; j < yellowFrame.robots_size(); j++) {
            if(i == j) continue;

            Position yellowRobotPosition = Position(true, yellowFrame.robots(i).x(), yellowFrame.robots(i).y());
            Position yellowRobotPosition2 = Position(true, yellowFrame.robots(j).x(), yellowFrame.robots(j).y());


            if(Utils::distance(yellowRobotPosition, yellowRobotPosition2) <= 1.1 * getConstants()->robotLength()) {
                std::cout << Text::yellow("[REPLACER] ", true) + Text::bold("Detected collision at yellow frame:") + '\n';
                std::cout << "Collision with yellow robot " + std::to_string(yellowFrame.robots(i).robot_id()) + " at position x: " + std::to_string(yellowRobotPosition.x()) + " y: " + std::to_string(yellowRobotPosition.y()) + '\n';
                std::cout << "Collision with yellow robot " + std::to_string(yellowFrame.robots(j).robot_id()) + " at position x: " + std::to_string(yellowRobotPosition2.x()) + " y: " + std::to_string(yellowRobotPosition2.y()) + '\n';

                return true;
            }
        }
    }

    // Check blue with blue
    for(int i = 0; i < blueFrame.robots_size(); i++) {
        for(int j = 0; j < blueFrame.robots_size(); j++) {
            if(i == j) continue;

            Position blueRobotPosition = Position(true, blueFrame.robots(i).x(), blueFrame.robots(i).y());
            Position blueRobotPosition2 = Position(true, blueFrame.robots(j).x(), blueFrame.robots(j).y());


            if(Utils::distance(blueRobotPosition, blueRobotPosition2) <= 1.1 * getConstants()->robotLength()) {
                std::cout << Text::yellow("[REPLACER] ", true) + Text::bold("Detected collision at blue frame:") + '\n';
                std::cout << "Collision with blue robot " + std::to_string(blueFrame.robots(i).robot_id()) + " at position x: " + std::to_string(blueRobotPosition.x()) + " y: " + std::to_string(blueRobotPosition.y()) + '\n';
                std::cout << "Collision with blue robot " + std::to_string(blueFrame.robots(j).robot_id()) + " at position x: " + std::to_string(blueRobotPosition2.x()) + " y: " + std::to_string(blueRobotPosition2.y()) + '\n';

                return true;
            }
        }
    }

    return false;
}

quint8 Replacer::getGoalie(VSSRef::Color color) {
    _goalieMutex.lock();
    quint8 goalieId = _goalies.value(color);
    _goalieMutex.unlock();

    return goalieId;
}

VSSRef::Foul Replacer::getFoul() {
    _foulMutex.lock();
    VSSRef::Foul foul = _foul;
    _foulMutex.unlock();

    return foul;
}

VSSRef::Color Replacer::getFoulColor() {
    _foulMutex.lock();
    VSSRef::Color foulColor = _foulColor;
    _foulMutex.unlock();

    return foulColor;
}

VSSRef::Quadrant Replacer::getFoulQuadrant() {
    _foulMutex.lock();
    VSSRef::Quadrant foulQuadrant = _foulQuadrant;
    _foulMutex.unlock();

    return foulQuadrant;
}

void Replacer::takeGoalie(VSSRef::Color color, quint8 playerId) {
    _goalieMutex.lock();
    _goalies.insert(color, playerId);
    _goalieMutex.unlock();
}

void Replacer::takeFoul(VSSRef::Foul foul, VSSRef::Color foulColor, VSSRef::Quadrant foulQuadrant) {
    if(foul == VSSRef::Foul::GAME_ON || foul == VSSRef::Foul::STOP) return ;

    _foulMutex.lock();
    _foul = foul;
    _foulColor = foulColor;
    _foulQuadrant = foulQuadrant;
    _foulProcessed = false;
    _foulMutex.unlock();
}

void Replacer::takeTeleport(bool teleport) {
    _teleport = teleport;
    debugTeleport(teleport);
}

Position Replacer::getBallPlaceByFoul(VSSRef::Foul foul, VSSRef::Color color, VSSRef::Quadrant quadrant){
    float goalKickX = (getField()->fieldLength() / 1000.0)/2.0 - 0.15;
    float markX = (getField()->fieldLength() / 1000.0)/2.0 - getField()->fieldFBMarkX()/1000.0;
    float markY = (getField()->fieldWidth() / 1000.0)/2.0 - getField()->fieldFBMarkY()/1000.0;

    switch(foul){
        case VSSRef::Foul::KICKOFF:{
            return Position(true, 0.0, 0.0);
        }
        break;
        case VSSRef::Foul::FREE_BALL:{
            if(quadrant == VSSRef::Quadrant::QUADRANT_1){
                return Position(true, markX, markY);
            }
            else if(quadrant == VSSRef::Quadrant::QUADRANT_2){
                return Position(true, -markX, markY);
            }
            else if(quadrant == VSSRef::Quadrant::QUADRANT_3){
                return Position(true, -markX, -markY);
            }
            else if(quadrant == VSSRef::Quadrant::QUADRANT_4){
                return Position(true, markX, -markY);
            }
        }
        break;
        case VSSRef::Foul::GOAL_KICK:{
        float stretch = getField()->defenseStretch()/2000.0 + getConstants()->ballRadius();
            if(color == VSSRef::Color::BLUE){
                if(getConstants()->blueIsLeftSide()) return Position(true, -goalKickX, ((_isGoaliePlacedAtTop) ? (stretch - getConstants()->ballRadius()) : (-stretch + getConstants()->ballRadius())));
                else return Position(true, goalKickX, ((_isGoaliePlacedAtTop) ? (stretch - getConstants()->ballRadius()) : (-stretch + getConstants()->ballRadius())));
            }
            else if(color == VSSRef::Color::YELLOW){
                if(getConstants()->blueIsLeftSide()) return Position(true, goalKickX, ((_isGoaliePlacedAtTop) ? (stretch - getConstants()->ballRadius()) : (-stretch + getConstants()->ballRadius())));
                else return Position(true, -goalKickX, ((_isGoaliePlacedAtTop) ? (stretch - getConstants()->ballRadius()) : (-stretch + getConstants()->ballRadius())));
            }
        }
        break;
        case VSSRef::Foul::PENALTY_KICK:{
            float penaltyMarkX = getConstants()->is5v5() ? (getField()->fieldLength()/2000.0 - 0.375) : markX;
            if(color == VSSRef::Color::BLUE){
                if(getConstants()->blueIsLeftSide()) return Position(true, penaltyMarkX, 0);
                else return Position(true, -markX, 0.0);
            }
            else if(color == VSSRef::Color::YELLOW){
                if(getConstants()->blueIsLeftSide()) return Position(true, -penaltyMarkX, 0);
                else return Position(true, markX, 0.0);
            }
        }
        break;
        case VSSRef::Foul::FREE_KICK:{
            if(color == VSSRef::Color::BLUE){
                if(getConstants()->blueIsLeftSide()) return Position(true, markX, 0);
                else return Position(true, -markX, 0.0);
            }
            else if(color == VSSRef::Color::YELLOW){
                if(getConstants()->blueIsLeftSide()) return Position(true, -markX, 0);
                else return Position(true, markX, 0.0);
            }
        }
        break;
        default:{
            return Position(true, 0.0, 0.0);
        }
    }

    return Position(true, 0.0, 0.0);
}


VSSRef::Frame Replacer::getOutsideFieldPlacement(VSSRef::Color teamColor){
    VSSRef::Frame frame;
    frame.set_teamcolor(teamColor);

    // Set players at frame
    // Get available players for teamColor
    QList<quint8> players = _vision->getAvailablePlayers(teamColor);

    // Check if teamColor is playing at left side (so, negative all x-axis position)
    bool teamIsAtLeft = (teamColor == VSSRef::Color::BLUE && getConstants()->blueIsLeftSide()) || (teamColor == VSSRef::Color::YELLOW && !getConstants()->blueIsLeftSide());

    // Get field data
    float sideLength = (getField()->fieldLength()/1000.0)/2.0 - (players.size() * 2 * getConstants()->robotLength());
    float fieldWidth = (getField()->fieldWidth()/1000.0)/2.0;

    // Place each robot outside field
    for(int i = 0; i < players.size(); i++) {
        PlaceData toPlace(Position(true, ((i+1) * 2*getConstants()->robotLength()) + (i * sideLength/players.size()), 1.1*fieldWidth), Angle(true, Angle::pi / 2.0));
        if(teamIsAtLeft) toPlace.reflect();
        movePlayerToPosition(frame.add_robots(), players.at(i), toPlace.getPosition(), toPlace.getOrientation());
    }

    // Return frame
    return frame;
}

VSSRef::Frame Replacer::getPenaltyShootoutPlacement(VSSRef::Color teamColor, bool placeAttacker){
    // Take outside field placement
    VSSRef::Frame frameOutside = getOutsideFieldPlacement(teamColor);

    // Create new frame
    VSSRef::Frame frame;
    frame.set_teamcolor(teamColor);

    // Get available players for teamColor
    QList<quint8> players = _vision->getAvailablePlayers(teamColor);

    quint8 chosenId = 255;
    VSSRef::Robot chosenRobot;
    // If is to place attacker
    if(placeAttacker) {
        // Start distance var
        float bestDistance = 999.0f;

        // Take ball position
        Position ballPosition = getBallPlaceByFoul(VSSRef::Foul::PENALTY_KICK, teamColor, VSSRef::Quadrant::NO_QUADRANT);

        // Take team placement frame (getPenaltyShootoutPlacement will only be call after placeTeams())
        VSSRef::Frame teamFrame = getTeamFrame(teamColor);

        // Take the closest player to ball that isn't the goalkeeper
        for(int i = 0; i < players.size(); i++) {
            // Take player position at teamFrame
            float playerDistToBall = 999.0f;
            VSSRef::Robot robotParams;
            for(int j = 0; j < teamFrame.robots_size(); j++) {
                VSSRef::Robot robot = teamFrame.robots(j);
                if(robot.robot_id() == players.at(i)) {
                    Position robotPosition = Position(true, robot.x(), robot.y());
                    playerDistToBall = Utils::distance(robotPosition, ballPosition);
                    robotParams = robot;
                    break;
                }
            }

            // Check if player distance to ball is lower than the best distance
            if(playerDistToBall <= bestDistance) {
                bestDistance = playerDistToBall;
                chosenId = players.at(i);
                chosenRobot = robotParams;
            }
        }
    }
    // Else, goalie as penalty goalkeeper
    else {
        chosenId = getGoalie(teamColor);

        VSSRef::Frame teamFrame = getTeamFrame(teamColor);
        for(int i = 0; i < teamFrame.robots_size(); i++) {
            if(teamFrame.robots(i).robot_id() == chosenId) {
                chosenRobot = teamFrame.robots(i);
                break;
            }
        }
    }

    for(int i = 0; i < frameOutside.robots_size(); i++) {
        VSSRef::Robot robotAt = frameOutside.robots(i);
        if(robotAt.robot_id() != chosenId) {
            movePlayerToPosition(frame.add_robots(), robotAt.robot_id(), Position(true, robotAt.x(), robotAt.y()), Angle(true, Angle::toRadians(robotAt.orientation())));
        }
        else {
            movePlayerToPosition(frame.add_robots(), chosenRobot.robot_id(), Position(true, chosenRobot.x(), chosenRobot.y()), Angle(true, Angle::toRadians(chosenRobot.orientation())));
        }
    }

    return frame;
}

void Replacer::placeFrame(VSSRef::Frame frame) {
    // Create aux vars
    fira_message::sim_to_ref::Packet packet;
    fira_message::sim_to_ref::Replacement *command = new fira_message::sim_to_ref::Replacement();
    std::string msg;

    // Create robot commands
    for(int i = 0; i < frame.robots_size(); i++) {
        fira_message::sim_to_ref::RobotReplacement *robotPlacement = command->add_robots();
        fira_message::Robot *robotPosition = new fira_message::Robot();
        VSSRef::Robot frameRobot = frame.robots(i);

        // Set robot position / data
        robotPosition->set_robot_id(frameRobot.robot_id());
        robotPosition->set_orientation(frameRobot.orientation());
        robotPosition->set_x(frameRobot.x());
        robotPosition->set_y(frameRobot.y());
        robotPosition->set_vx(0.0);
        robotPosition->set_vy(0.0);

        // Check if is foul goalie and if it is placed at top or not
        if(frame.teamcolor() == getFoulColor()) {
            if(Utils::isInsideGoalArea(frame.teamcolor(), Position(true, frameRobot.x(), frameRobot.y()))){
                if(frameRobot.y() >= 0) {
                    _isGoaliePlacedAtTop = true;
                }
                else {
                    _isGoaliePlacedAtTop = false;
                }
            }
        }

        // Set placement data
        robotPlacement->set_turnon(true);
        robotPlacement->set_yellowteam((frame.teamcolor() == VSSRef::Color::YELLOW) ? true : false);
        robotPlacement->set_allocated_position(robotPosition);
    }

    // Set replacement to packet
    packet.set_allocated_replace(command);

    // Send to network
    packet.SerializeToString(&msg);

    if(_firaClient->write(msg.c_str(), msg.length()) == -1){
       std::cout << Text::blue("[REPLACER] ", true) + Text::red("FiraClient failed to write to socket.", true) + '\n';
    }
}

void Replacer::placeBall(Position ballPos, Velocity ballVelocity) {
    // Create aux vars
    fira_message::sim_to_ref::Packet packet;
    fira_message::sim_to_ref::Replacement *command = new fira_message::sim_to_ref::Replacement();
    std::string msg;

    // Create ball place command
    fira_message::sim_to_ref::BallReplacement *ballPlacement = new fira_message::sim_to_ref::BallReplacement();

    ballPlacement->set_x(ballPos.x());
    ballPlacement->set_y(ballPos.y());
    ballPlacement->set_vx(ballVelocity.vx());
    ballPlacement->set_vy(ballVelocity.vy());

    // Setting replacement into command
    command->set_allocated_ball(ballPlacement);

    // Setting command into packet
    packet.set_allocated_replace(command);

    // Send to network
    packet.SerializeToString(&msg);

    if(_firaClient->write(msg.c_str(), msg.length()) == -1){
       std::cout << Text::blue("[REPLACER] ", true) + Text::red("FiraClient failed to write to socket.", true) + '\n';
    }
}

void Replacer::placeTeams(bool forceDefault, bool isToPlaceOutside) {
    VSSRef::Foul lastFoul = getFoul();
    VSSRef::Color lastFoulColor = getFoulColor();
    QHash<VSSRef::Color, VSSRef::Frame> frames;

    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        // if team placed
        if(_placementStatus.value(VSSRef::Color(i)) && !forceDefault) {
            // Take received frame
            frames.insert(VSSRef::Color(i), getTeamFrame(VSSRef::Color(i)));
        }
        // if team not placed, take default positions
        else {
            // Take default frame
            VSSRef::Frame defaultFrame = getPlacementFrameByFoul(VSSRef::Foul_Name(lastFoul).c_str(), getFoulQuadrant(), VSSRef::Color(i));

            // Save frame
            setTeamFrame(VSSRef::Color(i), defaultFrame);

            // Take frame
            frames.insert(VSSRef::Color(i), defaultFrame);
        }
    }

    if((lastFoul == VSSRef::Foul::KICKOFF || lastFoul == VSSRef::Foul::PENALTY_KICK) && isToPlaceOutside) {
        // Cast place outside
        VSSRef::Color oppositeColor = (lastFoulColor == VSSRef::BLUE) ? VSSRef::YELLOW : VSSRef::BLUE;
        placeOutside(lastFoul, oppositeColor);
    }

    // Check if frames collides
    if(checkIfCollides(getTeamFrame(VSSRef::Color::BLUE), getTeamFrame(VSSRef::Color::YELLOW))) {
        emit teamsCollided(_foul, _foulColor, _foulQuadrant, isToPlaceOutside);
    }
    else {
        // If frames not collides and the teleport is enabled, just place it
        if(_teleport){
            placeFrame(getTeamFrame(VSSRef::Color::BLUE));
            placeFrame(getTeamFrame(VSSRef::Color::YELLOW));
        }

        // Mark foul as processed
        _foulMutex.lock();
        _foulProcessed = true;
        _foulMutex.unlock();
    }

    // Place ball
    Position foulBallPosition = getBallPlaceByFoul(_foul, _foulColor, _foulQuadrant);
    placeBall(foulBallPosition);
}

void Replacer::placeOutside(VSSRef::Foul foul, VSSRef::Color oppositeTeam) {
    if(foul == VSSRef::Foul::KICKOFF) {
        VSSRef::Frame removedFrame;

        // Filling frames
        removedFrame = getOutsideFieldPlacement(oppositeTeam);

        // Set team frame
        setTeamFrame(oppositeTeam, removedFrame);
    }
    else if(foul == VSSRef::Foul::PENALTY_KICK) {
        VSSRef::Frame removedFrameKicker;
        VSSRef::Frame removedFrameGoalie;

        // Filling frames
        removedFrameKicker = getPenaltyShootoutPlacement((oppositeTeam == VSSRef::Color::BLUE) ? VSSRef::Color::YELLOW : VSSRef::Color::BLUE, true);
        removedFrameGoalie = getPenaltyShootoutPlacement(oppositeTeam, false);

        // Set teams frames
        setTeamFrame((oppositeTeam == VSSRef::Color::BLUE) ? VSSRef::Color::YELLOW : VSSRef::Color::BLUE, removedFrameKicker);
        setTeamFrame(oppositeTeam, removedFrameGoalie);
    }
}

VSSRef::Frame Replacer::getTeamFrame(VSSRef::Color teamColor) {
    _frameMutex.lock();
    VSSRef::Frame teamFrame = _placement.value(teamColor);
    _frameMutex.unlock();

    return teamFrame;
}

void Replacer::setTeamFrame(VSSRef::Color teamColor, VSSRef::Frame frame) {
    _frameMutex.lock();
    _placement.insert(teamColor, frame);
    _frameMutex.unlock();
}

void Replacer::clearLastData() {
    // Take team frames
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        if(_lastFrame.contains(VSSRef::Color(i))) {
            // Take hash
            QHash<quint8, fira_message::Robot*> *hash = _lastFrame.value(VSSRef::Color(i));

            // Take keys
            QList<quint8> keys = hash->keys();
            for(int j = 0; j < keys.size(); j++) {
                // Take robot
                fira_message::Robot *robot = hash->value(keys.at(j));

                // Delete
                delete robot;
            }

            // Clear hash
            hash->clear();
        }
    }

    _lastFrame.clear();
}

void Replacer::saveFrameAndBall() {
    _lastDataMutex.lock();

    // Update last ball data
    _lastBallPosition = _vision->getBallPosition();
    _lastBallVelocity = _vision->getBallVelocity();

    // Take team frames
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        // Creating hash
        if(!_lastFrame.contains(VSSRef::Color(i))) {
            _lastFrame.insert(VSSRef::Color(i), new QHash<quint8, fira_message::Robot*>());
        }

        // Taking hash
        QHash<quint8, fira_message::Robot*> *hash = _lastFrame.value(VSSRef::Color(i));

        // Taking available robots
        QList<quint8> avPlayers = _vision->getAvailablePlayers(VSSRef::Color(i));
        for(int j = 0; j < avPlayers.size(); j++) {
            // Creating RobotReplacement
            if(!hash->contains(avPlayers.at(j))) {
                hash->insert(avPlayers.at(j), new fira_message::Robot());
            }

            // Taking data from vision
            Position robotPosition = _vision->getPlayerPosition(VSSRef::Color(i), avPlayers.at(j));
            Angle robotOrientation = _vision->getPlayerOrientation(VSSRef::Color(i), avPlayers.at(j));
            Velocity robotVelocity = _vision->getPlayerVelocity(VSSRef::Color(i), avPlayers.at(j));

            // Filling robot data
            fira_message::Robot *robotData = hash->value(avPlayers.at(j));
            robotData->set_robot_id(avPlayers.at(j));
            robotData->set_x(robotPosition.x());
            robotData->set_y(robotPosition.y());
            robotData->set_vx(robotVelocity.vx());
            robotData->set_vy(robotVelocity.vy());
            robotData->set_orientation(robotOrientation.value() * (180.0 / M_PI));
        }
    }

    _lastDataMutex.unlock();
}

void Replacer::placeLastFrameAndBall() {
    _lastDataMutex.lock();

    // Place ball
    placeBall(_lastBallPosition, _lastBallVelocity);

    // Create robot commands
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        // Taking hash and av players
        QHash<quint8, fira_message::Robot*> *lastTeamFrame = _lastFrame.value(VSSRef::Color(i));
        QList<quint8> avPlayers = _vision->getAvailablePlayers(VSSRef::Color(i));

        // Create aux vars
        fira_message::sim_to_ref::Packet packet;
        fira_message::sim_to_ref::Replacement *command = new fira_message::sim_to_ref::Replacement();
        std::string msg;

        for(int j = 0; j < avPlayers.size(); j++) {
            // Avoid take data if player is not contained in last frame
            if(!lastTeamFrame->contains(avPlayers.at(j))) {
                continue;
            }

            // Creating robot in command
            fira_message::sim_to_ref::RobotReplacement *robotPlacement = command->add_robots();

            // Taking data from hash
            fira_message::Robot *robot = new fira_message::Robot();
            robot->CopyFrom(*lastTeamFrame->value(avPlayers.at(j)));

            // Setting data to robotPlacement
            robotPlacement->set_turnon(true);
            robotPlacement->set_yellowteam((i == VSSRef::Color::BLUE) ? false : true);
            robotPlacement->set_allocated_position(robot);
        }

        // Set replacement to packet
        packet.set_allocated_replace(command);

        // Send to network
        packet.SerializeToString(&msg);

        if(_firaClient->write(msg.c_str(), msg.length()) == -1){
           std::cout << Text::blue("[REPLACER] ", true) + Text::red("FiraClient failed to write to socket.", true) + '\n';
        }
    }

    clearLastData();

    _lastDataMutex.unlock();
}

Field* Replacer::getField() {
    if(_field == nullptr) {
        std::cout << Text::red("[ERROR] ", true) << Text::bold("Field with nullptr value at Replacer") + '\n';
    }
    else {
        return _field;
    }

    return nullptr;
}


Constants* Replacer::getConstants() {
    if(_constants == nullptr) {
        std::cout << Text::red("[ERROR] ", true) << Text::bold("Constants with nullptr value at Replacer") + '\n';
    }
    else {
        return _constants;
    }

    return nullptr;
}
