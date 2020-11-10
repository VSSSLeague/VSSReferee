#include "vssreplacer.h"

QString VSSReplacer::name(){
    return "VSSReplacer";
}

VSSReplacer::VSSReplacer(const QString& refereeAddress, int replacerPort, const QString& firaSimAddress, int firaSimCommandPort, Constants* constants)
{
    // Saving addresses
    _refereeAddress     = refereeAddress;
    _replacerPort       = replacerPort;
    _firaSimAddress     = firaSimAddress;
    _firaSimCommandPort = firaSimCommandPort;

    // Constants
    _constants          = constants;

    // Create a VSS Client to listen to replacement packets
    _vssClient = new VSSClient(_replacerPort, refereeAddress.toStdString());

    // Connect socket to send placement data to firaSim
    connect(firaSimAddress, firaSimCommandPort);

    // Reset vars
    _packetsReceived = 0;
    _blueSentPacket = false;
    _yellowSentPacket = false;
    _awaitingPackets = false;

    // Set initial goalies as 0
    goalie[VSSRef::Color::BLUE] = 0;
    goalie[VSSRef::Color::YELLOW] = 1;
}

VSSReplacer::~VSSReplacer(){
    delete _vssClient;
}

void VSSReplacer::initialization(){
    // Vision system connection (firaSim)
    if(_vssClient->open(true))
        std::cout << "[VSSReplacer] Listening to replace system on port " << _replacerPort << " and address = " << _refereeAddress.toStdString() << ".\n";
    else{
        std::cout << "[VSSReplacer] Cannot listen to replace system on port " << _replacerPort << " and address = " << _refereeAddress.toStdString() << ".\n";
        this->stopRunning();
    }
}

void VSSReplacer::loop(){
    VSSRef::team_to_ref::VSSRef_Placement packet;
    if(_vssClient->receive(packet)){
        if(packet.has_world()){
            VSSRef::Frame frame = packet.world();

            // Avoid to receive more than 1 packet from teams
            bool duplicated = false;
            if(frame.teamcolor() == VSSRef::Color::BLUE){
                _mutex.lock();

                if(!_blueSentPacket){
                    _blueSentPacket = true;
                }
                else duplicated = true;

                _mutex.unlock();
            }
            else if(frame.teamcolor() == VSSRef::Color::YELLOW){
                _mutex.lock();

                if(!_yellowSentPacket){
                    _yellowSentPacket = true;
                }
                else duplicated = true;

                _mutex.unlock();
            }

            if(!duplicated){
                // Debug frame (uncomment this if you want to)
                //debugFrame(frame);

                // Save Frame from team
                frames[frame.teamcolor()] = frame;

                std::string teamColor = frame.teamcolor() == VSSRef::Color::BLUE ? "BLUE" : "YELLOW";
                std::cout << "[VSSReplacer] Received packet from team " << teamColor << std::endl;

                // Send signal to VSSReferee
                emit teamPlaced(frame.teamcolor());

                _packetsReceived++;
            }
        }
    }
}

void VSSReplacer::finalization(){
    _vssClient->close();
}

void VSSReplacer::sendPacket(fira_message::sim_to_ref::Packet command){
    std::string msg;
    command.SerializeToString(&msg);

    if(_socket.write(msg.c_str(), msg.length()) == -1){
        std::cout << "[VSSReplacer] Failed to write to firaSim socket: " << _socket.errorString().toStdString() << std::endl;
    }
}

bool VSSReplacer::connect(const QString &firaSimAddress, int firaSimCommandPort){
    // Connect to referee address and port
    if(_socket.isOpen())
        _socket.close();

    _socket.connectToHost(firaSimAddress, firaSimCommandPort, QIODevice::WriteOnly, QAbstractSocket::IPv4Protocol);

    std::cout << "[VSSReplacer] Writing to firaSim replacement system on port " << firaSimCommandPort << " and address = " << firaSimAddress.toStdString() << ".\n";

    return true;
}

void VSSReplacer::disconnect() {
    // Close referee socket
    if(_socket.isOpen()){
        _socket.close();
    }
}

bool VSSReplacer::isConnected() const {
    return (_socket.isOpen());
}

