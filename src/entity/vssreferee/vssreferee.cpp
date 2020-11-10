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
    _manualStop       = false;
    _manualGameOn     = false;
    timePassed        = 0;
    startedGKTimer    = false;
    startedStuckTimer = false;
    _startedPenaltyTimer = false;
    startedDisputateTimer = false;
    _gameHalf         = VSSRef::Half::FIRST_HALF;

    // Start timers
    _placementTimer.start();
    _gameTimer.start();
    _gkTimer.start();
    _ballStuckTimer.start();
    _ballVelTimer.start();
    _stopTimer.start();

    // Allocating memory to time and timers (for gk checking)
    time = static_cast<float**>(malloc(2 * sizeof(float *)));
    for(int x = 0; x < 2; x++){
        time[x] = static_cast<float*>(malloc(static_cast<unsigned int>(getConstants()->getQtPlayers()) * sizeof(float)));
    }

    timers = static_cast<Timer**>(malloc(2 * sizeof(Timer *)));
    for(int x = 0; x < 2; x++){
        timers[x] = static_cast<Timer*>(malloc(static_cast<unsigned int>(getConstants()->getQtPlayers()) * sizeof(Timer)));
    }

    // Starting gk checking values and timers
    for(int x = 0; x < 2; x++){
        for(int y = 0; y < getConstants()->getQtPlayers(); y++){
            timers[x][y].start();
            time[x][y] = 0.0;
        }
    }
}

VSSReferee::~VSSReferee(){
    for(int x = 0; x < 2; x++){
        free(time[x]);
        free(timers[x]);
    }

    free(time);
    free(timers);
}

void VSSReferee::initialization(){
    std::cout << "[VSSReferee] Thread started" << std::endl;

    setGameStartStop();
}

