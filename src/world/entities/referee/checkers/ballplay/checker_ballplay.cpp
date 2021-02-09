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
    _possibleGoal = false;
}

void Checker_BallPlay::run() {
    // Take ball pos
    Position ballPos = getVision()->getBallPosition();

    if((Utils::isInsideGoalArea(VSSRef::Color::BLUE, ballPos) && !Utils::isBallInsideGoal(VSSRef::Color::BLUE, ballPos)) || (Utils::isInsideGoalArea(VSSRef::Color::YELLOW, ballPos) && !Utils::isBallInsideGoal(VSSRef::Color::YELLOW, ballPos))) {
        // Update control vars
        _isPlayRunning = true;
        if(!_possiblePenalty) {
            _possiblePenalty = _checkerTwoDef->isTwoPlayersDefending();
            if(_possiblePenalty)
                emit emitSuggestion(VSSRef::Foul_Name(VSSRef::Foul::PENALTY_KICK).c_str(), (_checkerTwoDef->defendingTeam() == VSSRef::Color::BLUE) ? VSSRef::Color::YELLOW : VSSRef::Color::BLUE);
        }
        if(!_possibleGoalKick) {
            _possibleGoalKick = _checkerTwoAtk->isTwoPlayersAttacking();
            if(_possibleGoalKick)
                emit emitSuggestion(VSSRef::Foul_Name(VSSRef::Foul::GOAL_KICK).c_str(), _checkerTwoAtk->attackingTeam());
        }
    }
    else {
        // If play was running before
        if(_isPlayRunning) {
            for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
                if(Utils::isBallInsideGoal(VSSRef::Color(i), ballPos)) {
                    // Mark possible goal
                    _possibleGoal = true;

                    // Check if not occurred possible penalty or goal kick
                    if(!_possiblePenalty && !_possibleGoalKick) {
                        // Send as valid goal
                        emit emitGoal((i == VSSRef::Color::BLUE) ? VSSRef::Color::YELLOW : VSSRef::Color::BLUE);

                        setPenaltiesInfo(VSSRef::Foul::KICKOFF, VSSRef::Color(i), VSSRef::Quadrant::NO_QUADRANT);
                        emit foulOccured();
                    }
                    else{
                        // If occurred penalty or goal kick, send an goal suggestion
                        emit emitSuggestion("GOAL", (i == VSSRef::Color::BLUE) ? VSSRef::Color::YELLOW : VSSRef::Color::BLUE);
                    }
                }
            }

            if(_possibleGoalKick || _possiblePenalty) {
                // If any of them occurred, send HALT command
                setPenaltiesInfo(VSSRef::Foul::HALT);
                emit foulOccured();

                // Add game on suggestion if not possible goal
                if(!_possibleGoal) emit emitSuggestion("GAME_ON");

                // Debug
                std::cout << Text::red("[PLAY] ", true) + Text::bold("Possible goal: " + std::to_string(_possibleGoal) + ", Possible penalty: " + std::to_string(_possiblePenalty) + " and Possible goalkick: " + std::to_string(_possibleGoalKick)) + '\n';
            }

            // Reset running control var
            _isPlayRunning = false;
        }
        else {
            // Reset control vars
            _possiblePenalty = false;
            _possibleGoalKick = false;
            _possibleGoal = false;
        }
    }
}
