#include "lossfilter.h"

float LossFilter::_filterTime = 300;

LossFilter::LossFilter() {
    _isInitialized = false;
}

void LossFilter::startLoss() {
    _isInitialized = true;
    _timer.start();
}

bool LossFilter::isInitialized() {
    return _isInitialized;
}

bool LossFilter::checkLoss() {
    _timer.stop();

    if(_timer.getMiliSeconds() >= getLossTime()) {
        return true;
    }
    else {
        return false;
    }
}

float LossFilter::getLossTime() {
    return _filterTime;
}

void LossFilter::setLossTime(float lossTime) {
    _filterTime = lossTime;
}
