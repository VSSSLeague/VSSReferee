#include "replacer.h"

#include <random>
#include <chrono>

#include <src/utils/types/field/field_default_3v3.h>
#include <src/utils/utils.h>

Replacer::Replacer(Vision *vision, Constants *constants) : Entity(ENT_REPLACER){
    // Take pointers
    _vision = vision;
    _constants = constants;

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
        std::cout << Text::blue("[VISION] " , true) << Text::red("Error while binding socket.", true) + '\n';
        return ;
    }

    // Joining multicast group
    if(_replacerClient->joinMulticastGroup(QHostAddress(_replacerAddress)) == false) {
        std::cout << Text::blue("[VISION] ", true) << Text::red("Error while joining multicast.", true) + '\n';
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
        _placement.insert(VSSRef::Color(i), VSSRef::Frame());
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
    if(!_foulProcessed) {
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
                _placement.insert(frameData.teamcolor(), frameData);

                // Set that team placed
                _placementStatus.insert(frameData.teamcolor(), true);
            }

            // Check if both placed
            if(_placementStatus.value(VSSRef::Color::BLUE) == true && _placementStatus.value(VSSRef::Color::YELLOW) == true) {
                // If both placed send signal
                emit teamsPlaced();
            }
        }

        // Force robots to stop
        stopRobots();
    }
    else {
        // Reset control vars
        for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
            _placementStatus.insert(VSSRef::Color(i), false);
        }
    }
}

void Replacer::finalization() {
    disconnectClient();
    std::cout << Text::blue("[REPLACER] ", true) + Text::bold("Module finished.") + '\n';
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
    _foulMutex.lock();
    _foul = foul;
    _foulColor = foulColor;
    _foulQuadrant = foulQuadrant;
    _foulProcessed = false;
    _foulMutex.unlock();
}

