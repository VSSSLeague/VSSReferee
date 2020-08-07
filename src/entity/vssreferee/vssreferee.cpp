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
    _placementTimer.start();
    _gameTimer.start();
    startedGKTimer = false;

    timePassed = 0;
    alreadySet = false;
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

        /// TODO HERE
        /// Receive and process VSSVisionClient informations to check fouls

/*
        // By default, send a FREE_BALL for team BLUE (foul packet test)
        _refereeCommand.set_foul(VSSRef::Foul::FREE_BALL);
        _refereeCommand.set_foulquadrant(VSSRef::Quadrant::QUADRANT_1);
        _refereeCommand.set_teamcolor(VSSRef::Color::BLUE);

        if(isConnected()){
            sendPacket(_refereeCommand);
            RefereeView::addRefereeCommand(getFoulNameById(_refereeCommand.foul()));
            emit setFoul(_refereeCommand.foul());
        }
*/
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

// Fouls detection
bool VSSReferee::checkTwoPlayersInsideGoalAreaWithBall(){
    fira_message::Frame frame = _visionClient->getDetectionData();

    // Checking for blue team
    int bluePlayersAtGoal = 0;
    bool ballIsAtBlueGoal = isInsideGoalArea(VSSRef::Color::BLUE, vector2d(frame.ball().x(), frame.ball().y()));
    for(int x = 0; x < frame.robots_blue().size(); x++){
        if(isInsideGoalArea(VSSRef::Color::BLUE, vector2d(frame.robots_blue(x).x(), frame.robots_blue(x).y())))
            bluePlayersAtGoal++;
    }
    if(bluePlayersAtGoal >= 2 && ballIsAtBlueGoal) return true;

    // Checking for yellow team
    int yellowPlayersAtGoal = 0;
    bool ballIsAtYellowGoal = isInsideGoalArea(VSSRef::Color::YELLOW, vector2d(frame.ball().x(), frame.ball().y()));
    for(int x = 0; x < frame.robots_yellow().size(); x++){
        if(isInsideGoalArea(VSSRef::Color::YELLOW, vector2d(frame.robots_yellow(x).x(), frame.robots_yellow(x).y())))
            yellowPlayersAtGoal++;
    }
    if(yellowPlayersAtGoal >= 2 && ballIsAtYellowGoal) return true;

    return false;
}

bool VSSReferee::checkTwoPlayersAttackingAtGoalArea(){
    fira_message::Frame frame = _visionClient->getDetectionData();

    // Checking for blue team
    int enemyPlayersAtBlueGoal = 0;
    bool ballIsAtBlueGoal = isInsideGoalArea(VSSRef::Color::BLUE, vector2d(frame.ball().x(), frame.ball().y()));
    for(int x = 0; x < frame.robots_yellow().size(); x++){
        if(isInsideGoalArea(VSSRef::Color::BLUE, vector2d(frame.robots_yellow(x).x(), frame.robots_yellow(x).y())))
            enemyPlayersAtBlueGoal++;
    }
    if(enemyPlayersAtBlueGoal >= 2 && ballIsAtBlueGoal) return true;

    // Checking for yellow team
    int enemyPlayersAtYellowGoal = 0;
    bool ballIsAtYellowGoal = isInsideGoalArea(VSSRef::Color::YELLOW, vector2d(frame.ball().x(), frame.ball().y()));
    for(int x = 0; x < frame.robots_blue().size(); x++){
        if(isInsideGoalArea(VSSRef::Color::YELLOW, vector2d(frame.robots_blue(x).x(), frame.robots_blue(x).y())))
            enemyPlayersAtYellowGoal++;
    }
    if(enemyPlayersAtYellowGoal >= 2 && ballIsAtYellowGoal) return true;

    return false;
}

bool VSSReferee::checkGKTakeoutTimeout(){
    fira_message::Frame frame = _visionClient->getDetectionData();

    int playersAtBlueGoal = 0;
    bool isBallInsideBlueGoal = isInsideGoalArea(VSSRef::Color::BLUE, vector2d(frame.ball().x(), frame.ball().y()));
    bool isBallInsideYellowGoal = isInsideGoalArea(VSSRef::Color::YELLOW, vector2d(frame.ball().x(), frame.ball().y()));

    // Checking for blue team if ball is inside their goal
    if(isBallInsideBlueGoal){
        for(int x = 0; x < frame.robots_yellow().size(); x++){
            if(isInsideGoalArea(VSSRef::Color::BLUE, vector2d(frame.robots_yellow(x).x(), frame.robots_yellow(x).y())))
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
                if(_gkTimer.timesec() >= GK_TIME_TAKEOUT)
                    return true;
            }
        }
    }
    // Checking for yellow team if ball is inside their goal
    else if(isBallInsideYellowGoal){
        int playersAtYellowGoal = 0;
        for(int x = 0; x < frame.robots_blue().size(); x++){
            if(isInsideGoalArea(VSSRef::Color::YELLOW, vector2d(frame.robots_blue(x).x(), frame.robots_blue(x).y())))
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
                if(_gkTimer.timesec() >= GK_TIME_TAKEOUT)
                    return true;
            }
        }
    }
    else{
        startedGKTimer = false;
    }

    return false;
}

bool VSSReferee::isInsideGoalArea(VSSRef::Color teamColor, vector2d pos){
    float goal_x = (FieldConstantsVSS::kFieldLength/2.0 - FieldConstantsVSS::kDefenseRadius) / 1000.0;
    float goal_y = (FieldConstantsVSS::kDefenseStretch / 2.0) / 1000.0;

    if(teamColor == VSSRef::Color::BLUE){
        if(RefereeView::getBlueIsLeftSide()){
            if(pos.x < -goal_x && abs(pos.y) < goal_y)
                return true;
        }
        else{
            if(pos.x > goal_x && abs(pos.y) < goal_y)
                return true;
        }
    }
    else if(teamColor == VSSRef::Color::YELLOW){
        if(RefereeView::getBlueIsLeftSide()){
            if(pos.x > goal_x && abs(pos.y) < goal_y)
                return true;
        }
        else{
            if(pos.x < -goal_x && abs(pos.y) < goal_y)
                return true;
        }
    }

    return false;
}
