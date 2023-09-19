#include "checker_stuckedball.h"
#include <src/world/entities/referee/referee.h>

QString Checker_StuckedBall::name() {
    return "Checker_StuckedBall";
}

void Checker_StuckedBall::configure() {
    _timer.start();
    _isLastStuckAtGoalArea = false;
    emit sendStuckedTime(0.0f);
}

void Checker_StuckedBall::setIsPenaltyShootout(bool isPenaltyShootout, VSSRef::Color firstPenaltyTeam) {
    _isPenaltyShootout = isPenaltyShootout;
    _penaltyTeam = firstPenaltyTeam;
}

void Checker_StuckedBall::setNextTeam() {
    int nextTeam = ((_penaltyTeam + 1) % 2);
    _penaltyTeam = VSSRef::Color(nextTeam);
}

void Checker_StuckedBall::run() {
    // Ball in area control
    bool isAtGoalAreas = false;

    // If ball valid and is stucked (low velocity)
    if(!getVision()->getBallPosition().isInvalid() && getVision()->getBallVelocity().abs() <= getConstants()->ballMinSpeedForStuck()) {
        // Take ball position
        Position ballPosition = getVision()->getBallPosition();

        // Check if ball is inside both goal areas
        for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
            if(Utils::isInsideGoalArea(VSSRef::Color(i), ballPosition)) {
                // Reset timer if is the first time that ball stuck in goal area
                if(!_isLastStuckAtGoalArea) {
                    _isLastStuckAtGoalArea = true;
                    _timer.start();
                }

                // Stop timer
                _timer.stop();

                // Check if timer passed max stuck time
                if(_timer.getSeconds() >= getConstants()->stuckedBallTime() && !_isPenaltyShootout) {
                    // Set penalties and emit that an foul occured
                    // If have ball disputation (nearly ball of both teams)
                    if(havePlayersNearlyBall(VSSRef::Color::BLUE) && havePlayersNearlyBall(VSSRef::Color::YELLOW)) {
                        setPenaltiesInfo(VSSRef::Foul::GOAL_KICK, VSSRef::Color(i), VSSRef::Quadrant::NO_QUADRANT);
                    }
                    else {
                        setPenaltiesInfo(VSSRef::Foul::PENALTY_KICK, ((i == VSSRef::Color::BLUE) ? VSSRef::Color::YELLOW : VSSRef::Color::BLUE), VSSRef::Quadrant::NO_QUADRANT);
                    }
                    emit foulOccured();

                    // Reset timer
                    _timer.start();
                }

                // Set that is inside goal area
                isAtGoalAreas = true;
            }
        }

        // Check if ball is outside goal areas
        if(!isAtGoalAreas) {
            // Reset timer if is the first time that ball stuck outside goal area
            if(_isLastStuckAtGoalArea) {
                _isLastStuckAtGoalArea = false;
                _timer.start();
            }

            // Stop timer
            _timer.stop();

            // Check if timer passed max stuck time
            if(_timer.getSeconds() >= getConstants()->stuckedBallTime()) {
                if(_isPenaltyShootout) {
                    setNextTeam();
                    setPenaltiesInfo(VSSRef::Foul::PENALTY_KICK, _penaltyTeam, VSSRef::Quadrant::NO_QUADRANT);
                    emit foulOccured();
                }
                else {
                    // Set penalties and emit that an foul occured
                    setPenaltiesInfo(VSSRef::Foul::FREE_BALL, VSSRef::Color::NONE, Utils::getBallQuadrant(ballPosition));
                    emit foulOccured();
                }

                // Reset timer
                _timer.start();
            }
        }
    }
    else {
        // Reset timer
        _timer.start();
    }

    if((_isPenaltyShootout && !isAtGoalAreas) || !_isPenaltyShootout) {
        emit sendStuckedTime(_timer.getSeconds());
    }
}

bool Checker_StuckedBall::havePlayersNearlyBall(VSSRef::Color teamColor) {
    QList<quint8> avPlayers = getVision()->getAvailablePlayers(teamColor);
    Position ballPosition = getVision()->getBallPosition();

    for(int i = 0; i < avPlayers.size(); i++) {
        Position playerPosition = getVision()->getPlayerPosition(teamColor, avPlayers.at(i));
        /// TODO: check this distance value later
        if(Utils::distance(playerPosition, ballPosition) <= 1.5 * getConstants()->robotLength()) {
            return true;
        }
    }

    return false;
}