Position Replacer::getBallPlaceByFoul(VSSRef::Foul foul, VSSRef::Color color, VSSRef::Quadrant quadrant){
    float goalKickX = (Field_Default_3v3::kFieldLength / 1000.0)/2.0 - 0.15;
    float markX = (Field_Default_3v3::kFieldLength / 1000.0)/2.0 - 0.375;
    float markY = (Field_Default_3v3::kFieldWidth / 1000.0)/2.0 - 0.25;

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
            if(color == VSSRef::Color::BLUE){
                if(getConstants()->blueIsLeftSide()) return Position(true, -goalKickX, (_isGoaliePlacedAtTop) ? (0.375 - getConstants()->ballRadius()) : (-0.375 + getConstants()->ballRadius()));
                else return Position(true, goalKickX, (_isGoaliePlacedAtTop) ? (0.375 - getConstants()->ballRadius()) : (-0.375 + getConstants()->ballRadius()));
            }
            else if(color == VSSRef::Color::YELLOW){
                if(getConstants()->blueIsLeftSide()) return Position(true, goalKickX, (_isGoaliePlacedAtTop) ? (0.375 - getConstants()->ballRadius()) : (-0.375 + getConstants()->ballRadius()));
                else return Position(true, -goalKickX, (_isGoaliePlacedAtTop) ? (0.375 - getConstants()->ballRadius()) : (-0.375 + getConstants()->ballRadius()));
            }
        }
        break;
        case VSSRef::Foul::PENALTY_KICK:{
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

VSSRef::Frame Replacer::getPenaltyPlacement(VSSRef::Color color){
    VSSRef::Frame frame;
    frame.set_teamcolor(color);

    // swap side check
    bool teamIsAtLeft = (color == VSSRef::Color::BLUE && getConstants()->blueIsLeftSide()) || (color == VSSRef::Color::YELLOW && !getConstants()->blueIsLeftSide());

    float factor = 1.0;
    if(teamIsAtLeft)
        factor = -1.0;

    // FB mark
    float markX = (Field_Default_3v3::kFieldLength / 1000.0)/2.0 - 0.375;
    float markY = (Field_Default_3v3::kFieldWidth / 1000.0)/2.0 - 0.25;

    QList<quint8> players = _vision->getAvailablePlayers(color);
    for(int i = 0; i < players.size(); i++) {
        if(players.at(i) == getGoalie(color)) {
            players.removeAt(i);
        }
    }

    // _color is the team that will make the kick
    if(color == getFoulColor()){
        // Insert GK
        if(players.size() == 0) return frame;
        VSSRef::Robot *gk = frame.add_robots();
        gk->set_robot_id(getGoalie(color));
        gk->set_orientation(0.0);
        gk->set_x(factor * ((Field_Default_3v3::kFieldLength / 2000.0) - getConstants()->robotLength()));
        gk->set_y(0.0);

        // Attacker
        if(players.size() == 0) return frame;
        VSSRef::Robot *striker = frame.add_robots();
        striker->set_robot_id(players.takeFirst());
        striker->set_orientation(0.0);
        striker->set_x((-factor) * (markX - (2.0 * getConstants()->robotLength())));
        striker->set_y(0.0);

        // Support / Second Attacker
        if(players.size() == 0) return frame;
        VSSRef::Robot *support = frame.add_robots();
        support->set_robot_id(players.takeFirst());
        support->set_orientation(0.0);
        support->set_x(factor * (1.5 * getConstants()->robotLength()));
        support->set_y(markY);
    }
    else{
        // Insert GK
        if(players.size() == 0) return frame;
        VSSRef::Robot *gk = frame.add_robots();
        gk->set_robot_id(getGoalie(color));
        gk->set_orientation(0.0);
        gk->set_x(factor * ((Field_Default_3v3::kFieldLength/2000.0) - (getConstants()->robotLength()/2.0)));
        gk->set_y(0.0);

        // Attacker
        if(players.size() == 0) return frame;
        VSSRef::Robot *striker = frame.add_robots();
        striker->set_robot_id(players.takeFirst());
        striker->set_orientation(0.0);
        striker->set_x((-factor) * (1.5 * getConstants()->robotLength()));
        striker->set_y(-markY);

        // Support / Second Attacker
        if(players.size() == 0) return frame;
        VSSRef::Robot *support = frame.add_robots();
        support->set_robot_id(players.takeFirst());
        support->set_orientation(0.0);
        support->set_x((-factor) * (1.5 * getConstants()->robotLength()));
        support->set_y(markY - (2.0 * getConstants()->robotLength()));
    }

    return frame;
}

VSSRef::Frame Replacer::getGoalKickPlacement(VSSRef::Color color){
    VSSRef::Frame frame;
    frame.set_teamcolor(color);

    // swap side check
    bool teamIsAtLeft = (color == VSSRef::Color::BLUE && getConstants()->blueIsLeftSide()) || (color == VSSRef::Color::YELLOW && !getConstants()->blueIsLeftSide());

    float factor = 1.0;
    if(teamIsAtLeft)
        factor = -1.0;

    // FB mark
    float markX = (Field_Default_3v3::kFieldLength / 1000.0)/2.0 - 0.375;
    float markY = (Field_Default_3v3::kFieldWidth / 1000.0)/2.0 - 0.25;

    QList<quint8> players = _vision->getAvailablePlayers(color);
    for(int i = 0; i < players.size(); i++) {
        if(players.at(i) == getGoalie(color)) {
            players.removeAt(i);
        }
    }

    // _color is the team that will make the kick
    if(color == getFoulColor()){
        // Random to choose GK position
        auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        std::mt19937 mt_rand(seed);
        _isGoaliePlacedAtTop = mt_rand() % 2;

        // Insert GK
        if(players.size() == 0) return frame;
        VSSRef::Robot *gk = frame.add_robots();
        gk->set_robot_id(getGoalie(color));
        if(_isGoaliePlacedAtTop){
            gk->set_orientation(factor * -45.0);
            gk->set_x(factor * 0.675);
            gk->set_y(0.270);
        }
        else{
            gk->set_orientation(factor * 45.0);
            gk->set_x(factor * 0.675);
            gk->set_y(-0.270);
        }

        // Attacker
        if(players.size() == 0) return frame;
        VSSRef::Robot *striker = frame.add_robots();
        striker->set_robot_id(players.takeFirst());
        striker->set_orientation(0.0);
        striker->set_x((factor) * (markX + getConstants()->robotLength()));
        striker->set_y(markY - getConstants()->robotLength());

        // Support / Second Attacker
        if(players.size() == 0) return frame;
        VSSRef::Robot *support = frame.add_robots();
        support->set_robot_id(players.takeFirst());
        support->set_orientation(0.0);
        support->set_x((factor) * (markX - getConstants()->robotLength()));
        support->set_y(-markY - getConstants()->robotLength());
    }
    else{
        // Insert GK
        if(players.size() == 0) return frame;
        VSSRef::Robot *gk = frame.add_robots();
        gk->set_robot_id(getGoalie(color));
        gk->set_orientation(0.0);
        gk->set_x(factor * ((Field_Default_3v3::kFieldLength/2000.0) - (getConstants()->robotLength())));
        gk->set_y(0.0);

        // Attacker
        if(players.size() == 0) return frame;
        VSSRef::Robot *striker = frame.add_robots();
        striker->set_robot_id(players.takeFirst());
        striker->set_orientation(0.0);
        striker->set_x((-factor) * (markX - (2.0 * getConstants()->robotLength())));
        striker->set_y(markY - (4.0 * getConstants()->robotLength()));

        // Support / Second Attacker
        if(players.size() == 0) return frame;
        VSSRef::Robot *support = frame.add_robots();
        support->set_robot_id(players.takeFirst());
        support->set_orientation(0.0);
        support->set_x((-factor) * (markX - (3.0 * getConstants()->robotLength())));
        support->set_y(-markY + getConstants()->robotLength());
    }

    return frame;
}

VSSRef::Frame Replacer::getFreeBallPlacement(VSSRef::Color color){
    VSSRef::Frame frame;
    frame.set_teamcolor(color);

    // swap side check
    bool teamIsAtLeft = (color == VSSRef::Color::BLUE && getConstants()->blueIsLeftSide()) || (color == VSSRef::Color::YELLOW && !getConstants()->blueIsLeftSide());

    float factor = 1.0;
    if(teamIsAtLeft)
        factor = -1.0;

    VSSRef::Quadrant foulQuadrant = getFoulQuadrant();

    // FB Mark
    float markX = (Field_Default_3v3::kFieldLength / 1000.0)/2.0 - 0.375;
    float markY = (Field_Default_3v3::kFieldWidth / 1000.0)/2.0 - 0.25;

    if(foulQuadrant == VSSRef::Quadrant::QUADRANT_2 || foulQuadrant == VSSRef::Quadrant::QUADRANT_3)
        markX *= -1;

    if(foulQuadrant == VSSRef::Quadrant::QUADRANT_3 || foulQuadrant == VSSRef::Quadrant::QUADRANT_4)
        markY *= -1;

    QList<quint8> players = _vision->getAvailablePlayers(color);
    for(int i = 0; i < players.size(); i++) {
        if(players.at(i) == getGoalie(color)) {
            players.removeAt(i);
        }
    }

    // First discover if FB will occur at our side
    if(teamIsAtLeft){
        if(players.size() == 0) return frame;
        VSSRef::Robot *gk = frame.add_robots();
        gk->set_robot_id(getGoalie(color));
        gk->set_orientation(0.0);
        gk->set_x(factor * ((Field_Default_3v3::kFieldLength / 2000.0) - getConstants()->robotLength()));
        gk->set_y(0.0);

        // If quadrant 2 or 3, gk will need to pos in an better way
        if(foulQuadrant == VSSRef::Quadrant::QUADRANT_2)
            gk->set_y(getConstants()->robotLength());
        else if(foulQuadrant == VSSRef::Quadrant::QUADRANT_3)
            gk->set_y(-getConstants()->robotLength());

        // Attacker
        if(players.size() == 0) return frame;
        VSSRef::Robot *striker = frame.add_robots();
        striker->set_robot_id(players.takeFirst());
        striker->set_orientation(0.0);
        striker->set_x(markX - 0.2);
        striker->set_y(markY);

        // Support
        if(players.size() == 0) return frame;
        VSSRef::Robot *support = frame.add_robots();
        support->set_robot_id(players.takeFirst());
        support->set_orientation(0.0);

        // Support pos in different ways
        if(foulQuadrant == VSSRef::Quadrant::QUADRANT_1){
            support->set_x(0.1);
            support->set_y(-0.2);
        }
        if(foulQuadrant == VSSRef::Quadrant::QUADRANT_2){
            support->set_x(-0.3);
            support->set_y(-0.1);
        }
        else if(foulQuadrant == VSSRef::Quadrant::QUADRANT_3){
            support->set_x(-0.3);
            support->set_y(0.1);
        }
        else if(foulQuadrant == VSSRef::Quadrant::QUADRANT_4){
            support->set_x(0.1);
            support->set_y(0.2);
        }
    }
    else{
        if(players.size() == 0) return frame;
        VSSRef::Robot *gk = frame.add_robots();
        gk->set_robot_id(getGoalie(color));
        gk->set_orientation(0.0);
        gk->set_x(factor * ((Field_Default_3v3::kFieldLength / 2000.0) - getConstants()->robotLength()));
        gk->set_y(0.0);

        // If quadrant 2 or 3, gk will need to pos in an better way
        if(foulQuadrant == VSSRef::Quadrant::QUADRANT_1)
            gk->set_y(getConstants()->robotLength());
        else if(foulQuadrant == VSSRef::Quadrant::QUADRANT_4)
            gk->set_y(-getConstants()->robotLength());

        // Attacker
        if(players.size() == 0) return frame;
        VSSRef::Robot *striker = frame.add_robots();
        striker->set_robot_id(players.takeFirst());
        striker->set_orientation(0.0);
        striker->set_x(markX + 0.2);
        striker->set_y(markY);

        // Support
        if(players.size() == 0) return frame;
        VSSRef::Robot *support = frame.add_robots();
        support->set_robot_id(players.takeFirst());
        support->set_orientation(0.0);

        // Support pos in different ways
        if(foulQuadrant == VSSRef::Quadrant::QUADRANT_1){
            support->set_x(0.3);
            support->set_y(-0.1);
        }
        if(foulQuadrant == VSSRef::Quadrant::QUADRANT_2){
            support->set_x(-0.1);
            support->set_y(-0.2);
        }
        else if(foulQuadrant == VSSRef::Quadrant::QUADRANT_3){
            support->set_x(-0.1);
            support->set_y(0.2);
        }
        else if(foulQuadrant == VSSRef::Quadrant::QUADRANT_4){
            support->set_x(0.3);
            support->set_y(0.1);
        }
    }

    return frame;
}

VSSRef::Frame Replacer::getKickoffPlacement(VSSRef::Color color){
    VSSRef::Frame frame;
    frame.set_teamcolor(color);

    // swap side check
    bool teamIsAtLeft = (color == VSSRef::Color::BLUE && getConstants()->blueIsLeftSide()) || (color == VSSRef::Color::YELLOW && !getConstants()->blueIsLeftSide());

    float factor = 1.0;
    if(teamIsAtLeft)
        factor = -1.0;

    QList<quint8> players = _vision->getAvailablePlayers(color);
    for(int i = 0; i < players.size(); i++) {
        if(players.at(i) == getGoalie(color)) {
            players.removeAt(i);
        }
    }

    // Goalkeeper
    if(players.size() == 0) return frame;
    VSSRef::Robot *gk = frame.add_robots();
    gk->set_robot_id(getGoalie(color));
    gk->set_orientation(0.0);
    gk->set_x(factor * ((Field_Default_3v3::kFieldLength / 2000.0) - getConstants()->robotLength()));
    gk->set_y(0.0);

    // Attacker
    if(players.size() == 0) return frame;
    VSSRef::Robot *striker = frame.add_robots();
    striker->set_robot_id(players.takeFirst());
    striker->set_orientation(0.0);
    striker->set_x(factor * Field_Default_3v3::kCenterRadius/1000.0);
    striker->set_y(0.0);

    // Support
    if(players.size() == 0) return frame;
    VSSRef::Robot *support = frame.add_robots();
    support->set_robot_id(players.takeFirst());
    support->set_orientation(0.0);
    support->set_x(factor * (Field_Default_3v3::kCenterRadius/1000.0 * 2.0));
    support->set_y(0.0);

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

void Replacer::placeBall() {
    // Create aux vars
    fira_message::sim_to_ref::Packet packet;
    fira_message::sim_to_ref::Replacement *command = new fira_message::sim_to_ref::Replacement();
    std::string msg;

    // Create ball place command
    fira_message::sim_to_ref::BallReplacement *ballPlacement = new fira_message::sim_to_ref::BallReplacement();

    // Set ball information
    Position ballPos = getBallPlaceByFoul(getFoul(), getFoulColor(), getFoulQuadrant());

    ballPlacement->set_x(ballPos.x());
    ballPlacement->set_y(ballPos.y());
    ballPlacement->set_vx(0.0);
    ballPlacement->set_vx(0.0);

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

void Replacer::stopRobots() {
    // Create aux vars
    fira_message::sim_to_ref::Packet command;
    std::string msg;

    // Create robot commands
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        for(int j = 0; j < getConstants()->qtPlayers(); j++) {
            // Set robot data
            fira_message::sim_to_ref::Command *robotCommand = command.mutable_cmd()->add_robot_commands();
            robotCommand->set_yellowteam((i == VSSRef::Color::YELLOW) ? true : false);
            robotCommand->set_id(j);
            robotCommand->set_wheel_left(0.0f);
            robotCommand->set_wheel_right(0.0f);
        }
    }

    command.SerializeToString(&msg);

    if(_firaClient->write(msg.c_str(), msg.length()) == -1){
       std::cout << Text::blue("[REPLACER] ", true) + Text::red("FiraClient failed to write to socket.", true) + '\n';
    }
}

void Replacer::placeTeams() {
    VSSRef::Foul lastFoul = getFoul();

    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        // if team placed
        if(_placementStatus.value(VSSRef::Color(i))) {
            // Take received frame
            VSSRef::Frame teamFrame = _placement.take(VSSRef::Color(i));

            // Send frame to network
            placeFrame(teamFrame);

            // Add new empty frame
            _placement.insert(VSSRef::Color(i), VSSRef::Frame());
        }
        // if team not placed, take default positions
        else {
            // Take default frame
            VSSRef::Frame defaultFrame;

            switch(lastFoul) {
                case VSSRef::Foul::FREE_BALL:{
                    defaultFrame = getFreeBallPlacement(VSSRef::Color(i));
                }
                break;
                case VSSRef::Foul::GOAL_KICK:{
                    defaultFrame = getGoalKickPlacement(VSSRef::Color(i));
                }
                break;
                case VSSRef::Foul::PENALTY_KICK:{
                    defaultFrame = getPenaltyPlacement(VSSRef::Color(i));
                }
                break;
                case VSSRef::Foul::KICKOFF:{
                    defaultFrame = getKickoffPlacement(VSSRef::Color(i));
                }
                break;
                default:{
                    defaultFrame = VSSRef::Frame();
                }
                break;
            }

            // Send frame to network
            placeFrame(defaultFrame);
        }
    }

    placeBall();

    // Mark foul as processed
    _foulMutex.lock();
    _foulProcessed = true;
    _foulMutex.unlock();
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
