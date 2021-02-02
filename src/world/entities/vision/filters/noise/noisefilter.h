#ifndef NOISEFILTER_H
#define NOISEFILTER_H

#include <src/utils/timer/timer.h>

class NoiseFilter
{
public:
    NoiseFilter();

    // Noise control
    void startNoise();
    bool isInitialized();
    bool checkNoise();

    // Getters
    float getNoiseTime();

    // Setters
    static void setNoiseTime(float noiseTime);

private:
    // Timer
    Timer _timer;

    // Params
    bool _isInitialized;
    static float _filterTime;
};

#endif // NOISEFILTER_H
