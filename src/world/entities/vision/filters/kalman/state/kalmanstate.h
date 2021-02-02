#ifndef KALMANSTATE_H
#define KALMANSTATE_H

#include <src/world/entities/vision/filters/kalman/matrix/matrix.h>

class KalmanState {
public:
    KalmanState();

    void setPosition(float value);
    void setVelocity(float value);
    void setAcceleration(float value);
    float getPosition() const;
    float getVelocity() const;
    float getAcceleration() const;
    Matrix& matrix();

private:
    Matrix _state;
};

#endif // KALMANSTATE_H
