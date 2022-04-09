#include "referee.h"

#include <chrono>
#include <random>
#include <thread>

#include <include/vssref_command.pb.h>
#include <src/soccerview/soccerview.h>

Referee::Referee(Vision *vision, Replacer *replacer, SoccerView *soccerView, Constants *constants) : Entity(ENT_REFEREE) {
    // Take vision pointer
    _vision = vision;

    // Take replacer pointer
    _replacer = replacer;

    // Take SoccerView
    _soccerView = soccerView;

    // Take constants
    _constants = constants;

    // Taking network data
    _refereeAddress = getConstants()->refereeAddress();
    _refereePort = getConstants()->refereePort();

    // Connecting referee to replacer
    connect(_replacer, SIGNAL(teamsPlaced()), this, SLOT(teamsPlaced()));
    connect(_replacer, SIGNAL(teamsCollided(VSSRef::Foul, VSSRef::Color, VSSRef::Quadrant, bool)), this, SLOT(processCollision(VSSRef::Foul, VSSRef::Color, VSSRef::Quadrant, bool)));
    connect(this, SIGNAL(sendFoul(VSSRef::Foul, VSSRef::Color, VSSRef::Quadrant)), _replacer, SLOT(takeFoul(VSSRef::Foul, VSSRef::Color, VSSRef::Quadrant)));
    connect(this, SIGNAL(callReplacer(bool, bool)), _replacer, SLOT(placeTeams(bool, bool)), Qt::DirectConnection);
    connect(this, SIGNAL(saveFrame()), _replacer, SLOT(saveFrameAndBall()), Qt::DirectConnection);
    connect(this, SIGNAL(placeFrame()), _replacer, SLOT(placeLastFrameAndBall()), Qt::DirectConnection);
    connect(this, SIGNAL(placeBall(Position, Velocity)), _replacer, SLOT(placeBall(Position, Velocity)), Qt::DirectConnection);
    connect(this, SIGNAL(emitSuggestion(QString, VSSRef::Color, VSSRef::Quadrant)), _soccerView, SLOT(addSuggestion(QString, VSSRef::Color, VSSRef::Quadrant)));

    // Init signal mapper
    _mapper = new QSignalMapper();
}

void Referee::initialization() {
    // Adding checkers
    // Stucked ball
    addChecker(_stuckedBallChecker = new Checker_StuckedBall(_vision, getConstants()), 0);
    connect(_stuckedBallChecker, SIGNAL(sendStuckedTime(float)), this, SLOT(takeStuckedTime(float)));
    _stuckedBallChecker->setIsPenaltyShootout(false, VSSRef::Color::NONE);

    // Two attackers
    addChecker(_twoAtkChecker = new Checker_TwoAttackers(_vision, getConstants()), 1);

    // Two defenders
    addChecker(_twoDefChecker = new Checker_TwoDefenders(_vision, getConstants()), 1);

    // Ball play
    addChecker(_ballPlayChecker = new Checker_BallPlay(_vision, getConstants()), 2);
    connect(_ballPlayChecker, SIGNAL(emitGoal(VSSRef::Color)), _soccerView, SLOT(addGoal(VSSRef::Color)), Qt::DirectConnection);
    connect(_ballPlayChecker, SIGNAL(emitGoal(VSSRef::Color)), this, SLOT(goalOccurred(VSSRef::Color)), Qt::DirectConnection);
    connect(_ballPlayChecker, SIGNAL(emitSuggestion(QString, VSSRef::Color, VSSRef::Quadrant)), _soccerView, SLOT(addSuggestion(QString, VSSRef::Color, VSSRef::Quadrant)));
    _ballPlayChecker->setAtkDefCheckers(_twoAtkChecker, _twoDefChecker);
    _ballPlayChecker->setIsPenaltyShootout(false, VSSRef::Color::NONE);

    // Goalie
    _goalieChecker = new Checker_Goalie(_vision, getConstants());
    connect(_goalieChecker, SIGNAL(updateGoalie(VSSRef::Color, quint8)), _replacer, SLOT(takeGoalie(VSSRef::Color, quint8)));
    addChecker(_goalieChecker, 0);

    // HalfTime
    _halfChecker = new Checker_HalfTime(_vision, getConstants());
    _halfChecker->setReferee(this);
    _halfChecker->setIsPenaltyShootout(false);
    connect(_halfChecker, SIGNAL(halfPassed()), this, SLOT(halfPassed()));
    connect(_soccerView, SIGNAL(addTime(int)), _halfChecker, SLOT(receiveTime(int)), Qt::DirectConnection);

    // Set default initial state
    _gameHalf = VSSRef::NO_HALF;
    _isStopped = false;
    _teamsPlaced = false;
    _isToPlaceOutside = false;
    _isEndGame = false;
    _isPenaltyShootout = false;
    _placedLast = true;
    _forceDefault = false;

    // Take first kickoff team
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 mt_rand(seed);
    _halfKickoff = VSSRef::Color(mt_rand() % 2); // take random half kickoff (initially)

    // Connect
    connectClient();

    // Debug network info
    std::cout << Text::blue("[REFEREE] ", true) + Text::bold("Module started at address '" + _refereeAddress.toStdString() + "' and port '" + std::to_string(_refereePort) + "'.") + '\n';

    // Call check collision
    checkIfTeamsAreColliding();

    // Call halfPassed (start game)
    halfPassed();
}

