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
}

void Checker_TwoAttackers::run() {
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        // Taking opposite color and ball position
        VSSRef::Color oppositeColor = (i == VSSRef::Color::BLUE) ? VSSRef::Color::YELLOW : VSSRef::Color::BLUE;
        Position ballPosition = getVision()->getBallPosition();

        // Check if >= 2 at goal area or >= 4 at big area and enable flag
        if((isTwoPlayersAttackingAtGoalArea(VSSRef::Color(i)) && Utils::isInsideGoalArea(oppositeColor, ballPosition))
          || (isFourPlayersAttackingAtBigArea(VSSRef::Color(i)) && Utils::isInsideBigArea(oppositeColor, ballPosition)) ) {
            _timers.value(VSSRef::Color(i))->stop();

            if(_timers.value(VSSRef::Color(i))->getSeconds() >= getConstants()->ballInAreaMaxTime() && !getConstants()->useRefereeSuggestions()) {
                setPenaltiesInfo(VSSRef::Foul::GOAL_KICK, oppositeColor, VSSRef::Quadrant::NO_QUADRANT);
                emit foulOccured();
            }
        }
        else {
            _timers.value(VSSRef::Color(i))->start();
        }
    }
}

bool Checker_TwoAttackers::isAnyTeamAttackingWithMoreThanPossible() {
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        if(isTwoPlayersAttackingAtGoalArea(VSSRef::Color(i)) || isFourPlayersAttackingAtBigArea(VSSRef::Color(i))) {
            return true;
        }
    }

    return false;
}

bool Checker_TwoAttackers::isTwoPlayersAttackingAtGoalArea(VSSRef::Color teamColor) {
    Position ballPosition = getVision()->getBallPosition();
    QList<quint8> avPlayers = getVision()->getAvailablePlayers(teamColor);
    VSSRef::Color oppositeColor = (teamColor == VSSRef::Color::BLUE) ? VSSRef::Color::YELLOW : VSSRef::Color::BLUE;

    // Count opposite players at goal
    int countAtOppositeGoal = 0;
    for(int i = 0; i < avPlayers.size(); i++) {
        Position playerPosition = getVision()->getPlayerPosition(VSSRef::Color(teamColor), avPlayers.at(i));
        if(Utils::isInsideGoalArea(oppositeColor, playerPosition) && Utils::isInsideGoalArea(oppositeColor, ballPosition)) {
            countAtOppositeGoal = countAtOppositeGoal + 1;
        }
    }

    // If 2 or more players are attacking, return true
    if(countAtOppositeGoal >= 2) {
        return true;
    }
    // Otherwise, return false
    else {
        return false;
    }
}

bool Checker_TwoAttackers::isFourPlayersAttackingAtBigArea(VSSRef::Color teamColor) {
    Position ballPosition = getVision()->getBallPosition();
    QList<quint8> avPlayers = getVision()->getAvailablePlayers(teamColor);
    VSSRef::Color oppositeColor = (teamColor == VSSRef::Color::BLUE) ? VSSRef::Color::YELLOW : VSSRef::Color::BLUE;

    // Count opposite players at goal
    int countAtOppositeGoal = 0;
    for(int i = 0; i < avPlayers.size(); i++) {
        Position playerPosition = getVision()->getPlayerPosition(VSSRef::Color(teamColor), avPlayers.at(i));
        if(Utils::isInsideBigArea(oppositeColor, playerPosition) && Utils::isInsideBigArea(oppositeColor, ballPosition)) {
            countAtOppositeGoal = countAtOppositeGoal + 1;
        }
    }

    // If 4 or more players are attacking, return true
    if(countAtOppositeGoal >= 4) {
        return true;
    }
    // Otherwise, return false
    else {
        return false;
    }
}

float Checker_TwoAttackers::getTimer() {
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        if(isTwoPlayersAttackingAtGoalArea(VSSRef::Color(i)) || isFourPlayersAttackingAtBigArea(VSSRef::Color(i))) {
            _timers.value(VSSRef::Color(i))->stop();
           return _timers.value(VSSRef::Color(i))->getSeconds();
        }
    }

    return 0.0f;
}

VSSRef::Color Checker_TwoAttackers::attackingTeam() {
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        if(isTwoPlayersAttackingAtGoalArea(VSSRef::Color(i)) || isFourPlayersAttackingAtBigArea(VSSRef::Color(i))) {
           return VSSRef::Color(i);
        }
    }

    return VSSRef::Color::NONE;
}