QString VSSReplacer::getFoulNameById(VSSRef::Foul foul){
    switch(foul){
        case VSSRef::Foul::FREE_BALL:    return "FREE_BALL";
        case VSSRef::Foul::FREE_KICK:    return "FREE_KICK";
        case VSSRef::Foul::GOAL_KICK:    return "GOAL_KICK";
        case VSSRef::Foul::PENALTY_KICK: return "PENALTY_KICK";
        case VSSRef::Foul::KICKOFF:      return "KICKOFF";
        case VSSRef::Foul::STOP:         return "STOP";
        case VSSRef::Foul::GAME_ON: return "GAME_ON";
        default:                         return "FOUL NOT IDENTIFIED";
    }
}

void VSSReplacer::takeFoul(VSSRef::Foul foul, VSSRef::Color color, VSSRef::Quadrant quadrant){
    _mutex.lock();
    _yellowSentPacket = false;
    _blueSentPacket   = false;
    _awaitingPackets  = true;
    _packetsReceived  = 0;

    _foul = foul;
    _color = color;
    _quadrant = quadrant;
    _mutex.unlock();

    std::cout << "[VSSReplacer] Waiting for placement packets for foul: " << getFoulNameById(foul).toStdString() << "\n";
}

void VSSReplacer::stopWaiting(){
    _mutex.lock();

    // Check team that don't placed and place
    if(!_yellowSentPacket){
        VSSRef::Frame *frame = new VSSRef::Frame();
        if(_foul == VSSRef::Foul::PENALTY_KICK)
            frame = getPenaltyPlacement(VSSRef::Color::YELLOW);
        else if(_foul == VSSRef::Foul::KICKOFF)
            frame = getKickoffPlacement(VSSRef::Color::YELLOW);
        else if(_foul == VSSRef::Foul::FREE_BALL)
            frame = getFreeBallPlacement(VSSRef::Color::YELLOW);
        else if(_foul == VSSRef::Foul::GOAL_KICK)
            frame = getGoalKickPlacement(VSSRef::Color::YELLOW);

        fillAndSendPacket(frame);
    }
    else{
        fillAndSendPacket(&frames[VSSRef::Color::YELLOW]);
    }

    if(!_blueSentPacket){
        VSSRef::Frame *frame = new VSSRef::Frame();
        if(_foul == VSSRef::Foul::PENALTY_KICK)
            frame = getPenaltyPlacement(VSSRef::Color::BLUE);
        else if(_foul == VSSRef::Foul::KICKOFF)
            frame = getKickoffPlacement(VSSRef::Color::BLUE);
        else if(_foul == VSSRef::Foul::FREE_BALL)
            frame = getFreeBallPlacement(VSSRef::Color::BLUE);
        else if(_foul == VSSRef::Foul::GOAL_KICK)
            frame = getGoalKickPlacement(VSSRef::Color::BLUE);

        fillAndSendPacket(frame);
    }else{
        fillAndSendPacket(&frames[VSSRef::Color::BLUE]);
    }

    _yellowSentPacket = false;
    _blueSentPacket   = false;
    _awaitingPackets  = false;
    _packetsReceived  = 0;
    _mutex.unlock();

    // Place ball at foul location
    std::cout << "Goleiro fica encima: " << _isGoaliePlacedAtTop << std::endl;
    vector2d ballPlacePos = getBallPlaceByFoul(_foul, _color, _quadrant);
    std::cout << "[VSSReplacer] Ball placed into x: " << ballPlacePos.x << " and y: " << ballPlacePos.y << "\n";
    placeBall(ballPlacePos.x, ballPlacePos.y);
}

void VSSReplacer::fillAndSendPacket(VSSRef::Frame *frame){
    // Creating command
    fira_message::sim_to_ref::Replacement *replacementCommand = new fira_message::sim_to_ref::Replacement();

    // Filling blue robots
    int sz = frame->robots_size();
    for(int x = 0; x < sz; x++){
        // Taking robot from frame
        VSSRef::Robot robotAt = frame->robots(x);
        // goalkeeper (is inside goal area)
        if(Utils::isInsideGoalArea(frame->teamcolor(), vector2d(robotAt.x(), robotAt.y()))){
            // if goalkeeper color is equal to the team that will take the foul (goal kick)
            if(_color == frame->teamcolor()){
                if(robotAt.y() >= 0){
                    // if robot desired y >= 0
                    _isGoaliePlacedAtTop = true;
                }
                else{
                    // if not
                    _isGoaliePlacedAtTop = false;
                }
            }
        }
        parseRobot(replacementCommand, &robotAt, frame->teamcolor());
    }

    // Filling packet with replacement data
    fira_message::sim_to_ref::Packet packet;
    packet.set_allocated_replace(replacementCommand);

    // Send packet to firaSim
    sendPacket(packet);
}

