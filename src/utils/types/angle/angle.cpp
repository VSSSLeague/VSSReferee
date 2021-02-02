#include "angle.h"

#include <cmath>

Angle::Angle() {
    setInvalid();
}

Angle::Angle(bool known, float theValue) {
    (known == true) ? setValue(theValue) : setInvalid();
}

float Angle::toRadians(float angle) {
    // Returns the converted value
    return(angle*toRad);
}

float Angle::toDegrees(const Angle& angle) {
    // Returns the converted value
    return(angle.value()*toDeg);
}

float Angle::toDegrees(float angle) {
    // Returns the converted value
    return(angle*toDeg);
}

float Angle::difference(const Angle& a, const Angle& b) {
    // Calculates the difference
    float difference = a.value()-b.value();

    // Verifies if the difference is the absolute smallest
    if (difference > Angle::pi) {
        difference -= Angle::twoPi;
    }
    else if (difference < -Angle::pi) {
        difference += Angle::twoPi;
    }

    // Returns the difference
    return(difference);
}

// Set/get functions
float Angle::value() const { return(_value); }
void  Angle::setValue(float theValue) {
    // Sets the angle in the range 0 <= angle < 2*Pi
    _value = fmod(theValue, Angle::twoPi);
    if (_value < 0.0) {
        _value += Angle::twoPi;
    }

    // Sets as a valid known angle
    _valid   = true;
    _unknown = false;
}


// Info functions
bool Angle::isValid()   const { return(_valid);   }
void Angle::setInvalid() { _valid = false; }
