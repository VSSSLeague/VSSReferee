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
        _secondsMutex.lock();

        // Stop timer
        _timer.stop();

        // Add passed time to seconds
        _secondsPassed += _timer.getSeconds();

        // Check if half passed
        if(_secondsPassed >= getConstants()->halfTime()) {
            _secondsPassed = 0;
            emit halfPassed();
        }

        _secondsMutex.unlock();
    }

    // Restart timer
    _timer.start();
}

void Checker_HalfTime::setReferee(Referee *referee) {
    _referee = referee;
}

float Checker_HalfTime::getTimeStamp() {
    _secondsMutex.lock();
    float timeStamp = _secondsPassed;
    _secondsMutex.unlock();

    return timeStamp;
}

void Checker_HalfTime::receiveTime(int seconds) {
    _secondsMutex.lock();
    _secondsPassed = std::max(0.0f, _secondsPassed - seconds);
    _secondsMutex.unlock();
}
