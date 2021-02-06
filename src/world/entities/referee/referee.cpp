#include "referee.h"

#include <include/vssref_command.pb.h>

Referee::Referee(Vision *vision, Replacer *replacer, Constants *constants) : Entity(ENT_REFEREE) {
    // Take vision pointer
    _vision = vision;

    // Take replacer pointer
    _replacer = replacer;

    // Take constants
    _constants = constants;

    // Taking network data
    _refereeAddress = getConstants()->refereeAddress();
    _refereePort = getConstants()->refereePort();

    // Connecting referee to replacer
    connect(_replacer, SIGNAL(teamsPlaced()), this, SLOT(teamsPlaced()));
    connect(this, SIGNAL(sendFoul(VSSRef::Foul, VSSRef::Color, VSSRef::Quadrant)), _replacer, SLOT(takeFoul(VSSRef::Foul, VSSRef::Color, VSSRef::Quadrant)));
    connect(this, SIGNAL(callReplacer()), _replacer, SLOT(placeTeams()));

    // Init signal mapper
    _mapper = new QSignalMapper();
}

void Referee::initialization() {
    // Adding checkers
    // Stucked ball
    addChecker(new Checker_StuckedBall(_vision, getConstants()), 0);

    // Two attackers
    addChecker(_twoAtkChecker = new Checker_TwoAttackers(_vision, getConstants()), 1);

    // Two defenders
    addChecker(_twoDefChecker = new Checker_TwoDefenders(_vision, getConstants()), 1);

    // Ball play
    addChecker(_ballPlayChecker = new Checker_BallPlay(_vision, getConstants()), 2);
    _ballPlayChecker->setAtkDefCheckers(_twoAtkChecker, _twoDefChecker);

    // Goalie
    _goalieChecker = new Checker_Goalie(_vision, getConstants());
    connect(_goalieChecker, SIGNAL(updateGoalie(VSSRef::Color, quint8)), _replacer, SLOT(takeGoalie(VSSRef::Color, quint8)));
    addChecker(_goalieChecker, 0);

    // HalfTime
    _halfChecker = new Checker_HalfTime(_vision, getConstants());
    _halfChecker->setReferee(this);
    connect(_halfChecker, SIGNAL(halfPassed()), this, SLOT(halfPassed()));

    // Set default initial state
    _gameHalf = VSSRef::NO_HALF;
    _isStopped = false;
    _teamsPlaced = false;

    // Connect
    connectClient();

    // Debug network info
    std::cout << Text::blue("[REFEREE] ", true) + Text::bold("Module started at address '" + _refereeAddress.toStdString() + "' and port '" + std::to_string(_refereePort) + "'.") + '\n';

    // Call halfPassed (start game)
    halfPassed();
}

void Referee::loop() {
    // Run half checker
    _halfChecker->run();

    // Send timestamp
    emit sendTimestamp(_halfChecker->getTimeStamp(), _gameHalf);

    // If game is on, run all checks
    if(_lastFoul == VSSRef::Foul::GAME_ON) {
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
        _isStopped = false;
        _resetedTimer = false;
        _teamsPlaced = false;
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

            // Check if passed transition time
            if(_transitionTimer.getSeconds() >= getConstants()->transitionTime() || (teamsPlaced && _transitionTimer.getSeconds() >= (getConstants()->transitionTime() / 2.0))) {
                // Set control vars
                _isStopped = true;
                _resetedTimer = false;

                // Update sent foul to STOP
                updatePenaltiesInfo(VSSRef::Foul::STOP, VSSRef::Color::NONE, VSSRef::Quadrant::NO_QUADRANT);
                sendPenaltiesToNetwork();

                // Call replacer (place teams)
                emit callReplacer();
            }
        }
        else {
            if(!_resetedTimer) {
                _transitionTimer.start();
                _resetedTimer = true;
            }

            // Stop timer
            _transitionTimer.stop();

            // Check if passed transition time
            if(_transitionTimer.getSeconds() >= getConstants()->transitionTime()) {
                // Reset control vars
                _isStopped = false;
                _resetedTimer = false;
                _teamsPlaced = false;

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
    return (_lastFoul == VSSRef::Foul::GAME_ON);
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

void Referee::updatePenaltiesInfo(VSSRef::Foul foul, VSSRef::Color foulTeam, VSSRef::Quadrant foulQuadrant) {
    _foulMutex.lock();
    _lastFoul = foul;
    _lastFoulTeam = foulTeam;
    _lastFoulQuadrant = foulQuadrant;
    _foulMutex.unlock();
}

void Referee::sendPenaltiesToNetwork() {
    VSSRef::ref_to_team::VSSRef_Command command;

    // Parsing last penalties info to command
    command.set_foul(_lastFoul);
    command.set_foulquadrant(_lastFoulQuadrant);
    command.set_teamcolor(_lastFoulTeam);

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
    emit sendFoul(_lastFoul, _lastFoulTeam, _lastFoulQuadrant);

    // Reset checkers
    resetCheckers();
}

void Referee::processChecker(QObject *checker) {
    Checker *occurredChecker = static_cast<Checker*>(checker);
    updatePenaltiesInfo(occurredChecker->penalty(), occurredChecker->teamColor(), occurredChecker->quadrant());
    sendPenaltiesToNetwork();
}

void Referee::halfPassed() {
    // Update half
    int half = (_gameHalf % 2) + 1;
    _gameHalf = VSSRef::Half(half);
    std::cout << Text::blue("[REFEREE] ", true) + Text::bold("Half passed, now at " + VSSRef::Half_Name(_gameHalf)) + '\n';

    // Update penaltie info for an kickoff
    updatePenaltiesInfo(VSSRef::Foul::KICKOFF, VSSRef::Color::NONE, VSSRef::Quadrant::NO_QUADRANT);

    // Send to network
    sendPenaltiesToNetwork();
}

void Referee::teamsPlaced() {
    _transitionMutex.lock();
    _teamsPlaced = true;
    _transitionMutex.unlock();
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
