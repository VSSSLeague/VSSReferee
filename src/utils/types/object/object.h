#ifndef OBJECT_H
#define OBJECT_H

#include <src/utils/types/angle/angle.h>
#include <src/world/entities/vision/filters/loss/lossfilter.h>
#include <src/world/entities/vision/filters/noise/noisefilter.h>
#include <src/world/entities/vision/filters/kalman/kalmanfilter.h>

class Object
{
public:
    Object();
    ~Object();

    // Getters
    Position getPosition();
    Velocity getVelocity();
    Angle getOrientation();

    // Filtering
    bool isObjectSafe();
    bool isObjectLoss();

    // Update
    void updateObject(float confidence, Position pos, Angle orientation = Angle(false, 0.0));
    void setInvalid();

private:
    // Object params
    Position _position;
    Velocity _velocity;
    Angle _orientation;
    float _confidence;

    // Object filters
    LossFilter _lossFilter;
    NoiseFilter _noiseFilter;
    KalmanFilter _kalmanFilter;
};

#endif // OBJECT_H
