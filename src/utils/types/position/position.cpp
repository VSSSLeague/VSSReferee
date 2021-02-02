#include "position.h"

Position::Position(){
    setInvalid();
}

Position::Position(bool isValid, float x, float y){
    _isValid = isValid;
    _x = x;
    _y = y;
}

void Position::setPosition(bool isValid, float x, float y){
    _isValid = isValid;
    _x = x;
    _y = y;
}

void Position::setInvalid(){
    _isValid = false;
    _x = 0.0;
    _y = 0.0;
}

bool Position::isInvalid() const{
    return !_isValid;
}

float Position::x() const {
    return(_x);
}

float Position::y() const {
    return(_y);
}