void Referee::loop() {
    // Run half checker
    _halfChecker->run();

    // Send timestamp
    emit sendTimestamp((_halfChecker->isOvertime() ? getConstants()->overtimeHalfTime() : getConstants()->halfTime()), _halfChecker->getTimeStamp(), _gameHalf, _isEndGame);

    // Game halted just return
    if(_gameHalted) {
        emit placeBall(_lastBallPosition, Velocity(true, 0.0, 0.0));
        return ;
    }

    // In long stop, check if timer has passed the defined time
    if(_longStop) {
        _transitionTimer.stop();

        if(_transitionTimer.getSeconds() >= (60 * getConstants()->transitionTime())) {
            resetTransitionVars();

            // Check if last foul is stop
            if(_lastFoul == VSSRef::Foul::STOP) {
                // Set isStopped as true (avoid double stop)
                _isStopped = true;
            }

            sendPenaltiesToNetwork();
        }
        return ;
    }

    // If game is on, run all checks
    if(_lastFoul == VSSRef::Foul::GAME_ON) {
        _checkerMutex.lock();

        // Take list of registered priorities
        QList<int> priorityKeys = _checkers.keys();
        QList<int>::iterator it;

        // Sort from higher to lower priority
        std::sort(priorityKeys.begin(), priorityKeys.end(), std::greater<int>());

        for(it = priorityKeys.begin(); it != priorityKeys.end(); it++) {
            // Take fouls with priority (*it)
            QVector<Checker*> *checkers = _checkers.value((*it));

            for(int i = 0; i < checkers->size(); i++) {
                // Take foul
                Checker *atChecker = checkers->at(i);

                // Run it
                atChecker->run();
            }
        }

        // Reset transition management vars
        resetTransitionVars();

        _checkerMutex.unlock();
    }
    // Else if game is not on, wait, go to stop and set game on again
    else {
        if(!_isStopped) {
            if(!_resetedTimer) {
                _transitionTimer.start();
                _resetedTimer = true;
            }
            // Stop timer
            _transitionTimer.stop();

            _transitionMutex.lock();
            bool teamsPlaced = _teamsPlaced;
            _transitionMutex.unlock();

            // Transition time
            float transitionTime = (_goalOccurred) ? 2.0 * getConstants()->transitionTime() : getConstants()->transitionTime();

            // Check if passed transition time
            if(_transitionTimer.getSeconds() >= transitionTime/* || (teamsPlaced && _transitionTimer.getSeconds() >= (getConstants()->transitionTime() / 2.0))*/) {
                // Set control vars
                _isStopped = true;
                _resetedTimer = false;

                // Call replacer (place teams)
                emit callReplacer(_forceDefault, _isToPlaceOutside);
                _forceDefault = false;
                _isToPlaceOutside = false;

                // Update sent foul to STOP
                updatePenaltiesInfo(VSSRef::Foul::STOP, VSSRef::Color::NONE, VSSRef::Quadrant::NO_QUADRANT);
                sendPenaltiesToNetwork();
            }
        }
        else {
            if(!_resetedTimer) {
                _transitionTimer.start();
                _resetedTimer = true;
            }

            _goalOccurred = false;

            // Stop timer
            _transitionTimer.stop();

            // Check if passed transition time
            if(_transitionTimer.getSeconds() >= getConstants()->transitionTime()) {
                // Update sent foul to GAME_ON
                updatePenaltiesInfo(VSSRef::Foul::GAME_ON, VSSRef::Color::NONE, VSSRef::Quadrant::NO_QUADRANT);
                sendPenaltiesToNetwork();
            }
        }
    }
}

