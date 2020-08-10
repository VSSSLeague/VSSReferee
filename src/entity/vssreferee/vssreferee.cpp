#include "vssreferee.h"
#include <src/entity/refereeview/refereeview.h>
#include <src/entity/refereeview/soccerview/util/field_default_constants.h>

QString VSSReferee::name(){
    return "VSSReferee";
}

VSSReferee::VSSReferee(VSSVisionClient *visionClient, const QString& refereeAddress, int refereePort)
{
    _visionClient   = visionClient;
    _refereeAddress = refereeAddress;
    _refereePort    = refereePort;

    connect(refereeAddress, refereePort);

    // Reset vars initially
    _placementIsSet = false;
    _blueSent       = false;
    _yellowSent     = false;
    timePassed      = 0;
    alreadySet      = false;
    startedGKTimer  = false;

    // Start timers
    _placementTimer.start();
    _gameTimer.start();
    _gkTimer.start();
    _ballStuckTimer.start();
    _ballVelTimer.start();

}

VSSReferee::~VSSReferee(){

}

void VSSReferee::initialization(){

}

void VSSReferee::loop(){
    // Checking if half passed, reseting the time count
    _gameTimer.stop();
    if((_gameTimer.timesec() + timePassed) > GAME_HALF_TIME){
        RefereeView::addRefereeWarning("Half passed!");
        _gameTimer.start();
        timePassed = 0;
    }

    // Checking timer overextended if a foul is set
    if(_placementIsSet){
        // Saves game passed time
        _gameTimer.stop();
        timePassed += _gameTimer.timesec();
        _gameTimer.start();

        _placementTimer.stop();
        RefereeView::setCurrentTime(PLACEMENT_WAIT_TIME - _placementTimer.timesec());
        if((_placementTimer.timensec() / 1E9) >= PLACEMENT_WAIT_TIME && (!_blueSent || !_yellowSent)){
            // If enters here, one of the teams didn't placed as required in the determined time
            if(!_blueSent){
                RefereeView::addRefereeWarning("Blue Team hasn't placed.");
                std::cout << "[VSSReferee] Team BLUE hasn't sent the placement command." << std::endl;
            }
            if(!_yellowSent){
                RefereeView::addRefereeWarning("Yellow Team hasn't placed.");
                std::cout << "[VSSReferee] Team YELLOW hasn't sent the placement command." << std::endl;
            }
            _placementMutex.lock();
            _blueSent = false;
            _yellowSent = false;
            _placementIsSet = false;
            _placementMutex.unlock();

            // Do something here (an foul when one of the teams haven't placed ?)

        }
        else if(_blueSent && _yellowSent){
            _placementMutex.lock();
            _blueSent = false;
            _yellowSent = false;
            _placementIsSet = false;
            _placementMutex.unlock();
        }
    }
    else{
        // Updating game left time
        _gameTimer.stop();
        RefereeView::setCurrentTime(GAME_HALF_TIME - (_gameTimer.timesec() + timePassed));
        RefereeView::setRefereeCommand("GAME_ON");

        checkTwoPlayersInsideGoalAreaWithBall();
        checkTwoPlayersAttackingAtGoalArea();
        checkBallStucked();
        checkGKTakeoutTimeout();

    }
}

void VSSReferee::finalization(){
    disconnect();
    std::cout << "[VSSReferee] Thread ended" << std::endl;
}

void VSSReferee::sendPacket(VSSRef::ref_to_team::VSSRef_Command command){
    std::string msg;
    command.SerializeToString(&msg);

    if(_socket.write(msg.c_str(), msg.length()) == -1){
        std::cout << "[VSSReferee] Failed to write to socket: " << _socket.errorString().toStdString() << std::endl;
    }
    else{
        _placementMutex.lock();
        _placementIsSet = true;
        _blueSent = false;
        _yellowSent = false;
        _placementTimer.start();
        _placementMutex.unlock();
    }
}

bool VSSReferee::connect(const QString &refereeAddress, int refereePort){
    // Connect to referee address and port
    if(_socket.isOpen())
        _socket.close();

    _socket.connectToHost(refereeAddress, refereePort, QIODevice::WriteOnly, QAbstractSocket::IPv4Protocol);

    std::cout << "[VSSReferee] Writing to referee system on port " << _refereePort << " and address = " << _refereeAddress.toStdString() << ".\n";

    return true;
}

void VSSReferee::disconnect() {
    // Close referee socket
    if(_socket.isOpen()){
        _socket.close();
    }
}

bool VSSReferee::isConnected() const {
    return (_socket.isOpen());
}

void VSSReferee::teamSent(VSSRef::Color color){
    /// TODO HERE
    if(color == VSSRef::Color::BLUE){
        _placementMutex.lock();
        _blueSent = true;
        _placementMutex.unlock();
    }
    else if(color == VSSRef::Color::YELLOW){
        _placementMutex.lock();
        _yellowSent = true;
        _placementMutex.unlock();
    }
}

