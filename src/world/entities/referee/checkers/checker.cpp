#include "checker.h"
#include <src/world/entities/referee/referee.h>

Checker::Checker(Vision *vision/*, Referee *referee*/, Constants *constants) {
    // Sets vision module
    _vision = vision;

    // Sets referee
    //_referee = referee;

    // Sets constants
    _constants = constants;
}

void Checker::setPenaltiesInfo(VSSRef::Foul penalty, VSSRef::Color teamColor, VSSRef::Quadrant quadrant) {
    _penalty = penalty;
    _teamColor = teamColor;
    _quadrant = quadrant;
}

VSSRef::Foul Checker::penalty() {
    return _penalty;
}

VSSRef::Color Checker::teamColor() {
    return _teamColor;
}

VSSRef::Quadrant Checker::quadrant() {
    return _quadrant;
}

Vision* Checker::getVision() {
    if(_vision == nullptr) {
        std::cout << Text::red("[ERROR] ", true) << Text::bold("Vision with nullptr value at " + name().toStdString()) + '\n';
    }
    else {
        return _vision;
    }

    return nullptr;
}
/*
Referee* Checker::getReferee() {
    if(_referee == nullptr) {
        std::cout << Text::red("[ERROR] ", true) << Text::bold("Referee with nullptr value at " + name().toStdString()) + '\n';
    }
    else {
        return _referee;
    }

    return nullptr;
}
*/
Constants* Checker::getConstants() {
    if(_constants == nullptr) {
        std::cout << Text::red("[ERROR] ", true) << Text::bold("Constants with nullptr value at " + name().toStdString()) + '\n';
    }
    else {
        return _constants;
    }

    return nullptr;
}
/*
bool Checker::isGameOn() {
    return (getReferee()->getLastPenaltyInfo().first == VSSRef::Foul::GAME_ON);
}
*/
