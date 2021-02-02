#include "kalmanstate.h"

KalmanState::KalmanState() {
    _state.setSize(3, 1);
}

float KalmanState::getPosition() const {
    return _state.get(0, 0);
}

float KalmanState::getVelocity() const {
    return _state.get(1, 0);
}

float KalmanState::getAcceleration() const {
    return _state.get(2, 0);
}

Matrix& KalmanState::matrix() {
    return _state;
}

void KalmanState::setPosition(float value) {
    _state.set(0, 0, value);
}

void KalmanState::setVelocity(float value) {
    _state.set(1, 0, value);
}

void KalmanState::setAcceleration(float value) {
    _state.set(2, 0, value);
}
