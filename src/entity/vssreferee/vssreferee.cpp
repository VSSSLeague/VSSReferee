#include "vssreferee.h"
#include <src/entity/refereeview/refereeview.h>
#include <src/entity/refereeview/soccerview/util/field_default_constants.h>

QString VSSReferee::name(){
    return "VSSReferee";
}

VSSReferee::VSSReferee(VSSVisionClient *visionClient, const QString& refereeAddress, int refereePort, Constants* constants)
{
    _visionClient   = visionClient;
    _refereeAddress = refereeAddress;
    _refereePort    = refereePort;
    _constants      = constants;

    connect(refereeAddress, refereePort);

    // Reset vars initially
    _placementIsSet   = false;
    _blueSent         = false;
    _yellowSent       = false;
    _stopEnabled      = false;
    timePassed        = 0;
    startedGKTimer    = false;
    startedStuckTimer = false;

    // Start timers
    _placementTimer.start();
    _gameTimer.start();
    _gkTimer.start();
    _ballStuckTimer.start();
    _ballVelTimer.start();
    _stopTimer.start();
}

VSSReferee::~VSSReferee(){

}

void VSSReferee::initialization(){
    std::cout << "[VSSReferee] Thread started" << std::endl;
}

void VSSReferee::loop(){
    // Checking if half passed, reseting the time count
    _gameTimer.stop();
    if((_gameTimer.timesec() + timePassed) > getConstants()->getGameHalfTime()){
        RefereeView::addRefereeWarning("Half passed!");
        _gameTimer.start();
        timePassed = 0;
        setTeamFoul(VSSRef::Foul::KICKOFF, VSSRef::Color::NONE, VSSRef::Quadrant::NO_QUADRANT);
        emit halfPassed();
    }

    // Checking timer overextended if a foul is set
    if(_placementIsSet){
        // Saves game passed time
        _gameTimer.stop();
        timePassed += _gameTimer.timesec();
        _gameTimer.start();

        _placementTimer.stop();
        RefereeView::setCurrentTime(getConstants()->getPlacementWaitTime() - _placementTimer.timesec());
        if((_placementTimer.timensec() / 1E9) >= getConstants()->getPlacementWaitTime() && (!_blueSent || !_yellowSent)){
            // If enters here, one of the teams didn't placed as required in the determined time
            if(!_blueSent){
                RefereeView::addRefereeWarning("Blue Team hasn't placed");
                std::cout << "[VSSReferee] Team BLUE hasn't sent the placement command." << std::endl;
                // place an automatic position here
            }
            if(!_yellowSent){
                RefereeView::addRefereeWarning("Yellow Team hasn't placed");
                std::cout << "[VSSReferee] Team YELLOW hasn't sent the placement command." << std::endl;
                // place an automatic position here
            }
            _placementMutex.lock();
            _blueSent = false;
            _yellowSent = false;
            _placementIsSet = false;
            _placementMutex.unlock();

            // Set stop and reset its timer
            _stopEnabled = true;
            _stopTimer.start();
            setTeamFoul(VSSRef::Foul::STOP, VSSRef::Color::NONE, VSSRef::Quadrant::NO_QUADRANT, true);

            // Stop replacer wait
            emit stopReplacerWaiting();
        }
        else if(_blueSent && _yellowSent){
            _placementMutex.lock();
            _blueSent = false;
            _yellowSent = false;
            _placementIsSet = false;
            _placementMutex.unlock();

            // Set stop and reset its timer
            _stopEnabled = true;
            _stopTimer.start();
            setTeamFoul(VSSRef::Foul::STOP, VSSRef::Color::NONE, VSSRef::Quadrant::NO_QUADRANT, true);

            // Stop replacer wait
            emit stopReplacerWaiting();
        }
    }
    else if(_stopEnabled){
        // Saves game passed time
        _gameTimer.stop();
        timePassed += _gameTimer.timesec();
        _gameTimer.start();

        // Taking stop time
        _stopTimer.stop();
        RefereeView::setCurrentTime(getConstants()->getStopWaitTime() - _stopTimer.timesec());

        // Checking if timer ends
        if((_stopTimer.timensec() / 1E9) >= getConstants()->getStopWaitTime()){
            _stopEnabled = false;
            RefereeView::addRefereeWarning("Stop time ended");
        }
    }
    else{
        // Updating game left time
        _gameTimer.stop();
        RefereeView::setCurrentTime(getConstants()->getGameHalfTime() - (_gameTimer.timesec() + timePassed));
        RefereeView::setRefereeCommand("GAME_ON");

        checkTwoPlayersInsideGoalAreaWithBall();
        checkTwoPlayersAttackingAtGoalArea();
        checkBallStucked();
        checkGKTakeoutTimeout();
        checkGoal();
    }
}