void VSSReferee::loop(){
    // Checking if half passed, reseting the time count
    _gameTimer.stop();
    if((_gameTimer.timesec() + timePassed) > static_cast<double>(getConstants()->getGameHalfTime())){

        // Swapping halfs
        if(_gameHalf == VSSRef::Half::FIRST_HALF){
            _gameHalf = VSSRef::Half::SECOND_HALF;
        }
        else{
            _gameHalf = VSSRef::Half::FIRST_HALF;
        }

        _gameTimer.start();
        timePassed = 0;
        setTeamFoul(VSSRef::Foul::KICKOFF, VSSRef::Color::NONE, VSSRef::Quadrant::NO_QUADRANT);
        emit halfPassed();
    }

    // Checking timer overextended if a foul is set
    if(_placementIsSet){
        // Saves game passed time
        _gameTimer.stop();
        timePassed += static_cast<int>(_gameTimer.timesec());
        _gameTimer.start();

        _placementTimer.stop();
        RefereeView::setCurrentTime(static_cast<int>(static_cast<double>(getConstants()->getPlacementWaitTime()) - _placementTimer.timesec()));
        if((_placementTimer.timensec() / 1E9) >= static_cast<double>(getConstants()->getPlacementWaitTime()) && (!_blueSent || !_yellowSent)){
            // If enters here, one of the teams didn't placed as required in the determined time
            if(!_blueSent){
                std::cout << "[VSSReferee] Team BLUE hasn't sent the placement command." << std::endl;
                // place an automatic position here
            }
            if(!_yellowSent){
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
        // Reset foul timers
        resetFoulTimers();

        // Saves game passed time
        _gameTimer.stop();
        timePassed += static_cast<int>(_gameTimer.timesec());
        _gameTimer.start();

        // Taking stop time
        _stopTimer.stop();

        float stopTime;
        if(_manualStop)
            if(_gameStartStop)  stopTime = 120;
            else                stopTime = 600;
        else                    stopTime = getConstants()->getStopWaitTime();

        RefereeView::setCurrentTime(static_cast<int>(static_cast<double>(stopTime) - _stopTimer.timesec()));

        // Checking if timer ends
        if((_stopTimer.timensec() / 1E9) >= static_cast<double>(stopTime) || _manualGameOn){
            _stopEnabled = false;
            _manualStop  = false;
            _manualGameOn = false;
            if(_gameStartStop){
                _gameStartStop = false;
                setTeamFoul(VSSRef::Foul::KICKOFF, VSSRef::Color::NONE, VSSRef::Quadrant::NO_QUADRANT);
            }
            else{
                setTeamFoul(VSSRef::Foul::GAME_ON, VSSRef::Color::NONE, VSSRef::Quadrant::NO_QUADRANT, true);
            }
        }
    }
    else{
        // Updating game left time
        _gameTimer.stop();
        RefereeView::setCurrentTime(static_cast<int>(static_cast<double>(getConstants()->getGameHalfTime()) - (_gameTimer.timesec() + timePassed)));
        RefereeView::setRefereeCommand("GAME_ON");

        checkTwoPlayersInsideGoalAreaWithBall();
        checkTwoPlayersAttackingAtGoalArea();
        checkBallStucked();
        checkGKTakeoutTimeout();
        checkGoal();
        updateGoalieTimers();
    }
}

void VSSReferee::finalization(){
    disconnect();
    std::cout << "[VSSReferee] Thread ended" << std::endl;
}

void VSSReferee::sendPacket(VSSRef::ref_to_team::VSSRef_Command command, bool isStop){
    std::string msg;
    command.SerializeToString(&msg);

    if(_socket.write(msg.c_str(), static_cast<qint64>(msg.length())) == -1){
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

    _socket.connectToHost(refereeAddress, static_cast<quint16>(refereePort), QIODevice::WriteOnly, QAbstractSocket::IPv4Protocol);

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
        case VSSRef::Foul::GAME_ON:      return "GAME_ON";

        default:                         return "FOUL NOT IDENTIFIED";
    }
}

void VSSReferee::setTeamFoul(VSSRef::Foul foul, VSSRef::Color forTeam, VSSRef::Quadrant foulQuadrant, bool isStop){
    _refereeCommand.set_foul(foul);
    _refereeCommand.set_teamcolor(forTeam);
    _refereeCommand.set_foulquadrant(foulQuadrant);
    _refereeCommand.set_gamehalf(_gameHalf);

    // Setting timestamp
    _gameTimer.stop();
    _refereeCommand.set_timestamp(_gameTimer.timesec() + timePassed);

    // Setting half

    if(isConnected()){
        std::cout << "[VSSReferee] Command from referee: " << getFoulNameById(foul).toStdString() << "\n";
        if(!isStop) emit setFoul(foul, forTeam, foulQuadrant);
        sendPacket(_refereeCommand, isStop);
        RefereeView::addRefereeCommand(getFoulNameById(_refereeCommand.foul()));
        resetFoulTimers();
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
        if(!_startedPenaltyTimer){
            _startedPenaltyTimer = true;
            _penaltyTimer.start();
        }
        else{
            _penaltyTimer.stop();
            if(_penaltyTimer.timesec() >= static_cast<double>(getConstants()->getPenaltyTime())){
                _startedPenaltyTimer = false;
                setTeamFoul(VSSRef::Foul::PENALTY_KICK, VSSRef::Color::YELLOW);
            }
        }
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
        if(!_startedPenaltyTimer){
            _startedPenaltyTimer = true;
            _penaltyTimer.start();
        }
        else{
            _penaltyTimer.stop();
            if(_penaltyTimer.timesec() >= static_cast<double>(getConstants()->getPenaltyTime())){
                _startedPenaltyTimer = false;
                setTeamFoul(VSSRef::Foul::PENALTY_KICK, VSSRef::Color::BLUE);
            }
        }
        return true;
    }

    _startedPenaltyTimer = false;
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
                if(_gkTimer.timesec() >= static_cast<double>(getConstants()->getGKTakeoutTime())){
                    setTeamFoul(VSSRef::Foul::PENALTY_KICK, VSSRef::Color::YELLOW);
                    startedGKTimer = false;
                    return true;
                }
            }

            startedDisputateTimer = false;
        }
        else{
            startedGKTimer = false;
            // Disputating ball
            if(opPlayersAtBlueGoal == 1 && alliePlayersAtBlueGoal == 1){
                if(getBallVelocity() < getConstants()->getBallMinimumVelocity()){
                    if(!startedDisputateTimer){
                        startedDisputateTimer = true;
                        _disputateTimer.start();
                    }
                    else{
                        _disputateTimer.stop();
                        if(_disputateTimer.timesec() >= 10){
                            float distanceBlue = 999.0f;
                            float distanceYellow = 999.0f;

                            for(int x = 0; x < frame.robots_blue().size(); x++){
                                if(Utils::isInsideGoalArea(VSSRef::Color::BLUE, vector2d(frame.robots_blue(x).x(), frame.robots_blue(x).y()))){
                                    distanceBlue = Utils::distance(vector2d(frame.robots_blue(x).x(), frame.robots_blue(x).y()), vector2d(frame.ball().x(), frame.ball().y()));
                                }
                            }

                            for(int x = 0; x < frame.robots_yellow().size(); x++){
                                if(Utils::isInsideGoalArea(VSSRef::Color::BLUE, vector2d(frame.robots_yellow(x).x(), frame.robots_yellow(x).y()))){
                                    distanceYellow = Utils::distance(vector2d(frame.robots_yellow(x).x(), frame.robots_yellow(x).y()), vector2d(frame.ball().x(), frame.ball().y()));
                                }
                            }

                            if((distanceBlue <= ((getConstants()->getRobotLength() * sqrt(2)) + getConstants()->getBallRadius())) && (distanceYellow <= ((getConstants()->getRobotLength() * sqrt(2)) + getConstants()->getBallRadius()))){
                                setTeamFoul(VSSRef::Foul::GOAL_KICK, VSSRef::Color::BLUE);
                                startedDisputateTimer = false;
                                return true;
                            }else{
                                setTeamFoul(VSSRef::Foul::PENALTY_KICK, VSSRef::Color::YELLOW);
                                startedDisputateTimer = false;
                                return true;
                            }
                        }
                    }
                }
                else{
                    startedDisputateTimer = false;
                }
            }
            else{
                startedDisputateTimer = false;
            }
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
                if(_gkTimer.timesec() >= static_cast<double>(getConstants()->getGKTakeoutTime())){
                    setTeamFoul(VSSRef::Foul::PENALTY_KICK, VSSRef::Color::BLUE);
                    startedGKTimer = false;
                    return true;
                }
            }

            startedDisputateTimer = false;
        }else{
            startedGKTimer = false;
            // Disputating ball
            if(opPlayersAtYellowGoal == 1 && alliePlayersAtYellowGoal == 1){
                if(getBallVelocity() < getConstants()->getBallMinimumVelocity()){
                    if(!startedDisputateTimer){
                        startedDisputateTimer = true;
                        _disputateTimer.start();
                    }
                    else{
                        _disputateTimer.stop();
                        if(_disputateTimer.timesec() >= 10){
                            float distanceYellow = 999.0f;
                            for(int x = 0; x < frame.robots_yellow().size(); x++){
                                if(Utils::isInsideGoalArea(VSSRef::Color::YELLOW, vector2d(frame.robots_yellow(x).x(), frame.robots_yellow(x).y()))){
                                    distanceYellow = Utils::distance(vector2d(frame.robots_yellow(x).x(), frame.robots_yellow(x).y()), vector2d(frame.ball().x(), frame.ball().y()));
                                }
                            }

                            float distanceBlue = 999.0f;
                            for(int x = 0; x < frame.robots_blue().size(); x++){
                                if(Utils::isInsideGoalArea(VSSRef::Color::YELLOW, vector2d(frame.robots_blue(x).x(), frame.robots_blue(x).y()))){
                                    distanceBlue = Utils::distance(vector2d(frame.robots_blue(x).x(), frame.robots_blue(x).y()), vector2d(frame.ball().x(), frame.ball().y()));
                                }
                            }

                            if((distanceYellow <= ((getConstants()->getRobotLength() * sqrt(2)) + getConstants()->getBallRadius())) && (distanceBlue <= ((getConstants()->getRobotLength() * sqrt(2)) + getConstants()->getBallRadius()))){
                                setTeamFoul(VSSRef::Foul::GOAL_KICK, VSSRef::Color::YELLOW);
                            }else{
                                setTeamFoul(VSSRef::Foul::PENALTY_KICK, VSSRef::Color::BLUE);
                            }

                            startedDisputateTimer = false;
                        }
                    }
                }
                else{
                    startedDisputateTimer = false;
                }
            }else{
                startedDisputateTimer = false;
            }
        }
    }
    else{
        startedDisputateTimer = false;
        startedGKTimer = false;
    }

    return false;
}

