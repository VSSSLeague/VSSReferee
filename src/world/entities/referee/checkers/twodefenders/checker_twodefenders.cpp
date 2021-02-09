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

    // Set default flag value
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        _twoDefending.insert(VSSRef::Color(i), false);
    }
}

void Checker_TwoDefenders::run() {
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        // Taking players at allie goal
        QList<quint8> avPlayers = getVision()->getAvailablePlayers(VSSRef::Color(i));
        VSSRef::Color oppositeColor = (i == VSSRef::Color::BLUE) ? VSSRef::Color::YELLOW : VSSRef::Color::BLUE;
        Position ballPosition = getVision()->getBallPosition();

        // Count qt players at allie goal
        int countAtAllieGoal = 0;
        for(int j = 0; j < avPlayers.size(); j++) {
            Position playerPosition = getVision()->getPlayerPosition(VSSRef::Color(i), avPlayers.at(j));
            if(Utils::isInsideGoalArea(VSSRef::Color(i), playerPosition)) {
                countAtAllieGoal = countAtAllieGoal + 1;
            }
        }

        // Check if >= 2 and enable flag
        if(countAtAllieGoal >= 2 && Utils::isInsideGoalArea(VSSRef::Color(i), ballPosition)) {
            _twoDefending.insert(VSSRef::Color(i), true);
            _timers.value(VSSRef::Color(i))->stop();
            // TODO: change this timer for constants later
            if(_timers.value(VSSRef::Color(i))->getSeconds() >= 2.0) {
                setPenaltiesInfo(VSSRef::Foul::PENALTY_KICK, oppositeColor, VSSRef::Quadrant::NO_QUADRANT);
                emit foulOccured();
            }
        }
        else {
            _twoDefending.insert(VSSRef::Color(i), false);
            _timers.value(VSSRef::Color(i))->start();
        }
    }
}

bool Checker_TwoDefenders::isTwoPlayersDefending() {
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        if(_twoDefending.value(VSSRef::Color(i))) {
            return true;
        }
    }

    return false;
}

VSSRef::Color Checker_TwoDefenders::defendingTeam() {
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        if(_twoDefending.value(VSSRef::Color(i))) {
            return VSSRef::Color(i);
        }
    }

    return VSSRef::Color::NONE;
}
