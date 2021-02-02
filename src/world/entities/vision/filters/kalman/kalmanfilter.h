#ifndef KALMANFILTER_H
#define KALMANFILTER_H

#include <QString>
#include <math.h>
#include <src/utils/types/position/position.h>
#include <src/utils/types/velocity/velocity.h>
#include <src/utils/timer/timer.h>
#include <src/world/entities/vision/filters/kalman/state/kalmanstate.h>

class KalmanFilter {
private:
    Timer _timer;

    // Initial state
    bool _has1stPosition, _has2ndPosition;

    // State and covariance matrices
    KalmanState _X, _Y;
    Matrix _Px, _Py;

    // Model
    Matrix _A;
    Matrix _H;
    Matrix _Q;
    Matrix _R;

    // Model config
    static constexpr float _p = 1;
    static constexpr float _sigma_a = 0.01; // affects Q
    static constexpr float _r = 0.000283; // affects R

    // Private methods
    void updateMatrices(const float T);
    bool enabled;

public:
    KalmanFilter();

    QString name();
    void iterate(const Position &pos);
    void predict();
    Position getPosition() const;
    Velocity getVelocity()  const;
    Velocity getAcceleration() const;

    void setEnabled(bool _enable);
    bool getEnabled();
};

#endif // KALMANFILTER_H