void VSSReferee::finalization(){
    disconnect();
    std::cout << "[VSSReferee] Thread ended" << std::endl;
}

void VSSReferee::sendPacket(VSSRef::ref_to_team::VSSRef_Command command, bool isStop){
    std::string msg;
    command.SerializeToString(&msg);

    if(_socket.write(msg.c_str(), msg.length()) == -1){
        std::cout << "[VSSReferee] Failed to write to socket: " << _socket.errorString().toStdString() << std::endl;
    }
    else{
        if(!isStop){
            _placementMutex.lock();
            _placementIsSet = true;
            _blueSent = false;
            _yellowSent = false;
            _placementTimer.start();
            _placementMutex.unlock();
        }
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
        case VSSRef::Foul::KICKOFF:      return "KICKOFF";
        case VSSRef::Foul::STOP:         return "STOP";
        default:                         return "FOUL NOT IDENTIFIED";
    }
}

void VSSReferee::setTeamFoul(VSSRef::Foul foul, VSSRef::Color forTeam, VSSRef::Quadrant foulQuadrant, bool isStop){
    _refereeCommand.set_foul(foul);
    _refereeCommand.set_teamcolor(forTeam);
    _refereeCommand.set_foulquadrant(foulQuadrant);

    if(isConnected()){
        std::cout << "[VSSReferee] Command from referee: " << getFoulNameById(foul).toStdString() << "\n";
        sendPacket(_refereeCommand, isStop);
        RefereeView::addRefereeCommand(getFoulNameById(_refereeCommand.foul()));
        resetFoulTimers();
        if(!isStop) emit setFoul(foul, forTeam, foulQuadrant);
    }
}

