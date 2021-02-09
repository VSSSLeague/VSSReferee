#include "object.h"

Object::Object(bool useKalman) {
    _useKalman = useKalman;
    setInvalid();
}

Object::~Object() {

}

Position Object::getPosition() {
    Position retn = _position;

    return retn;
}

Velocity Object::getVelocity() {
    Velocity retn = _velocity;

    return retn;
}

Angle Object::getOrientation() {
    Angle retn = _orientation;

    return retn;
}

bool Object::isObjectLoss() {
    return _lossFilter.checkLoss();
}

bool Object::isObjectSafe() {
    return _noiseFilter.checkNoise();
}

void Object::updateObject(float confidence, Position pos, Angle orientation) {
    // Update confidence
    _confidence = confidence;

    // If pos is invalid (robot is not visible in frame)
    if(pos.isInvalid()) {
        // If loss filter is not initialized
        if(!_lossFilter.isInitialized()) {
            // Just start loss
            _lossFilter.startLoss();
        }
        // If loss filter is already initialized
        else {
            // If object is really lost
            if(isObjectLoss()) {
                // Reset noise and set invalid
                _noiseFilter.startNoise();
                setInvalid();
            }
            // If object is not lost already and is safe
            else if((!isObjectLoss() && isObjectSafe()) && _useKalman){
                // Predict with Kalman
                _kalmanFilter.predict();
                _position = _kalmanFilter.getPosition();
                _velocity = _kalmanFilter.getVelocity();
            }
            else if((!isObjectLoss() && isObjectSafe()) && !_useKalman) {
                // Reset vel timer
                _velTimer.start();
            }
        }
    }
    // If pos is valid (robot was saw in frame)
    else {
        // If noise filter is not initialized
        if(!_noiseFilter.isInitialized()) {
            // Init noise
            _noiseFilter.startNoise();
            // Set invalid
            setInvalid();
        }
        // If noise filter is already initialized
        else {
            // If object is safe (survived at noise filter)
            if(isObjectSafe()) {
                // Reset loss filter
                _lossFilter.startLoss();

                if(_useKalman) {
                    // Iterate in kalman filter
                    _kalmanFilter.iterate(pos);

                    // Update positions, orientations, velocity and confidence
                    _position.setPosition(true, _kalmanFilter.getPosition().x(), _kalmanFilter.getPosition().y());
                    _velocity = _kalmanFilter.getVelocity();
                    _orientation = orientation;
                }
                else {
                    // Stop timer (get speed)
                    _velTimer.stop();
                    float vx = (pos.x() - _position.x()) / _velTimer.getSeconds();
                    float vy = (pos.y() - _position.y()) / _velTimer.getSeconds();

                    _position.setPosition(true, pos.x(), pos.y());
                    _velocity.setVelocity(true, vx, vy);
                    _orientation = orientation;

                    // Start timer again (reset to next it)
                    _velTimer.start();
                }
            }
            // If object is unsafe yet (noise is running)
            else {
                // Set invalid
                setInvalid();

                // Reset loss
                _lossFilter.startLoss();
            }
        }
    }
}

void Object::setInvalid() {
    _position.setInvalid();
    _velocity.setInvalid();
    _orientation.setInvalid();
    _velTimer.start();
    _confidence = 0.0;
}
