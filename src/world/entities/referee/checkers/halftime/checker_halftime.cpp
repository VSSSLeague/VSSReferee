#include "checker_halftime.h"
#include <src/world/entities/referee/referee.h>

QString Checker_HalfTime::name() {
    return "Checker_HalfTime";
}

void Checker_HalfTime::configure() {
    _timer.start();
    _secondsPassed = 0.0f;
}

void Checker_HalfTime::run() {
    // If is game on
    if(_referee->isGameOn()) {
        // Stop timer
        _timer.stop();

        // Add passed time to seconds
        _secondsPassed += _timer.getSeconds();

        // Check if half passed
        if(_secondsPassed >= getConstants()->halfTime()) {
            _secondsPassed = 0;
            emit halfPassed();
        }
    }

    // Restart timer
    _timer.start();
}

void Checker_HalfTime::setReferee(Referee *referee) {
    _referee = referee;
}

float Checker_HalfTime::getTimeStamp() {
    return _secondsPassed;
}