void Referee::finalization() {
    // Delete fouls
    deleteCheckers();

    // Disconnect client
    disconnectClient();

    std::cout << Text::blue("[REFEREE] ", true) + Text::bold("Module finished.") + '\n';
}

bool Referee::isGameOn() {
    return (_lastFoul == VSSRef::Foul::GAME_ON && !_gameHalted && !_longStop && !_isPenaltyShootout);
}

void Referee::connectClient() {
    // Create socket pointer
    _refereeClient = new QUdpSocket();

    // Close if already opened
    if(_refereeClient->isOpen()) {
        _refereeClient->close();
    }

    // Connect to referee address and port
    _refereeClient->connectToHost(_refereeAddress, _refereePort, QIODevice::WriteOnly, QAbstractSocket::IPv4Protocol);
}

void Referee::disconnectClient() {
    // Close referee client
    if(_refereeClient->isOpen()) {
        _refereeClient->close();
    }

    // Delete client
    delete _refereeClient;
}

void Referee::addChecker(Checker *checker, int priority) {
    // Check if priority already exists in hash
    if(!_checkers.contains(priority)) {
        // Create it
        _checkers.insert(priority, new QVector<Checker*>());
    }

    // Taking Foul vector
    QVector<Checker*> *checkerVector = _checkers.value(priority);

    // Check if foul is already added
    if(!checkerVector->contains(checker)) {
        // Connect in map
        connect(checker, SIGNAL(foulOccured()), _mapper, SLOT(map()), Qt::UniqueConnection);
        _mapper->setMapping(checker, checker);
        connect(_mapper, SIGNAL(mapped(QObject *)), this, SLOT(processChecker(QObject *)), Qt::UniqueConnection);

        // Call configure method
        checker->configure();

        // Add it
        checkerVector->push_back(checker);
    }
}

void Referee::resetCheckers() {
    // For each check, call configure() (reset it)
    QList<int> priorityKeys = _checkers.keys();
    QList<int>::iterator it;
    for(it = priorityKeys.begin(); it != priorityKeys.end(); it++) {
        QVector<Checker*> *fouls = _checkers.value((*it));
        for(int i = 0; i < fouls->size(); i++) {
            Checker *atFoul = fouls->at(i);
            atFoul->configure();
        }
    }
}

void Referee::deleteCheckers() {
    QList<int> priorityKeys = _checkers.keys();
    QList<int>::iterator it;

    for(it = priorityKeys.begin(); it != priorityKeys.end(); it++) {
        QVector<Checker*> *fouls = _checkers.take((*it));
        for(int i = 0; i < fouls->size(); i++) {
            Checker *atFoul = fouls->at(i);
            delete atFoul;
        }
    }
}

