#include "checker_twodefenders.h"

QString Checker_TwoDefenders::name() {
    return "Checker_TwoDefenders";
}

void Checker_TwoDefenders::configure() {
    // Insert and start timers
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        _timers.insert(VSSRef::Color(i), new Timer());
        _timers.value(VSSRef::Color(i))->start();
    }
}

void Checker_TwoDefenders::run() {
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        // Taking players at allie goal
        VSSRef::Color oppositeColor = (i == VSSRef::Color::BLUE) ? VSSRef::Color::YELLOW : VSSRef::Color::BLUE;
        Position ballPosition = getVision()->getBallPosition();

        // Check if >= 2 and enable flag
        if(isTwoPlayersDefending(VSSRef::Color(i)) && Utils::isInsideGoalArea(VSSRef::Color(i), ballPosition)) {
            _timers.value(VSSRef::Color(i))->stop();

            if(_timers.value(VSSRef::Color(i))->getSeconds() >= getConstants()->ballInAreaMaxTime() && !getConstants()->useRefereeSuggestions()) {
                setPenaltiesInfo(VSSRef::Foul::PENALTY_KICK, oppositeColor, VSSRef::Quadrant::NO_QUADRANT);
                emit foulOccured();
            }
        }
        else {
            _timers.value(VSSRef::Color(i))->start();
        }
    }
}

bool Checker_TwoDefenders::isAnyTeamDefendingWithTwo() {
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        if(isTwoPlayersDefending(VSSRef::Color(i))) {
            return true;
        }
    }

    return false;
}

bool Checker_TwoDefenders::isTwoPlayersDefending(VSSRef::Color teamColor) {
    // Getting avPlayers
    QList<quint8> avPlayers = getVision()->getAvailablePlayers(teamColor);

    // Count qt players at allie goal
    int countAtAllieGoal = 0;
    for(int j = 0; j < avPlayers.size(); j++) {
        Position playerPosition = getVision()->getPlayerPosition(teamColor, avPlayers.at(j));
        if(Utils::isInsideGoalArea(teamColor, playerPosition)) {
            countAtAllieGoal = countAtAllieGoal + 1;
        }
    }

    // If 2 or more players are defending, return true
    if(countAtAllieGoal >= 2) {
        return true;
    }
    // Otherwise, return false
    else {
        return false;
    }
}

float Checker_TwoDefenders::getTimer() {
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        if(isTwoPlayersDefending(VSSRef::Color(i))) {
            _timers.value(VSSRef::Color(i))->stop();
            return _timers.value(VSSRef::Color(i))->getSeconds();
        }
    }

    return 0.0f;
}

VSSRef::Color Checker_TwoDefenders::defendingTeam() {
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        if(isTwoPlayersDefending(VSSRef::Color(i))) {
            return VSSRef::Color(i);
        }
    }

    return VSSRef::Color::NONE;
}