QString VSSReferee::getFoulNameById(VSSRef::Foul foul){
    switch(foul){
        case VSSRef::Foul::FREE_BALL:    return "FREE_BALL";
        case VSSRef::Foul::FREE_KICK:    return "FREE_KICK";
        case VSSRef::Foul::GOAL_KICK:    return "GOAL_KICK";
        case VSSRef::Foul::PENALTY_KICK: return "PENALTY_KICK";
        default:                         return "FOUL NOT IDENTIFIED";
    }
}

void VSSReferee::setTeamFoul(VSSRef::Foul foul, VSSRef::Color forTeam, VSSRef::Quadrant foulQuadrant){
    _refereeCommand.set_foul(foul);
    _refereeCommand.set_teamcolor(forTeam);
    _refereeCommand.set_foulquadrant(foulQuadrant);

    if(isConnected()){
        sendPacket(_refereeCommand);
        RefereeView::addRefereeCommand(getFoulNameById(_refereeCommand.foul()));
        emit setFoul(_refereeCommand.foul());
    }
}

// Fouls detection
bool VSSReferee::checkTwoPlayersInsideGoalAreaWithBall(){
    fira_message::Frame frame = _visionClient->getDetectionData();

    // Checking for blue team
    int bluePlayersAtGoal = 0;
    bool ballIsAtBlueGoal = Utils::isInsideGoalArea(VSSRef::Color::BLUE, vector2d(frame.ball().x(), frame.ball().y()));
    for(int x = 0; x < frame.robots_blue().size(); x++){
        if(Utils::isInsideGoalArea(VSSRef::Color::BLUE, vector2d(frame.robots_blue(x).x(), frame.robots_blue(x).y())))
            bluePlayersAtGoal++;
    }
    if(bluePlayersAtGoal >= 2 && ballIsAtBlueGoal){
        setTeamFoul(VSSRef::Foul::PENALTY_KICK, VSSRef::Color::YELLOW);
        return true;
    }
    // Checking for yellow team
    int yellowPlayersAtGoal = 0;
    bool ballIsAtYellowGoal = Utils::isInsideGoalArea(VSSRef::Color::YELLOW, vector2d(frame.ball().x(), frame.ball().y()));
    for(int x = 0; x < frame.robots_yellow().size(); x++){
        if(Utils::isInsideGoalArea(VSSRef::Color::YELLOW, vector2d(frame.robots_yellow(x).x(), frame.robots_yellow(x).y())))
            yellowPlayersAtGoal++;
    }
    if(yellowPlayersAtGoal >= 2 && ballIsAtYellowGoal){
        setTeamFoul(VSSRef::Foul::PENALTY_KICK, VSSRef::Color::BLUE);
        return true;
    }
    return false;
}

bool VSSReferee::checkTwoPlayersAttackingAtGoalArea(){
    fira_message::Frame frame = _visionClient->getDetectionData();

    // Checking for blue team
    int enemyPlayersAtBlueGoal = 0;
    bool ballIsAtBlueGoal = Utils::isInsideGoalArea(VSSRef::Color::BLUE, vector2d(frame.ball().x(), frame.ball().y()));
    for(int x = 0; x < frame.robots_yellow().size(); x++){
        if(Utils::isInsideGoalArea(VSSRef::Color::BLUE, vector2d(frame.robots_yellow(x).x(), frame.robots_yellow(x).y())))
            enemyPlayersAtBlueGoal++;
    }
    if(enemyPlayersAtBlueGoal >= 2 && ballIsAtBlueGoal){
        setTeamFoul(VSSRef::Foul::GOAL_KICK, VSSRef::Color::BLUE);
        return true;
    }
    // Checking for yellow team
    int enemyPlayersAtYellowGoal = 0;
    bool ballIsAtYellowGoal = Utils::isInsideGoalArea(VSSRef::Color::YELLOW, vector2d(frame.ball().x(), frame.ball().y()));
    for(int x = 0; x < frame.robots_blue().size(); x++){
        if(Utils::isInsideGoalArea(VSSRef::Color::YELLOW, vector2d(frame.robots_blue(x).x(), frame.robots_blue(x).y())))
            enemyPlayersAtYellowGoal++;
    }
    if(enemyPlayersAtYellowGoal >= 2 && ballIsAtYellowGoal){
        setTeamFoul(VSSRef::Foul::GOAL_KICK, VSSRef::Color::YELLOW);
        return true;
    }
    return false;
}

