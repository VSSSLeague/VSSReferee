#include "velocity.h"

Velocity::Velocity(){
    setInvalid();
}

Velocity::Velocity(bool isValid, float vx, float vy){
    _isValid = isValid;
    _vx = vx;
    _vy = vy;
}

void Velocity::setVelocity(bool isValid, float vx, float vy){
    _isValid = isValid;
    _vx = vx;
    _vy = vy;
}

void Velocity::setInvalid(){
    _isValid = false;
    _vx = 0.0;
    _vy = 0.0;
}

bool Velocity::isInvalid(){
    return _isValid;
}

float Velocity::vx() const {
    return(_vx);
}

float Velocity::vy() const {
    return(_vy);
}

float Velocity::abs() const {
    return sqrt(pow(_vx, 2) + pow(_vy, 2));
}

float Velocity::arg() const {
    return atan2(_vy, _vx);
}