void VSSReplacer::parseRobot(fira_message::sim_to_ref::Replacement *replacementPacket, VSSRef::Robot *robot, VSSRef::Color robotTeam){
    // Creating firaRobot and robotPosition
    fira_message::sim_to_ref::RobotReplacement *firaRobot = replacementPacket->add_robots();
    fira_message::Robot *robotPosition = new fira_message::Robot();

    // Parsing position
    robotPosition->set_x(robot->x());
    robotPosition->set_y(robot->y());
    robotPosition->set_robot_id(robot->robot_id());
    robotPosition->set_orientation(robot->orientation());

    // Inserting infos in firaRobot
    firaRobot->set_turnon(true);
    firaRobot->set_yellowteam(robotTeam);
    firaRobot->set_allocated_position(robotPosition);
}

void VSSReplacer::placeBall(double x, double y){
    // Creating firaBall
    fira_message::sim_to_ref::BallReplacement *firaBall = new fira_message::sim_to_ref::BallReplacement();

    // Set ball informations
    firaBall->set_x(x);
    firaBall->set_y(y);
    firaBall->set_vx(0.0);
    firaBall->set_vx(0.0);

    // Inserting infos at replacementCommand
    fira_message::sim_to_ref::Replacement *ballReplaceCommand = new fira_message::sim_to_ref::Replacement();
    ballReplaceCommand->set_allocated_ball(firaBall);

    // Sending to fira
    fira_message::sim_to_ref::Packet packet;
    packet.set_allocated_replace(ballReplaceCommand);

    // Send packet to firaSim
    sendPacket(packet);
}

void VSSReplacer::debugFrame(VSSRef::Frame frame){
    std::cout << "Team color: " << frame.teamcolor() << std::endl;
    for(int x = 0; x < frame.robots_size(); x++){
        std::cout << "Robot " << frame.robots(x).robot_id() << " : " << std::endl;
        std::cout << "x: " << frame.robots(x).x() << " y: " << frame.robots(x).y() << " ori: " << frame.robots(x).orientation() << std::endl;
    }
}

vector2d VSSReplacer::getBallPlaceByFoul(VSSRef::Foul foul, VSSRef::Color color, VSSRef::Quadrant quadrant){
    float goalKickX = (FieldConstantsVSS::kFieldLength / 1000.0)/2.0 - 0.15;
    float markX = (FieldConstantsVSS::kFieldLength / 1000.0)/2.0 - 0.375;
    float markY = (FieldConstantsVSS::kFieldWidth / 1000.0)/2.0 - 0.25;

    switch(foul){
        case VSSRef::Foul::KICKOFF:{
            return vector2d(0.0, 0.0);
        }
        break;
        case VSSRef::Foul::FREE_BALL:{
            if(quadrant == VSSRef::Quadrant::QUADRANT_1){
                return vector2d(markX, markY);
            }
            else if(quadrant == VSSRef::Quadrant::QUADRANT_2){
                return vector2d(-markX, markY);
            }
            else if(quadrant == VSSRef::Quadrant::QUADRANT_3){
                return vector2d(-markX, -markY);
            }
            else if(quadrant == VSSRef::Quadrant::QUADRANT_4){
                return vector2d(markX, -markY);
            }
        }
        break;
        case VSSRef::Foul::GOAL_KICK:{
            if(color == VSSRef::Color::BLUE){
                if(RefereeView::getBlueIsLeftSide()) return vector2d(-goalKickX, (_isGoaliePlacedAtTop) ? (0.375 - getConstants()->getBallRadius()) : (-0.375 + getConstants()->getBallRadius()));
                else return vector2d(goalKickX, (_isGoaliePlacedAtTop) ? (0.375 - getConstants()->getBallRadius()) : (-0.375 + getConstants()->getBallRadius()));
            }
            else if(color == VSSRef::Color::YELLOW){
                if(RefereeView::getBlueIsLeftSide()) return vector2d(goalKickX, (_isGoaliePlacedAtTop) ? (0.375 - getConstants()->getBallRadius()) : (-0.375 + getConstants()->getBallRadius()));
                else return vector2d(-goalKickX, (_isGoaliePlacedAtTop) ? (0.375 - getConstants()->getBallRadius()) : (-0.375 + getConstants()->getBallRadius()));
            }
        }
        break;
        case VSSRef::Foul::PENALTY_KICK:{
            if(color == VSSRef::Color::BLUE){
                if(RefereeView::getBlueIsLeftSide()) return vector2d(markX, 0);
                else return vector2d(-markX, 0.0);
            }
            else if(color == VSSRef::Color::YELLOW){
                if(RefereeView::getBlueIsLeftSide()) return vector2d(-markX, 0);
                else return vector2d(markX, 0.0);
            }
        }
        break;
        case VSSRef::Foul::FREE_KICK:{
            if(color == VSSRef::Color::BLUE){
                if(RefereeView::getBlueIsLeftSide()) return vector2d(markX, 0);
                else return vector2d(-markX, 0.0);
            }
            else if(color == VSSRef::Color::YELLOW){
                if(RefereeView::getBlueIsLeftSide()) return vector2d(-markX, 0);
                else return vector2d(markX, 0.0);
            }
        }
        break;
        default:{
            return vector2d(0.0, 0.0);
        }
    }

    return vector2d(0.0, 0.0);
}