void Referee::resetTransitionVars() {
    _transitionTimer.start();
    _resetedTimer = false;
    _isStopped = false;
    _teamsPlaced = false;
    _gameHalted = false;
    _longStop = false;
}

void Referee::checkIfTeamsAreColliding() {
    std::cout << Text::yellow("[VSSReferee] ", true) + Text::bold("Checking collisions with teams placements.\n");

    for(int i = VSSRef::PENALTY_KICK; i <= VSSRef::KICKOFF; i++) {
        if(VSSRef::Foul(i) == VSSRef::Foul::FREE_BALL) {
            for(int j = VSSRef::QUADRANT_1; j <= VSSRef::QUADRANT_4; j++) {
                std::cout << Text::yellow("[VSSReferee] ", true) + Text::bold("Sending FREE_BALL in " + VSSRef::Quadrant_Name(VSSRef::Quadrant(j)) + '\n');
                updatePenaltiesInfo(VSSRef::Foul(i), VSSRef::Color::NONE, VSSRef::Quadrant(j));
                sendPenaltiesToNetwork();
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                emit callReplacer(false, false);
            }
        }
        else {
            for(int j = VSSRef::Color::BLUE; j <= VSSRef::Color::YELLOW; j++) {
                std::cout << Text::yellow("[VSSReferee] ", true) + Text::bold("Sending " + VSSRef::Foul_Name(VSSRef::Foul(i)) + "to team " + VSSRef::Color_Name(VSSRef::Color(j)) + '\n');
                updatePenaltiesInfo(VSSRef::Foul(i), VSSRef::Color(j), VSSRef::Quadrant::NO_QUADRANT);
                sendPenaltiesToNetwork();
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                emit callReplacer(false, false);
            }
        }
    }
}

void Referee::updatePenaltiesInfo(VSSRef::Foul foul, VSSRef::Color foulTeam, VSSRef::Quadrant foulQuadrant, bool isManual) {
    // Update info
    _foulMutex.lock();
    _lastFoul = foul;
    _lastFoulTeam = foulTeam;
    _lastFoulQuadrant = foulQuadrant;
    _foulMutex.unlock();

    // Reset transition vars
    if(isManual) {
        resetTransitionVars();
    }
}

void Referee::sendPenaltiesToNetwork() {
    VSSRef::ref_to_team::VSSRef_Command command;

    // Taking last commands
    _foulMutex.lock();
    VSSRef::Foul foul = _lastFoul;
    VSSRef::Quadrant quadrant = _lastFoulQuadrant;
    VSSRef::Color team = _lastFoulTeam;
    _foulMutex.unlock();

    // Parsing last penalties info to command
    command.set_foul(foul);
    command.set_foulquadrant(quadrant);
    command.set_teamcolor(team);

    // Setting timestamp and gamehalf
    command.set_timestamp(_halfChecker->getTimeStamp());
    command.set_gamehalf(_gameHalf);

    // Serializing protobuf to str
    std::string datagram;
    command.SerializeToString(&datagram);

    // Send via socket
    if(_refereeClient->write(datagram.c_str(), static_cast<quint64>(datagram.length())) == -1) {
        std::cout << Text::cyan("[REFEREE] ", true) + Text::red("Failed to write to socket.", true) + '\n';
    }

    // Debug sent foul
    std::cout << Text::blue("[REFEREE] ", true) + Text::yellow("[" + VSSRef::Half_Name(_gameHalf) + ":" + std::to_string(_halfChecker->getTimeStamp()) + "] ", true) + Text::bold("Sent command '" + VSSRef::Foul_Name(_lastFoul) + "' for team '" + VSSRef::Color_Name(_lastFoulTeam) + "' at quadrant '" + VSSRef::Quadrant_Name(_lastFoulQuadrant)) + "'\n";

    // Send foul
    emit sendFoul(foul, team, quadrant);

    // Reset checkers
    resetCheckers();
}