void VSSReferee::resetFoulTimers(){
    _placementTimer.start();
    _gkTimer.start();
    _ballStuckTimer.start();
    _ballVelTimer.start();
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

    bool isBallInsideBlueGoal = Utils::isInsideGoalArea(VSSRef::Color::BLUE, vector2d(frame.ball().x(), frame.ball().y()));
    bool isBallInsideYellowGoal = Utils::isInsideGoalArea(VSSRef::Color::YELLOW, vector2d(frame.ball().x(), frame.ball().y()));

    // Checking for blue team if ball is inside their goal
    if(isBallInsideBlueGoal){
        int opPlayersAtBlueGoal = 0;
        int alliePlayersAtBlueGoal = 0;
        for(int x = 0; x < frame.robots_yellow().size(); x++){
            if(Utils::isInsideGoalArea(VSSRef::Color::BLUE, vector2d(frame.robots_yellow(x).x(), frame.robots_yellow(x).y())))
                opPlayersAtBlueGoal++;
        }
        for(int x = 0; x < frame.robots_blue().size(); x++){
            if(Utils::isInsideGoalArea(VSSRef::Color::BLUE, vector2d(frame.robots_blue(x).x(), frame.robots_blue(x).y())))
                alliePlayersAtBlueGoal++;
        }
        // If have no contestation and only GK is at goal area
        if(opPlayersAtBlueGoal == 0 && alliePlayersAtBlueGoal == 1){
            // Check if timer is started, if don't, start it and check after
            if(!startedGKTimer){
                _gkTimer.start();
                startedGKTimer = true;
            }
            else{
                // Debug to GUI
                char str[1024];
                snprintf(str, 1023, "%.2f", _gkTimer.timesec());
                RefereeView::drawText(vector2d(frame.ball().x() * 1000.0, frame.ball().y() * 1000.0), str);

                // Check timer
                _gkTimer.stop();
                if(_gkTimer.timesec() >= getConstants()->getGKTakeoutTime()){
                    setTeamFoul(VSSRef::Foul::PENALTY_KICK, VSSRef::Color::YELLOW);
                    startedGKTimer = false;
                    return true;
                }
            }
        }
        else{
            startedGKTimer = false;
        }
    }
    // Checking for yellow team if ball is inside their goal
    else if(isBallInsideYellowGoal){
        int opPlayersAtYellowGoal = 0;
        int alliePlayersAtYellowGoal = 0;
        for(int x = 0; x < frame.robots_blue().size(); x++){
            if(Utils::isInsideGoalArea(VSSRef::Color::YELLOW, vector2d(frame.robots_blue(x).x(), frame.robots_blue(x).y())))
                opPlayersAtYellowGoal++;
        }
        for(int x = 0; x < frame.robots_yellow().size(); x++){
            if(Utils::isInsideGoalArea(VSSRef::Color::YELLOW, vector2d(frame.robots_yellow(x).x(), frame.robots_yellow(x).y())))
                alliePlayersAtYellowGoal++;
        }

        // If have no contestation and only GK is at goal area
        if(opPlayersAtYellowGoal == 0 && alliePlayersAtYellowGoal == 1){
            // Check if timer is started, if don't, start it and check after
            if(!startedGKTimer){
                _gkTimer.start();
                startedGKTimer = true;
            }
            else{
                // Debug to GUI
                char str[1024];
                snprintf(str, 1023, "%.2f", _gkTimer.timesec());
                RefereeView::drawText(vector2d(frame.ball().x() * 1000.0, frame.ball().y() * 1000.0), str);

                // Check timer
                _gkTimer.stop();
                if(_gkTimer.timesec() >= getConstants()->getGKTakeoutTime()){
                    setTeamFoul(VSSRef::Foul::PENALTY_KICK, VSSRef::Color::BLUE);
                    startedGKTimer = false;
                    return true;
                }
            }
        }else{
            startedGKTimer = false;
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

    if(ballVelocity > getConstants()->getBallMinimumVelocity() || !startedStuckTimer){
        if(!startedStuckTimer) startedStuckTimer = true;
        _ballStuckTimer.start();
    }
    else{
        VSSRef::Quadrant foulQuadrant = Utils::getBallQuadrant(ballPos);
        // If the ball isn't at goal
        if(foulQuadrant != VSSRef::Quadrant::NO_QUADRANT){
            // Showing timer at GUI
            char str[1024];
            snprintf(str, 1023, "%.2f", _ballStuckTimer.timesec());
            RefereeView::drawText(vector2d(frame.ball().x() * 1000.0, frame.ball().y() * 1000.0), str);

            // Check timer
            _ballStuckTimer.stop();
            if(_ballStuckTimer.timesec() >= getConstants()->getBallStuckTime()){
                setTeamFoul(VSSRef::Foul::FREE_BALL, VSSRef::Color::NONE, Utils::getBallQuadrant(ballPos));
                startedStuckTimer = false;
                return true;
            }
        }
    }

    return false;
}

bool VSSReferee::checkGoal(){
    fira_message::Frame frame = _visionClient->getDetectionData();
    vector2d ballPos = vector2d(frame.ball().x(), frame.ball().y());

    if(Utils::isBallInsideGoal(VSSRef::BLUE, ballPos)){
        setTeamFoul(VSSRef::KICKOFF, VSSRef::Color::BLUE);
        emit goalMarked(VSSRef::YELLOW);
        return true;
    }
    else if(Utils::isBallInsideGoal(VSSRef::YELLOW, ballPos)){
        setTeamFoul(VSSRef::KICKOFF, VSSRef::Color::YELLOW);
        emit goalMarked(VSSRef::BLUE);
        return true;
    }
    else{
        return false;
    }
}

Constants* VSSReferee::getConstants(){
    if(_constants == NULL){
        std::cout << "[ERROR] Referee is requesting constants, but it's NULL\n";
        return NULL;
    }

    return _constants;
}
