#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <QString>

class Timer
{
public:
    Timer();

    // Timer control
    void start();
    void stop();

    // Getters
    double getSeconds();
    double getMiliSeconds();
    double getMicroSeconds();
    double getNanoSeconds();
    static QString getActualTime();
    static qint64 systemTime();

private:
    //timespec _time1;
    //timespec _time2;

    std::chrono::high_resolution_clock::time_point _time1;
    std::chrono::high_resolution_clock::time_point _time2;
};

#endif // TIMER_H