void Referee::processChecker(QObject *checker) {
    _checkerMutex.lock();
    Checker *occurredChecker = static_cast<Checker*>(checker);

    if(occurredChecker->penalty() == VSSRef::Foul::HALT) {
        sendControlFoul(occurredChecker->penalty());
        _gameHalted = true;
        return ;
    }

    // In penaltyShootout, only hear commands from checker ball play
    if(_isPenaltyShootout && !(occurredChecker->name() == "Checker_BallPlay" || occurredChecker->name() == "Checker_StuckedBall")) {
        return ;
    }
    else if(_isPenaltyShootout && (occurredChecker->name() == "Checker_BallPlay" || occurredChecker->name() == "Checker_StuckedBall")){
        // Send penalty foul to place outside
        takeManualFoul(occurredChecker->penalty(), occurredChecker->teamColor(), VSSRef::NO_QUADRANT, true);
        _ballPlayChecker->setIsPenaltyShootout(true, occurredChecker->teamColor());
        _stuckedBallChecker->setIsPenaltyShootout(true, occurredChecker->teamColor());
        return ;
    }

    emit saveFrame();
    updatePenaltiesInfo(occurredChecker->penalty(), occurredChecker->teamColor(), occurredChecker->quadrant());
    sendPenaltiesToNetwork();
    _checkerMutex.unlock();
}

void Referee::halfPassed() {
    // Check actual half
    // If has at second half, check if is needed to go to overtime
    if(_gameHalf == VSSRef::Half::SECOND_HALF) {
        // If eq goals (go to overtime)
        if((_soccerView->getLeftTeamGoals() == _soccerView->getRightTeamGoals()) && (_soccerView->getStage().toLower() != "group_phase" && !_soccerView->getStage().toLower().contains("rodada"))) {
            _halfChecker->setIsOvertime(true);
        }
        else {
            // halt game (end game)
            sendControlFoul(VSSRef::Foul::HALT);
            _gameHalted = true;
            _isEndGame = true;
            return ;
        }
    }
    // If has at end of overtime, check if is need to go to penalty shootouts
    else if(_gameHalf == VSSRef::Half::OVERTIME_SECOND_HALF) {
        // If not eq goals (not go to penalty shootouts)
        if((_soccerView->getLeftTeamGoals() != _soccerView->getRightTeamGoals())) {
            // halt game (end game)
            sendControlFoul(VSSRef::Foul::HALT);
            _gameHalted = true;
            _isEndGame = true;
            return ;
        }
        else {
            // Set halftime checker to penalty shootout
            _halfChecker->setIsPenaltyShootout(true);
            _isPenaltyShootout = true;
        }
    }

    // If passed, update half and kickoff
    int half = (_gameHalf % 5) + 1;
    _gameHalf = VSSRef::Half(half);
    int kickoff = ((_halfKickoff + 1) % 2);
    _halfKickoff = VSSRef::Color(kickoff);
    std::cout << Text::blue("[REFEREE] ", true) + Text::bold("Half passed, now at " + VSSRef::Half_Name(_gameHalf)) + '\n';

    // If is penalty shootout, set penalty kick for one team
    if(_gameHalf == VSSRef::Half::PENALTY_SHOOTOUTS) {
        takeManualFoul(VSSRef::Foul::PENALTY_KICK, _halfKickoff, VSSRef::Quadrant::NO_QUADRANT, true);
        _ballPlayChecker->setIsPenaltyShootout(true, _halfKickoff);
        _stuckedBallChecker->setIsPenaltyShootout(true, _halfKickoff);
        return ;
    }
    // If not is penalty shootout, call kickoff normally
    else {
        // Update penaltie info for an kickoff
        updatePenaltiesInfo(VSSRef::Foul::KICKOFF, _halfKickoff, VSSRef::Quadrant::NO_QUADRANT);
    }

    // Call long stop (5min)
    sendControlFoul(VSSRef::Foul::STOP);
    _longStop = true;
}

