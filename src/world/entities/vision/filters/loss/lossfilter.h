#ifndef LOSSFILTER_H
#define LOSSFILTER_H

#include <src/utils/timer/timer.h>

class LossFilter
{
public:
    LossFilter();

    // Noise control
    void startLoss();
    bool isInitialized();
    bool checkLoss();

    // Getters
    float getLossTime();

    // Setters
    static void setLossTime(float lossTime);
private:
    // Timer
    Timer _timer;

    // Params
    bool _isInitialized;
    static float _filterTime;
};

#endif // LOSSFILTER_H
