#ifndef VELOCITY_H
#define VELOCITY_H

#include <math.h>

class Velocity
{
public:
    Velocity();
    Velocity(bool isValid, float vx, float vy);

    // Setters
    void setVelocity(bool isValid, float vx, float vy);
    void setInvalid();

    // Getterrs
    bool isInvalid();
    float vx() const;
    float vy() const;
    float abs() const;
    float arg() const;

private:
    // Params
    bool _isValid;
    float _vx;
    float _vy;
};

#endif // VELOCITY_H