void Referee::teamsPlaced() {
    _transitionMutex.lock();
    _teamsPlaced = true;
    _transitionMutex.unlock();
}

void Referee::goalOccurred(VSSRef::Color) {
    _goalOccurred = true;
}

void Referee::sendControlFoul(VSSRef::Foul foul) {
    // Take copy of last foul
    VSSRef::Foul lastFoul = _lastFoul;
    VSSRef::Quadrant lastFoulQuadrant = _lastFoulQuadrant;
    VSSRef::Color lastFoulTeam = _lastFoulTeam;

    // If foul is halt
    if(foul == VSSRef::Foul::HALT) {
        // Take ball last data (before stopping)
        _placedLast = false;
        _lastBallPosition = _vision->getBallPosition();
        _lastBallVelocity = _vision->getBallVelocity();
        if(getConstants()->maintainSpeedAtSuggestions()) emit saveFrame();
    }

    // Update penalties info
    updatePenaltiesInfo(foul, VSSRef::Color::NONE, VSSRef::Quadrant::NO_QUADRANT, true);

    // Send to network
    sendPenaltiesToNetwork();

    // Back penalties to last penalty info
    updatePenaltiesInfo(lastFoul, lastFoulTeam, lastFoulQuadrant, true);
}

void Referee::takeManualFoul(VSSRef::Foul foul, VSSRef::Color foulColor, VSSRef::Quadrant foulQuadrant, bool isToPlaceOutside) {
    if(foul == VSSRef::Foul::GAME_ON) {
        // Reset transitions vars
        resetTransitionVars();

        // Check if last foul is stop
        if(_lastFoul == VSSRef::Foul::STOP) {
            // Set isStopped as true (avoid double stop)
            _isStopped = true;
        }

        if(!_placedLast) {
            if(getConstants()->maintainSpeedAtSuggestions()) emit placeFrame();
            _placedLast = true;
        }

        // Re-send last penalties to network
        sendPenaltiesToNetwork();
    }
    else if(foul == VSSRef::Foul::HALT) {
        // Call halt
        sendControlFoul(foul);
        _gameHalted = true;
    }
    else if(foul == VSSRef::Foul::STOP) {
        // Call long stop
        sendControlFoul(foul);
        _longStop = true;
    }
    else {
        // Update penalties info
        updatePenaltiesInfo(foul, foulColor, foulQuadrant, true);

        // Send to network
        sendPenaltiesToNetwork();

        // Set to place outside if needed
        _isToPlaceOutside = isToPlaceOutside;
    }
}

void Referee::takeStuckedTime(float time) {
    _soccerView->getFieldView()->setStuckedTime(time);
}

void Referee::processCollision(VSSRef::Foul foul, VSSRef::Color foulColor, VSSRef::Quadrant foulQuadrant, bool isToPlaceOutside)  {
    // Call halt
    sendControlFoul(VSSRef::Foul::HALT);
    _gameHalted = true;

    // Take collision vars
    _collisionFoul = foul;
    _collisionColor = foulColor;
    _collisionQuadrant = foulQuadrant;
    _isToPlaceOutside = isToPlaceOutside;

    // Send collision suggestion
    emit emitSuggestion("Collision detected, needs to place by default");
}

void Referee::processCollisionDecision() {
    // Update penalties info
    updatePenaltiesInfo(_collisionFoul, _collisionColor, _collisionQuadrant, true);

    // Send to network
    sendPenaltiesToNetwork();

    // Set as force default
    _forceDefault = true;
}

Constants* Referee::getConstants() {
    if(_constants == nullptr) {
        std::cout << Text::red("[ERROR] ", true) << Text::bold("Constants with nullptr value at Referee") + '\n';
    }
    else {
        return _constants;
    }

    return nullptr;
}
