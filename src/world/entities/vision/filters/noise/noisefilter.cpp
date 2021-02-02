#include "noisefilter.h"

float NoiseFilter::_filterTime = 300;

NoiseFilter::NoiseFilter() {
    _isInitialized = false;
}

void NoiseFilter::startNoise() {
    _isInitialized = true;
    _timer.start();
}

bool NoiseFilter::isInitialized() {
    return _isInitialized;
}

bool NoiseFilter::checkNoise() {
    _timer.stop();

    if(_timer.getMiliSeconds() >= getNoiseTime()) {
        return true;
    }
    else {
        return false;
    }
}

float NoiseFilter::getNoiseTime() {
    return _filterTime;
}

void NoiseFilter::setNoiseTime(float noiseTime) {
    _filterTime = noiseTime;
}
