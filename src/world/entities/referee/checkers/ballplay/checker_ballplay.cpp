﻿#include "checker_ballplay.h"

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
    _areaTimerControl = false;
    _areaTimer.start();
}

void Checker_BallPlay::run() {
    // Take ball pos
    Position ballPos = getVision()->getBallPosition();

    if(!_areaTimerControl && ((Utils::isInsideGoalArea(VSSRef::Color::BLUE, ballPos) && !Utils::isBallInsideGoal(VSSRef::Color::BLUE, ballPos)) || (Utils::isInsideGoalArea(VSSRef::Color::YELLOW, ballPos) && !Utils::isBallInsideGoal(VSSRef::Color::YELLOW, ballPos)))) {
        // Update control vars
        _isPlayRunning = true;
        if(!_possiblePenalty) {
            _possiblePenalty = _checkerTwoDef->isTwoPlayersDefending();
            if(_possiblePenalty && getConstants()->useRefereeSuggestions()) {
                emit emitSuggestion(VSSRef::Foul_Name(VSSRef::Foul::PENALTY_KICK).c_str(), (_checkerTwoDef->defendingTeam() == VSSRef::Color::BLUE) ? VSSRef::Color::YELLOW : VSSRef::Color::BLUE);
            }
        }

        if(!_possibleGoalKick) {
            _possibleGoalKick = _checkerTwoAtk->isTwoPlayersAttacking();
            if(_possibleGoalKick && getConstants()->useRefereeSuggestions()) {
                emit emitSuggestion(VSSRef::Foul_Name(VSSRef::Foul::GOAL_KICK).c_str(), _checkerTwoAtk->attackingTeam());
            }
        }

        // Update passed timer if an foul has detected ( >= ballInAreaMaxTime() will halt game )
        if((_possibleGoalKick || _possiblePenalty) && getConstants()->useRefereeSuggestions()) {
            _areaTimer.stop();
            if(_areaTimer.getSeconds() >= getConstants()->ballInAreaMaxTime()) {
                _areaTimerControl = true;
            }
        }
        else {
            _areaTimer.start();
        }
    }
    else {
        if(_isPlayRunning) {
            // If play was running before, check if occurred an goal or ball just leaved goal area
            for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
                if(Utils::isBallInsideGoal(VSSRef::Color(i), ballPos)) {
                    // Mark possible goal
                    _possibleGoal = true;
                    _possibleGoalTeam = (i == VSSRef::Color::BLUE) ? VSSRef::Color::YELLOW : VSSRef::Color::BLUE;

                    // Check if not occurred possible penalty or goal kick
                    if(!_possiblePenalty && !_possibleGoalKick) {
                        // Send as valid goal
                        emit emitGoal(_possibleGoalTeam);

                        setPenaltiesInfo(VSSRef::Foul::KICKOFF, VSSRef::Color(i), VSSRef::Quadrant::NO_QUADRANT);
                        emit foulOccured();
                    }
                    else{
                        if(getConstants()->useRefereeSuggestions()) {
                            // If occurred penalty or goal kick, send an goal suggestion
                            emit emitSuggestion("GOAL", _possibleGoalTeam);

                            // Send also an suggestion of free ball
                            emit emitSuggestion("FREE_BALL", VSSRef::Color::NONE, Utils::getBallQuadrant(getVision()->getBallPosition()));
                        }
                    }
                }
            }

            if(_possibleGoalKick || _possiblePenalty) {
                // If any of them occurred, send HALT command
                if(getConstants()->useRefereeSuggestions()) {
                    setPenaltiesInfo(VSSRef::Foul::HALT);
                    emit foulOccured();
                }

                // Add game on suggestion if not possible goal
                if(!_possibleGoal && getConstants()->useRefereeSuggestions()) {
                    emit emitSuggestion("GAME_ON");
                }

                if(!getConstants()->useRefereeSuggestions()) {
                    // Check priority
                    if(_possibleGoal) {
                        // Possible goal, possible goal kick and not possible penalty, priority: GOAL_KICK
                        if(_possibleGoalKick && !_possiblePenalty) {
                            setPenaltiesInfo(VSSRef::Foul::GOAL_KICK, _checkerTwoAtk->attackingTeam());
                            emit foulOccured();
                        }
                        // Possible goal, possible penalty and not possible goal kick, priority: GOAL
                        else if(!_possibleGoalKick && _possiblePenalty) {
                            emit emitGoal(_possibleGoalTeam);

                            setPenaltiesInfo(VSSRef::Foul::KICKOFF, (_possibleGoalTeam == VSSRef::Color::BLUE) ? VSSRef::Color::YELLOW : VSSRef::Color::BLUE, VSSRef::Quadrant::NO_QUADRANT);
                            emit foulOccured();
                        }
                        // Possible goal, possible penalty and possible goal kick... priority: the later
                        else if(_possibleGoalKick && _possiblePenalty) {
                            // Take the most later foul
                            float bestTime = 0.0f;

                            if(_checkerTwoAtk->getTimer() > bestTime) {
                                bestTime = _checkerTwoAtk->getTimer();
                                setPenaltiesInfo(VSSRef::Foul::GOAL_KICK, _checkerTwoAtk->attackingTeam());
                            }

                            if(_checkerTwoDef->getTimer() > bestTime) {
                                bestTime = _checkerTwoDef->getTimer();
                                setPenaltiesInfo(VSSRef::Foul::PENALTY_KICK, (_checkerTwoDef->defendingTeam() == VSSRef::Color::BLUE) ? VSSRef::Color::YELLOW : VSSRef::Color::BLUE);
                            }

                            emit foulOccured();
                        }
                    }
                    // If no possible goal occurred
                    else {
                        // Take the most later foul
                        float bestTime = 0.0f;

                        if(_checkerTwoAtk->getTimer() > bestTime) {
                            bestTime = _checkerTwoAtk->getTimer();
                            setPenaltiesInfo(VSSRef::Foul::GOAL_KICK, _checkerTwoAtk->attackingTeam());
                        }

                        if(_checkerTwoDef->getTimer() > bestTime) {
                            bestTime = _checkerTwoDef->getTimer();
                            setPenaltiesInfo(VSSRef::Foul::PENALTY_KICK, (_checkerTwoDef->defendingTeam() == VSSRef::Color::BLUE) ? VSSRef::Color::YELLOW : VSSRef::Color::BLUE);
                        }

                        emit foulOccured();
                    }
                }

                // Debug
                //std::cout << Text::red("[PLAY] ", true) + Text::bold("Possible goal: " + std::to_string(_possibleGoal) + ", Possible penalty: " + std::to_string(_possiblePenalty) + " and Possible goalkick: " + std::to_string(_possibleGoalKick)) + '\n';
            }

            // Reset running control var
            _isPlayRunning = false;
        }
        else {
            // Reset control vars
            _possiblePenalty = false;
            _possibleGoalKick = false;
            _possibleGoal = false;
            _areaTimerControl = false;
            _areaTimer.start();
        }
    }
}
