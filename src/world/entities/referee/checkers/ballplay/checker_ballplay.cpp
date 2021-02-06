#include "checker_ballplay.h"

QString Checker_BallPlay::name() {
    return "Checker_BallPlay";
}

void Checker_BallPlay::setAtkDefCheckers(Checker_TwoAttackers *twoAtk, Checker_TwoDefenders *twoDef) {
    _checkerTwoAtk = twoAtk;
    _checkerTwoDef = twoDef;
}

void Checker_BallPlay::configure() {
    // Reset control
    _isPlayRunning = false;
    _possiblePenalty = false;
    _possibleGoalKick = false;
}

void Checker_BallPlay::run() {
    // Take ball pos
    Position ballPos = getVision()->getBallPosition();

    if(Utils::isInsideGoalArea(VSSRef::Color::BLUE, ballPos) || Utils::isInsideGoalArea(VSSRef::Color::YELLOW, ballPos)) {
        // Update control vars
        _isPlayRunning = true;
        if(!_possiblePenalty) {
            _possiblePenalty = _checkerTwoDef->isTwoPlayersDefending();
        }
        if(!_possibleGoalKick) {
            _possibleGoalKick = _checkerTwoAtk->isTwoPlayersAttacking();
        }
    }
    else {
        // If play was running before
        if(_isPlayRunning) {
            // Debug
            std::cout << Text::red("[PLAY] ", true) + Text::bold("Possible penalty: " + std::to_string(_possiblePenalty) + " and Possible goalkick: " + std::to_string(_possibleGoalKick)) + '\n';

            // Reset running control var
            _isPlayRunning = false;
        }
        else {
            // Reset control vars
            _possiblePenalty = false;
            _possibleGoalKick = false;
        }
    }
}
