#include "checker_twoattackers.h"

QString Checker_TwoAttackers::name() {
    return "Checker_TwoAttackers";
}

void Checker_TwoAttackers::configure() {
    // Insert and start timers
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        _timers.insert(VSSRef::Color(i), new Timer());
        _timers.value(VSSRef::Color(i))->start();
    }

    // Set default flag value
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        _twoAttacking.insert(VSSRef::Color(i), false);
    }
}

void Checker_TwoAttackers::run() {
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        // Taking players at opposite goal
        QList<quint8> avPlayers = getVision()->getAvailablePlayers(VSSRef::Color(i));
        VSSRef::Color oppositeColor = (i == VSSRef::Color::BLUE) ? VSSRef::Color::YELLOW : VSSRef::Color::BLUE;
        Position ballPosition = getVision()->getBallPosition();

        // Count qt players at opposite goal
        int countAtOppositeGoal = 0;
        for(int j = 0; j < avPlayers.size(); j++) {
            Position playerPosition = getVision()->getPlayerPosition(VSSRef::Color(i), avPlayers.at(j));
            if(Utils::isInsideGoalArea(oppositeColor, playerPosition)) {
                countAtOppositeGoal = countAtOppositeGoal + 1;
            }
        }

        // Check if >= 2 and enable flag
        if(countAtOppositeGoal >= 2 && Utils::isInsideGoalArea(oppositeColor, ballPosition)) {
            _twoAttacking.insert(VSSRef::Color(oppositeColor), true);
            _timers.value(VSSRef::Color(i))->stop();
            // TODO: change this timer for constants later
            if(_timers.value(VSSRef::Color(i))->getSeconds() >= 2.0) {
                setPenaltiesInfo(VSSRef::Foul::GOAL_KICK, oppositeColor, VSSRef::Quadrant::NO_QUADRANT);
                emit foulOccured();
            }
        }
        else {
            _twoAttacking.insert(VSSRef::Color(oppositeColor), false);
            _timers.value(VSSRef::Color(i))->start();
        }
    }
}

bool Checker_TwoAttackers::isTwoPlayersAttacking() {
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        if(_twoAttacking.value(VSSRef::Color(i))) {
            return true;
        }
    }

    return false;
}