VSSRef::Frame* VSSReplacer::getPenaltyPlacement(VSSRef::Color color){
    VSSRef::Frame* frame = new VSSRef::Frame();
    frame->set_teamcolor(color);

    // swap side check
    bool teamIsAtLeft = (color == VSSRef::Color::BLUE && RefereeView::getBlueIsLeftSide()) || (color == VSSRef::Color::YELLOW && !RefereeView::getBlueIsLeftSide());

    float factor = 1.0;
    if(teamIsAtLeft)
        factor = -1.0;

    // FB mark
    float markX = (FieldConstantsVSS::kFieldLength / 1000.0)/2.0 - 0.375;
    float markY = (FieldConstantsVSS::kFieldWidth / 1000.0)/2.0 - 0.25;

    emit requestGoalie(color);
    QList<int> players;
    for(int x = 0; x < getConstants()->getQtPlayers(); x++){
        if(x != goalie[color]) players.push_back(x);
    }

    // _color is the team that will make the kick
    if(color == _color){
        // Insert GK
        VSSRef::Robot *gk = frame->add_robots();
        gk->set_robot_id(goalie[color]);
        gk->set_orientation(0.0);
        gk->set_x(factor * ((FieldConstantsVSS::kFieldLength / 2000.0) - getConstants()->getRobotLength()));
        gk->set_y(0.0);

        // Attacker
        VSSRef::Robot *striker = frame->add_robots();
        striker->set_robot_id(players.takeFirst());
        striker->set_orientation(0.0);
        striker->set_x((-factor) * (markX - (2.0 * getConstants()->getRobotLength())));
        striker->set_y(0.0);

        // Support / Second Attacker
        VSSRef::Robot *support = frame->add_robots();
        support->set_robot_id(players.takeFirst());
        support->set_orientation(0.0);
        support->set_x(factor * (1.5 * getConstants()->getRobotLength()));
        support->set_y(markY);
    }
    else{
        // Insert GK
        VSSRef::Robot *gk = frame->add_robots();
        gk->set_robot_id(goalie[color]);
        gk->set_orientation(0.0);
        gk->set_x(factor * ((FieldConstantsVSS::kFieldLength/2000.0) - (getConstants()->getRobotLength()/2.0)));
        gk->set_y(0.0);

        // Attacker
        VSSRef::Robot *striker = frame->add_robots();
        striker->set_robot_id(players.takeFirst());
        striker->set_orientation(0.0);
        striker->set_x((-factor) * (1.5 * getConstants()->getRobotLength()));
        striker->set_y(-markY);

        // Support / Second Attacker
        VSSRef::Robot *support = frame->add_robots();
        support->set_robot_id(players.takeFirst());
        support->set_orientation(0.0);
        support->set_x((-factor) * (1.5 * getConstants()->getRobotLength()));
        support->set_y(markY - (2.0 * getConstants()->getRobotLength()));
    }

    return frame;
}

