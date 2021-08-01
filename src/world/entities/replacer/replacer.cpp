#include "replacer.h"

#include <random>
#include <chrono>

#include <src/utils/types/field/field_default_3v3.h>
#include <src/utils/utils.h>

void movePlayerToPosition(VSSRef::Robot *bot, quint8 botId, double xCoord=0.0, double yCoord=0.0, double orientation=0.0) {
    bot->set_orientation(0.0);
    bot->set_robot_id(botId);
    bot->set_orientation(orientation);
    bot->set_x(xCoord);
    bot->set_y(yCoord);
    return;
}

void debugReplacePosition(double x, double y, int num = 0, std::string side = "") {
    std::cout << Text::blue("[REPLACER] ", true) + Text::bold("Moving player " + std::to_string(num) + " from team \"" + side + "\" " + " to position ("+ std::to_string(x) + ", " + std::to_string(y) + ")") + '\n';
}

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

        // Check if placed ball
        if(!_placedLastPosition) {
            placeBall(_lastBallPosition);
            _placedLastPosition = true;
        }
    }

    if(_foulProcessed) {
        // Reset control vars
        for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
            _placement.insert(VSSRef::Color(i), VSSRef::Frame());
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
        const double gkXPosition = factor * ((Field_Default_3v3::kFieldLength / 2000.0) - getConstants()->robotLength());
        movePlayerToPosition(frame.add_robots(), getGoalie(color), gkXPosition);

        // Attacker
        if(players.size() == 0) return frame;
        const double strikerXPosition = ((-factor) * (markX - (2.0 * getConstants()->robotLength())));
        movePlayerToPosition(frame.add_robots(), players.takeFirst(), strikerXPosition);

        // Support / Second Attacker
        if(players.size() == 0) return frame;
        const double supportXPosition = factor * (1.5 * getConstants()->robotLength());
        const double supportYPosition = markY;
        movePlayerToPosition(frame.add_robots(), players.takeFirst(), supportXPosition, supportYPosition);
    }
    else{
        // Insert GK
        if(players.size() == 0) return frame;
        const double gkXPosition = (factor * ((Field_Default_3v3::kFieldLength/2000.0) - (getConstants()->robotLength()/2.0)));
        movePlayerToPosition(frame.add_robots(), getGoalie(color), gkXPosition);

        // Attacker
        if(players.size() == 0) return frame;
        const double strikerXPosition = ((-factor) * (1.5 * getConstants()->robotLength()));
        const double strikerYPosition = -markY;
        movePlayerToPosition(frame.add_robots(), players.takeFirst(), strikerXPosition, strikerYPosition);

        // Support / Second Attacker
        if(players.size() == 0) return frame;
        const double supportXPosition = ((-factor) * (1.5 * getConstants()->robotLength()));
        const double supportYPosition = (markY - (2.0 * getConstants()->robotLength()));
        movePlayerToPosition(frame.add_robots(), players.takeFirst(), supportXPosition, supportYPosition);
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

    QList<quint8> players = _vision->getAvailablePlayers(color);
    for(int i = 0; i < players.size(); i++) {
        if(players.at(i) == getGoalie(color)) {
            players.removeAt(i);
        }
    }

    const static double length = Field_Default_3v3::kFieldLength/1000; 
    const static double width = Field_Default_3v3::kFieldWidth/1000;

    const double xStep = length/8;
    const double yStep = width/8;

    const double xPosA = xStep;
    const double yPosA = yStep;

    const double xPosB = xStep;
    const double yPosB = yStep/2 + yStep;
    
    // _color is the team that will make the kick
    if(color == getFoulColor()){
        // Random to choose GK position
        auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        std::mt19937 mt_rand(seed);
        _isGoaliePlacedAtTop = mt_rand() % 2;

        // Insert GK
        if(players.size() == 0) return frame;
        double gkOri, gkXPos, gkYPos;
        if(_isGoaliePlacedAtTop){
            gkOri = (factor * -45.0);
            gkXPos = (factor * 0.675);
            gkYPos = 0.270;
        }
        else{
            gkOri = factor * 45.0;
            gkXPos = factor * 0.675;
            gkYPos = -0.270;
        }
        movePlayerToPosition(frame.add_robots(), getGoalie(color), gkXPos, gkYPos, gkOri);

        // #1
        if(players.size() == 0) return frame;
        movePlayerToPosition(frame.add_robots(), players.takeFirst(), (factor) * (-1) * xStep, -3 * yStep);

        // #2
        if(players.size() == 0) return frame;
        movePlayerToPosition(frame.add_robots(), players.takeFirst(), (factor) * (1) * xStep, (-3) * yStep);


        // #3
        if(players.size() == 0) return frame;
        movePlayerToPosition(frame.add_robots(), players.takeFirst(), (factor) * (-3) * xStep, 3 * yStep);

        // #4
        if(players.size() == 0) return frame;
        movePlayerToPosition(frame.add_robots(), players.takeFirst(), (factor) * (-2) * xStep, 1 * yStep);

    }
    else {
        // #0
        if(players.size() == 0) return frame;
        const double gkXPos = (factor * ((Field_Default_3v3::kFieldLength/2000.0) - (getConstants()->robotLength())));
        movePlayerToPosition(frame.add_robots(), getGoalie(color), gkXPos);

        // #1
        if(players.size() == 0) return frame;
        movePlayerToPosition(frame.add_robots(), players.takeFirst(), -factor * (-1*xStep), 3*yStep);


        // #2
        if(players.size() == 0) return frame;
        movePlayerToPosition(frame.add_robots(), players.takeFirst(), -factor * (-xStep), 0);

        // #3
        if(players.size() == 0) return frame;
        // movePlayerToPosition(frame.add_robots(), players.takeFirst(), (-factor) * (-2) * xStep, 3 * yStep);
        movePlayerToPosition(frame.add_robots(), players.takeFirst(), -factor * (-3*xStep), (-2*yStep));

        // #4
        if(players.size() == 0) return frame;
        // movePlayerToPosition(frame.add_robots(), players.takeFirst(), (-factor) * (-2) * xStep, 1 * yStep);
        movePlayerToPosition(frame.add_robots(), players.takeFirst(), -factor * (-3*xStep), -4*yStep);

        
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
        const static double gk_x = factor * ((Field_Default_3v3::kFieldLength / 2000.0) - getConstants()->robotLength());
        double gk_y = 0;

        // If quadrant 2 or 3, gk will need to pos in an better way
        if(foulQuadrant == VSSRef::Quadrant::QUADRANT_2)
            gk_y = getConstants()->robotLength();
        else if(foulQuadrant == VSSRef::Quadrant::QUADRANT_3)
            gk_y = -getConstants()->robotLength();
        movePlayerToPosition(frame.add_robots(), getGoalie(color), gk_x, gk_y);


        // Attacker
        if(players.size() == 0) return frame;
        movePlayerToPosition(frame.add_robots(), players.takeFirst(), markX - getConstants()->robotLength(), markY);

        if(players.size() == 0) return frame;
        movePlayerToPosition(frame.add_robots(), players.takeFirst(), markX - getConstants()->robotLength() + (factor) * 0.2, markY);


        // Support
        if(players.size() == 0) return frame;
        double sup_x = 0;
        double sup_y = 0;
        // Support pos in different ways
        switch(foulQuadrant) {
            case VSSRef::Quadrant::QUADRANT_1:
                sup_x = 0.1;
                sup_y = -0.2;
                break;
            case VSSRef::Quadrant::QUADRANT_2:
                sup_x = -0.3;
                sup_y = -0.1;
                break;
            case VSSRef::Quadrant::QUADRANT_3:
                sup_x = -0.3;
                sup_y = 0.1;
                break;
            case VSSRef::Quadrant::QUADRANT_4:
                sup_x = 0.1;
                sup_y = 0.2;
                break;
            default:
            break;
        }
        movePlayerToPosition(frame.add_robots(), players.takeFirst(), sup_x, sup_y);
        movePlayerToPosition(frame.add_robots(), players.takeFirst(), -1000, -1000);
    }
    else{
        if(players.size() == 0) return frame;
        const static double gk_x = factor * ((Field_Default_3v3::kFieldLength / 2000.0) - getConstants()->robotLength());
        double gk_y = 0;

        // If quadrant 2 or 3, gk will need to pos in an better way
        if(foulQuadrant == VSSRef::Quadrant::QUADRANT_1)
            gk_y = getConstants()->robotLength();
        else if(foulQuadrant == VSSRef::Quadrant::QUADRANT_4)
            gk_y = -getConstants()->robotLength();
        movePlayerToPosition(frame.add_robots(), getGoalie(color), gk_x, gk_y);
        

        // Attacker
        if(players.size() == 0) return frame;
        movePlayerToPosition(frame.add_robots(), players.takeFirst(), markX + 0.2, markY);


        // Support
        if(players.size() == 0) return frame;
        double sup_x = 0;
        double sup_y = 0;

        // Support pos in different ways
        if(foulQuadrant == VSSRef::Quadrant::QUADRANT_1){
            sup_x = 0.3;
            sup_y = -0.1;
        }
        if(foulQuadrant == VSSRef::Quadrant::QUADRANT_2){
            sup_x = -0.1;
            sup_y = -0.2;
        }
        else if(foulQuadrant == VSSRef::Quadrant::QUADRANT_3){
            sup_x = -0.1;
            sup_y = 0.2;
        }
        else if(foulQuadrant == VSSRef::Quadrant::QUADRANT_4){
            sup_x = 0.3;
            sup_y = 0.1;
        }
        movePlayerToPosition(frame.add_robots(), players.takeFirst(), sup_x, sup_y);
        movePlayerToPosition(frame.add_robots(), players.takeFirst(), 1000, 1000);
        movePlayerToPosition(frame.add_robots(), players.takeFirst(), 1000, 1000);
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

VSSRef::Frame Replacer::getOutsideFieldPlacement(VSSRef::Color color){
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
    gk->set_y(-0.8);

    // Attacker
    if(players.size() == 0) return frame;
    VSSRef::Robot *striker = frame.add_robots();
    striker->set_robot_id(players.takeFirst());
    striker->set_orientation(0.0);
    striker->set_x(factor * Field_Default_3v3::kCenterRadius/1000.0);
    striker->set_y(-0.8);

    // Support
    if(players.size() == 0) return frame;
    VSSRef::Robot *support = frame.add_robots();
    support->set_robot_id(players.takeFirst());
    support->set_orientation(0.0);
    support->set_x(factor * (Field_Default_3v3::kCenterRadius/1000.0 * 2.0));
    support->set_y(-0.8);

    return frame;
}

VSSRef::Frame Replacer::getPenaltyShootoutPlacement(VSSRef::Color color, bool placeAttacker){
    VSSRef::Frame frame;
    frame.set_teamcolor(color);

    // swap side check
    bool teamIsAtLeft = (color == VSSRef::Color::BLUE && getConstants()->blueIsLeftSide()) || (color == VSSRef::Color::YELLOW && !getConstants()->blueIsLeftSide());

    float factor = 1.0;
    if(teamIsAtLeft)
        factor = -1.0;

    // Taking available players
    VSSRef::Frame lastFrame = _placement.value(color);

    // The chosen id (keeper or atk)
    quint8 id = 255;

    // Taking attacker
    if(placeAttacker) {
        // Get the closest player to ball
        float bestDistance = 999.0f;
        for(int i = 0; i < lastFrame.robots_size(); i++) {
            VSSRef::Robot robot = lastFrame.robots(i);
            Position playerPosition = Position(true, robot.x(), robot.y());
            Position futureBallPosition = getBallPlaceByFoul(VSSRef::Foul::PENALTY_KICK, color, VSSRef::Quadrant::NO_QUADRANT);

            float distBall = Utils::distance(playerPosition, futureBallPosition);
            if(distBall < bestDistance) {
                bestDistance = distBall;
                id = robot.robot_id();
            }
        }
    }
    // Taking goalie
    else {
        // Get the player inside goal area
        for(int i = 0; i < lastFrame.robots_size(); i++) {
            VSSRef::Robot robot = lastFrame.robots(i);
            Position playerPosition = Position(true, robot.x(), robot.y());
            if(Utils::isInsideGoalArea(color, playerPosition)) {
                id = robot.robot_id();
                break;
            }
        }
    }

    // Remove id from avPlayers
    QList<quint8> avPlayers = _vision->getAvailablePlayers(color);
    for(int i = 0; i < avPlayers.size(); i++) {
        if(avPlayers.at(i) == id) {
            avPlayers.removeAt(i);
            break;
        }
    }

    // Removing from field players != id
    if(avPlayers.size() == 0) return frame;
    VSSRef::Robot *out1 = frame.add_robots();
    out1->set_robot_id(avPlayers.takeFirst());
    out1->set_orientation(0.0);
    out1->set_x(factor * 0.1);
    out1->set_y(-0.8);

    if(avPlayers.size() == 0) return frame;
    VSSRef::Robot *out2 = frame.add_robots();
    out2->set_robot_id(avPlayers.takeFirst());
    out2->set_orientation(0.0);
    out2->set_x(factor * 0.2);
    out2->set_y(-0.8);

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

void Replacer::placeTeams() {
    VSSRef::Foul lastFoul = getFoul();

    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        // if team placed
        if(_placementStatus.value(VSSRef::Color(i))) {
            // Take received frame
            VSSRef::Frame teamFrame = _placement.value(VSSRef::Color(i));

            // Send frame to network
            placeFrame(teamFrame);
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

            // Save frame
            _placement.insert(VSSRef::Color(i), defaultFrame);
        }
    }

    Position foulBallPosition = getBallPlaceByFoul(_foul, _foulColor, _foulQuadrant);
    placeBall(foulBallPosition);

    // Mark foul as processed
    _foulMutex.lock();
    _foulProcessed = true;
    _foulMutex.unlock();
}

void Replacer::placeOutside(VSSRef::Foul foul, VSSRef::Color oppositeTeam) {
    if(foul == VSSRef::Foul::KICKOFF) {
        VSSRef::Frame removedFrame;

        // Filling frames
        removedFrame = getOutsideFieldPlacement(oppositeTeam);

        // Send frames to network
        placeFrame(removedFrame);
    }
    else if(foul == VSSRef::Foul::PENALTY_KICK) {
        VSSRef::Frame removedFrameKicker;
        VSSRef::Frame removedFrameGoalie;

        // Filling frames
        removedFrameKicker = getPenaltyShootoutPlacement((oppositeTeam == VSSRef::Color::BLUE) ? VSSRef::Color::YELLOW : VSSRef::Color::BLUE, true);
        removedFrameGoalie = getPenaltyShootoutPlacement(oppositeTeam, false);

        // Send frames to network
        placeFrame(removedFrameKicker);
        placeFrame(removedFrameGoalie);
    }
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

Constants* Replacer::getConstants() {
    if(_constants == nullptr) {
        std::cout << Text::red("[ERROR] ", true) << Text::bold("Constants with nullptr value at Replacer") + '\n';
    }
    else {
        return _constants;
    }

    return nullptr;
}