float VSSReferee::getBallVelocity(){
    fira_message::Frame frame = _visionClient->getDetectionData();
    vector2d ballPos = vector2d(frame.ball().x(), frame.ball().y());

    // Update ball velocity
    _ballVelTimer.stop();
    float vx = static_cast<float>((frame.ball().x() - lastBallPos.x) / _ballVelTimer.timesec());
    float vy = static_cast<float>((frame.ball().y() - lastBallPos.y) / _ballVelTimer.timesec());
    if(isnan(vx) || isnan(vy)) vx = vy = 0.0;
    _ballVelTimer.start();
    lastBallPos = ballPos;

    float ballVelocity = static_cast<float>(sqrt(pow(vx, 2) + pow(vy, 2)));

    return ballVelocity;
}

bool VSSReferee::checkBallStucked(){
    fira_message::Frame frame = _visionClient->getDetectionData();
    vector2d ballPos = vector2d(frame.ball().x(), frame.ball().y());
    float ballVelocity = getBallVelocity();

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
            if(_ballStuckTimer.timesec() >= static_cast<double>(getConstants()->getBallStuckTime())){
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

void VSSReferee::updateGoalieTimers(){
    fira_message::Frame frame = _visionClient->getDetectionData();

    for(int x = 0; x < frame.robots_blue_size(); x++){
        vector2d pos = vector2d(frame.robots_blue(x).x(), frame.robots_blue(x).y());
        if(Utils::isInsideGoalArea(VSSRef::Color::BLUE, pos)){
            timers[VSSRef::Color::BLUE][x].stop();
            // If passed more than 1s, this player appearly isn't the gk formerly, so reset to count in the next it
            if(timers[VSSRef::Color::BLUE][x].timesec() >= 1.0){
                timers[VSSRef::Color::BLUE][x].start();
            }
            // Else, if passed less than 1s, probably this player is at goal area and isn't noise
            else{
                timers[VSSRef::Color::BLUE][x].stop();
                time[VSSRef::Color::BLUE][x] += static_cast<float>(timers[VSSRef::Color::BLUE][x].timesec());
                timers[VSSRef::Color::BLUE][x].start();
            }
        }
    }

    for(int x = 0; x < frame.robots_yellow_size(); x++){
        vector2d pos = vector2d(frame.robots_yellow(x).x(), frame.robots_yellow(x).y());
        if(Utils::isInsideGoalArea(VSSRef::Color::YELLOW, pos)){
            timers[VSSRef::Color::YELLOW][x].stop();
            // If passed more than 1s, this player appearly isn't the gk formerly, so reset to count in the next it
            if(timers[VSSRef::Color::YELLOW][x].timesec() >= 1.0){
                timers[VSSRef::Color::YELLOW][x].start();
            }
            // Else, if passed less than 1s, probably this player is at goal area and isn't noise
            else{
                timers[VSSRef::Color::YELLOW][x].stop();
                time[VSSRef::Color::YELLOW][x] += static_cast<float>(timers[VSSRef::Color::YELLOW][x].timesec());
                timers[VSSRef::Color::YELLOW][x].start();
            }
        }
    }
}

void VSSReferee::requestGoalie(VSSRef::Color team){
    float bestVal = 0.0;
    int bestId = 0;
    for(int x = 0; x < getConstants()->getQtPlayers(); x++){
        if(time[team][x] > bestVal){
            bestVal = time[team][x];
            bestId = x;
        }
    }

    emit sendGoalie(team, bestId);
}

void VSSReferee::takeManualCommand(VSSRef::Foul foul, VSSRef::Color color, VSSRef::Quadrant quadrant){
    if(foul == VSSRef::GAME_ON || foul == VSSRef::STOP){
        if(foul == VSSRef::STOP){
            _placementIsSet = false;
            _stopEnabled = true;
            _manualStop = true;
            _stopTimer.start();

            setTeamFoul(foul, color, quadrant, true);
        }else{
            _manualGameOn = true;
            if(!_gameStartStop) setTeamFoul(foul, color, quadrant, true);
        }
    }
    else{
        _manualStop   = false;
        _manualGameOn = false;
        setTeamFoul(foul, color, quadrant);
    }
}

void VSSReferee::setGameStartStop(){
    _placementIsSet = false;
    _stopEnabled = true;
    _manualStop = true;
    _gameStartStop = true;
    setTeamFoul(VSSRef::Foul::STOP, VSSRef::Color::NONE, VSSRef::Quadrant::NO_QUADRANT, true);
    _stopTimer.start();
}