VSSRef::Frame* VSSReplacer::getGoalKickPlacement(VSSRef::Color color){
    VSSRef::Frame* frame = new VSSRef::Frame();
    frame->set_teamcolor(color);

    // swap side check
    bool teamIsAtLeft = (color == VSSRef::Color::BLUE && RefereeView::getBlueIsLeftSide()) || (color == VSSRef::Color::YELLOW && !RefereeView::getBlueIsLeftSide());

    float factor = 1.0;
    if(teamIsAtLeft)
        factor = -1.0;

    // FB mark
    float markX = (FieldConstantsVSS::kFieldLength / 1000.0)/2.0 - 0.375;
    float markY = (FieldConstantsVSS::kFieldWidth / 1000.0)/2.0 - 0.25;

    emit requestGoalie(color);
    QList<int> players;
    for(int x = 0; x < getConstants()->getQtPlayers(); x++){
        if(x != goalie[color]) players.push_back(x);
    }

    // _color is the team that will make the kick
    if(color == _color){
        // Random to choose GK position
        auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        std::mt19937 mt_rand(seed);
        _isGoaliePlacedAtTop = mt_rand() % 2;

        std::cout << "Botamo o goleiro encima: " << _isGoaliePlacedAtTop << std::endl;

        // Insert GK
        VSSRef::Robot *gk = frame->add_robots();
        gk->set_robot_id(goalie[color]);
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
        VSSRef::Robot *striker = frame->add_robots();
        striker->set_robot_id(players.takeFirst());
        striker->set_orientation(0.0);
        striker->set_x((factor) * (markX + getConstants()->getRobotLength()));
        striker->set_y(markY - getConstants()->getRobotLength());

        // Support / Second Attacker
        VSSRef::Robot *support = frame->add_robots();
        support->set_robot_id(players.takeFirst());
        support->set_orientation(0.0);
        support->set_x((factor) * (markX - getConstants()->getRobotLength()));
        support->set_y(-markY - getConstants()->getRobotLength());
    }
    else{
        // Insert GK
        VSSRef::Robot *gk = frame->add_robots();
        gk->set_robot_id(goalie[color]);
        gk->set_orientation(0.0);
        gk->set_x(factor * ((FieldConstantsVSS::kFieldLength/2000.0) - (getConstants()->getRobotLength())));
        gk->set_y(0.0);

        // Attacker
        VSSRef::Robot *striker = frame->add_robots();
        striker->set_robot_id(players.takeFirst());
        striker->set_orientation(0.0);
        striker->set_x((-factor) * (markX - (2.0 * getConstants()->getRobotLength())));
        striker->set_y(markY - (4.0 * getConstants()->getRobotLength()));

        // Support / Second Attacker
        VSSRef::Robot *support = frame->add_robots();
        support->set_robot_id(players.takeFirst());
        support->set_orientation(0.0);
        support->set_x((-factor) * (markX - (3.0 * getConstants()->getRobotLength())));
        support->set_y(-markY + getConstants()->getRobotLength());
    }

    return frame;
}