bool VSSReferee::checkGKTakeoutTimeout(){
    fira_message::Frame frame = _visionClient->getDetectionData();

    int playersAtBlueGoal = 0;
    bool isBallInsideBlueGoal = Utils::isInsideGoalArea(VSSRef::Color::BLUE, vector2d(frame.ball().x(), frame.ball().y()));
    bool isBallInsideYellowGoal = Utils::isInsideGoalArea(VSSRef::Color::YELLOW, vector2d(frame.ball().x(), frame.ball().y()));

    // Checking for blue team if ball is inside their goal
    if(isBallInsideBlueGoal){
        for(int x = 0; x < frame.robots_yellow().size(); x++){
            if(Utils::isInsideGoalArea(VSSRef::Color::BLUE, vector2d(frame.robots_yellow(x).x(), frame.robots_yellow(x).y())))
                playersAtBlueGoal++;
        }
        if(playersAtBlueGoal == 0){
            // Check if timer is started, if don't, start it and check after
            if(!startedGKTimer){
                _gkTimer.start();
                startedGKTimer = true;
            }
            else{
                _gkTimer.stop();
                char str[1024];
                snprintf(str, 1023, "%.2f", _gkTimer.timesec());
                RefereeView::drawText(vector2d(frame.ball().x() * 1000.0, frame.ball().y() * 1000.0), str);
                if(_gkTimer.timesec() >= GK_TIME_TAKEOUT){
                    setTeamFoul(VSSRef::Foul::PENALTY_KICK, VSSRef::Color::YELLOW);
                    return true;
                }
            }
        }
    }
    // Checking for yellow team if ball is inside their goal
    else if(isBallInsideYellowGoal){
        int playersAtYellowGoal = 0;
        for(int x = 0; x < frame.robots_blue().size(); x++){
            if(Utils::isInsideGoalArea(VSSRef::Color::YELLOW, vector2d(frame.robots_blue(x).x(), frame.robots_blue(x).y())))
                playersAtYellowGoal++;
        }
        if(playersAtYellowGoal == 0){
            // Check if timer is started, if don't, start it and check after
            if(!startedGKTimer){
                _gkTimer.start();
                startedGKTimer = true;
            }
            else{
                _gkTimer.stop();
                char str[1024];
                snprintf(str, 1023, "%.2f", _gkTimer.timesec());
                RefereeView::drawText(vector2d(frame.ball().x() * 1000.0, frame.ball().y() * 1000.0), str);
                if(_gkTimer.timesec() >= GK_TIME_TAKEOUT){
                    setTeamFoul(VSSRef::Foul::PENALTY_KICK, VSSRef::Color::BLUE);
                    return true;
                }
            }
        }
    }
    else{
        startedGKTimer = false;
    }

    return false;
}

bool VSSReferee::checkBallStucked(){
    fira_message::Frame frame = _visionClient->getDetectionData();
    vector2d ballPos = vector2d(frame.ball().x(), frame.ball().y());

    // Update ball velocity
    _ballVelTimer.stop();
    float vx = (frame.ball().x() - lastBallPos.x) / _ballVelTimer.timesec();
    float vy = (frame.ball().y() - lastBallPos.y) / _ballVelTimer.timesec();
    if(isnan(vx) || isnan(vy)) vx = vy = 0.0;
    _ballVelTimer.start();
    lastBallPos = ballPos;

    float ballVelocity = sqrt(pow(vx, 2) + pow(vy, 2));

    if(ballVelocity > BALL_MINVELOCITY){
        _ballStuckTimer.start();
    }
    else{
        // Check if a player is at least L * sqrt(2) dist from the ball (possible stuck)
        /// CHECK THIS LATER WITH JSON
        bool haveAtLeastOne = false;
        VSSRef::Color closestColor;
        float closestDist = 999.0f;
        float playerMaxDistToBall = 0.075 * sqrt(2); // meters
        for(int x = 0; x < frame.robots_blue_size(); x++){
            float dist = Utils::distance(vector2d(frame.robots_blue(x).x(), frame.robots_blue(x).y()), ballPos);
            if(dist <= playerMaxDistToBall){
                haveAtLeastOne = true;
                if(dist < closestDist){
                    closestDist = dist;
                    closestColor = VSSRef::Color::BLUE;
                }
                break;
            }
        }
        for(int x = 0; x < frame.robots_yellow_size(); x++){
            float dist = Utils::distance(vector2d(frame.robots_blue(x).x(), frame.robots_blue(x).y()), ballPos);
            if(dist <= playerMaxDistToBall){
                haveAtLeastOne = true;
                if(dist < closestDist){
                    closestDist = dist;
                    closestColor = VSSRef::Color::YELLOW;
                }
                break;
            }
        }

        if(haveAtLeastOne){
            _ballStuckTimer.stop();
            char str[1024];
            snprintf(str, 1023, "%.2f", _ballStuckTimer.timesec());
            RefereeView::drawText(vector2d(frame.ball().x() * 1000.0, frame.ball().y() * 1000.0), str);
            if(_ballStuckTimer.timesec() >= BALL_STUCK_TIMEOUT){
                VSSRef::Quadrant foulQuadrant = Utils::getBallQuadrant(ballPos);
                if(foulQuadrant == VSSRef::Quadrant::NO_QUADRANT){
                    // If foul occurs at goal
                    VSSRef::Color foulKicker = (closestColor == VSSRef::Color::BLUE) ? VSSRef::Color::YELLOW : VSSRef::Color::BLUE;
                    setTeamFoul(VSSRef::Foul::PENALTY_KICK, foulKicker, Utils::getBallQuadrant(ballPos));
                }
                else{
                    // If occurs at field
                    setTeamFoul(VSSRef::Foul::FREE_BALL, VSSRef::Color::BLUE, Utils::getBallQuadrant(ballPos));
                }
                _ballStuckTimer.start();
                return true;
            }
        }
        else{
            _ballStuckTimer.start();
        }
    }

    return false;
}
