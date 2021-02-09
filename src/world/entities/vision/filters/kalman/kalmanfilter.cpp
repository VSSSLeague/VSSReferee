#include "kalmanfilter.h"

QString KalmanFilter::name() {
    return "KalmanFilter";
}

KalmanFilter::KalmanFilter() {
    _has1stPosition = _has2ndPosition = false;

    // Initialize state matrices
    _Px = Matrix::diag(3, KalmanFilter::_p);
    _Py = Matrix::diag(3, KalmanFilter::_p);

    // Initialize model matrices
    _A = Matrix::identity(3);

    _H.setSize(1, 3);
    _H.set(0, 0, 1);

    _Q.setSize(3, 3);

    _R.setSize(1, 1);
    _R.set(0, 0, pow(KalmanFilter::_r, 2));

    _timer.start();
}

void KalmanFilter::updateMatrices(const float T) {
    // Update A with time (T)
    _A.set(0, 1, T);
    _A.set(0, 2, pow(T,2)/2);
    _A.set(1, 2, T);

    /// Time update / PREDICT
    // Project state ahead
    _X.matrix() = _A*_X.matrix();
    _Y.matrix() = _A*_Y.matrix();

    // Update Q with time (T)
    _Q.set(0, 0, pow(T,4)/4);
    _Q.set(0, 1, pow(T,3)/2);
    _Q.set(0, 2, pow(T,2)/2);
    _Q.set(1, 0, pow(T,3)/2);
    _Q.set(1, 1, pow(T,2));
    _Q.set(1, 2, T);
    _Q.set(2, 0, pow(T,2)/2);
    _Q.set(2, 1, T);
    _Q.set(2, 2, 1);
    _Q *= pow(KalmanFilter::_sigma_a, 2);

    // Project the error covariance ahead
    _Px = _A*_Px*_A.transposed() + _Q;
    _Py = _A*_Py*_A.transposed() + _Q;
}

void KalmanFilter::iterate(const Position &pos) {

    // Check if position is known
    if(pos.isInvalid())
        return this->predict();

    // Get iteration time
    _timer.stop();
    const float T = _timer.getSeconds();
    _timer.start();

    // Initial states
    if(_has1stPosition==false) {
        _X.setPosition(pos.x());
        _Y.setPosition(pos.y());
        _has1stPosition = true;
        return;
    }
    if(_has2ndPosition==false) {
        _X.setVelocity((pos.x() - _X.getPosition())/T);
        _X.setPosition(pos.x());
        _Y.setVelocity((pos.y() - _Y.getPosition())/T);
        _Y.setPosition(pos.y());
        _has2ndPosition = true;
        return;
    }

    updateMatrices(T);

    /// Measurement update / CORRECT
    // Compute the Kalman gain
    Matrix Kx = _Px*_H.transposed() * (1/(_H*_Px*_H.transposed() + _R).get(0, 0));
    Matrix Ky = _Py*_H.transposed() * (1/(_H*_Py*_H.transposed() + _R).get(0, 0));

    // Update estimate with measurement Zk
    _X.matrix() = _X.matrix() + Kx*(pos.x() - (_H*_X.matrix()).get(0, 0));
    _Y.matrix() = _Y.matrix() + Ky*(pos.y() - (_H*_Y.matrix()).get(0, 0));

    // Update the error covariance
    const Matrix I = Matrix::identity(3);
    _Px = (I - Kx*_H)*_Px;
    _Py = (I - Ky*_H)*_Py;
}

void KalmanFilter::predict() {

    // Get iteration time
    _timer.stop();
    const float T = _timer.getSeconds();
    _timer.start();

    // Check initial states, if do not have, quit. cannot make prevision...
    if(_has1stPosition==false || _has2ndPosition==false) {
        return;
    }

    updateMatrices(T);
}

Position KalmanFilter::getPosition() const {
    return Position(true, _X.getPosition(), _Y.getPosition());
}

Velocity KalmanFilter::getVelocity() const {
    return Velocity(true, _X.getVelocity(), _Y.getVelocity());
}

Velocity KalmanFilter::getAcceleration() const {
    return Velocity(true, _X.getAcceleration(), _Y.getAcceleration());
}

void KalmanFilter::setEnabled(bool enable){
    enabled = enable;
}

bool KalmanFilter::getEnabled(){
    return enabled;
}