VSSRef::Frame* VSSReplacer::getFreeBallPlacement(VSSRef::Color color){
    VSSRef::Frame* frame = new VSSRef::Frame();
    frame->set_teamcolor(color);

    // swap side check
    bool teamIsAtLeft = (color == VSSRef::Color::BLUE && RefereeView::getBlueIsLeftSide()) || (color == VSSRef::Color::YELLOW && !RefereeView::getBlueIsLeftSide());

    float factor = 1.0;
    if(teamIsAtLeft)
        factor = -1.0;

    // FB Mark
    float markX = (FieldConstantsVSS::kFieldLength / 1000.0)/2.0 - 0.375;
    float markY = (FieldConstantsVSS::kFieldWidth / 1000.0)/2.0 - 0.25;

    if(_quadrant == VSSRef::Quadrant::QUADRANT_2 || _quadrant == VSSRef::Quadrant::QUADRANT_3)
        markX *= -1;

    if(_quadrant == VSSRef::Quadrant::QUADRANT_3 || _quadrant == VSSRef::Quadrant::QUADRANT_4)
        markY *= -1;

    emit requestGoalie(color);
    QList<int> players;
    for(int x = 0; x < getConstants()->getQtPlayers(); x++){
        if(x != goalie[color]) players.push_back(x);
    }

    // First discover if FB will occur at our side
    if(teamIsAtLeft){
        VSSRef::Robot *gk = frame->add_robots();
        gk->set_robot_id(goalie[color]);
        gk->set_orientation(0.0);
        gk->set_x(factor * ((FieldConstantsVSS::kFieldLength / 2000.0) - getConstants()->getRobotLength()));
        gk->set_y(0.0);

        // If quadrant 2 or 3, gk will need to pos in an better way
        if(_quadrant == VSSRef::Quadrant::QUADRANT_2)
            gk->set_y(getConstants()->getRobotLength());
        else if(_quadrant == VSSRef::Quadrant::QUADRANT_3)
            gk->set_y(-getConstants()->getRobotLength());

        // Attacker
        VSSRef::Robot *striker = frame->add_robots();
        striker->set_robot_id(players.takeFirst());
        striker->set_orientation(0.0);
        striker->set_x(markX - 0.2);
        striker->set_y(markY);

        // Support
        VSSRef::Robot *support = frame->add_robots();
        support->set_robot_id(players.takeFirst());
        support->set_orientation(0.0);

        // Support pos in different ways
        if(_quadrant == VSSRef::Quadrant::QUADRANT_1){
            support->set_x(0.1);
            support->set_y(-0.2);
        }
        if(_quadrant == VSSRef::Quadrant::QUADRANT_2){
            support->set_x(-0.3);
            support->set_y(-0.1);
        }
        else if(_quadrant == VSSRef::Quadrant::QUADRANT_3){
            support->set_x(-0.3);
            support->set_y(0.1);
        }
        else if(_quadrant == VSSRef::Quadrant::QUADRANT_4){
            support->set_x(0.1);
            support->set_y(0.2);
        }
    }
    else{
        VSSRef::Robot *gk = frame->add_robots();
        gk->set_robot_id(goalie[color]);
        gk->set_orientation(0.0);
        gk->set_x(factor * ((FieldConstantsVSS::kFieldLength / 2000.0) - getConstants()->getRobotLength()));
        gk->set_y(0.0);

        // If quadrant 2 or 3, gk will need to pos in an better way
        if(_quadrant == VSSRef::Quadrant::QUADRANT_1)
            gk->set_y(getConstants()->getRobotLength());
        else if(_quadrant == VSSRef::Quadrant::QUADRANT_4)
            gk->set_y(-getConstants()->getRobotLength());

        // Attacker
        VSSRef::Robot *striker = frame->add_robots();
        striker->set_robot_id(players.takeFirst());
        striker->set_orientation(0.0);
        striker->set_x(markX + 0.2);
        striker->set_y(markY);

        // Support
        VSSRef::Robot *support = frame->add_robots();
        support->set_robot_id(players.takeFirst());
        support->set_orientation(0.0);

        // Support pos in different ways
        if(_quadrant == VSSRef::Quadrant::QUADRANT_1){
            support->set_x(0.3);
            support->set_y(-0.1);
        }
        if(_quadrant == VSSRef::Quadrant::QUADRANT_2){
            support->set_x(-0.1);
            support->set_y(-0.2);
        }
        else if(_quadrant == VSSRef::Quadrant::QUADRANT_3){
            support->set_x(-0.1);
            support->set_y(0.2);
        }
        else if(_quadrant == VSSRef::Quadrant::QUADRANT_4){
            support->set_x(0.3);
            support->set_y(0.1);
        }
    }

    return frame;
}

VSSRef::Frame* VSSReplacer::getKickoffPlacement(VSSRef::Color color){
    VSSRef::Frame* frame = new VSSRef::Frame();
    frame->set_teamcolor(color);

    // swap side check
    bool teamIsAtLeft = (color == VSSRef::Color::BLUE && RefereeView::getBlueIsLeftSide()) || (color == VSSRef::Color::YELLOW && !RefereeView::getBlueIsLeftSide());

    float factor = 1.0;
    if(teamIsAtLeft)
        factor = -1.0;

    emit requestGoalie(color);
    QList<int> players;
    for(int x = 0; x < getConstants()->getQtPlayers(); x++){
        if(x != goalie[color]) players.push_back(x);
    }

    // Goalkeeper
    VSSRef::Robot *gk = frame->add_robots();
    gk->set_robot_id(goalie[color]);
    gk->set_orientation(0.0);
    gk->set_x(factor * ((FieldConstantsVSS::kFieldLength / 2000.0) - getConstants()->getRobotLength()));
    gk->set_y(0.0);

    // Attacker
    VSSRef::Robot *striker = frame->add_robots();
    striker->set_robot_id(players.takeFirst());
    striker->set_orientation(0.0);
    striker->set_x(factor * FieldConstantsVSS::kCenterRadius/1000.0);
    striker->set_y(0.0);

    // Support
    VSSRef::Robot *support = frame->add_robots();
    support->set_robot_id(players.takeFirst());
    support->set_orientation(0.0);
    support->set_x(factor * (FieldConstantsVSS::kCenterRadius/1000.0 * 2.0));
    support->set_y(0.0);

    return frame;
}

void VSSReplacer::takeGoalie(VSSRef::Color team, int id){
    _goalieMutex.lock();
    goalie[team] = id;
    _goalieMutex.unlock();
}

Constants* VSSReplacer::getConstants(){
    if(_constants == NULL){
        std::cout << "[ERROR] Referee is requesting constants, but it's NULL\n";
        return NULL;
    }

    return _constants;
}
