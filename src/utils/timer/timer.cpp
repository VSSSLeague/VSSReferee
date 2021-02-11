#include "timer.h"

Timer::Timer() {
    // Updating time1 and time2 with actual time
    _time1 = std::chrono::high_resolution_clock::now();
    _time2 = std::chrono::high_resolution_clock::now();
}

void Timer::start() {
    // Updating time1 with last time
    _time1 = std::chrono::high_resolution_clock::now();
}

void Timer::stop() {
    // Updating time2 with last time
    _time2 = std::chrono::high_resolution_clock::now();
}

double Timer::getSeconds() {
    return (getNanoSeconds() / 1E9);
}

double Timer::getMiliSeconds() {
    return (getNanoSeconds() / 1E6);
}

double Timer::getMicroSeconds() {
    return (getNanoSeconds() / 1E3);
}

double Timer::getNanoSeconds() {
    auto passedTime = std::chrono::duration_cast<std::chrono::nanoseconds>(_time2 - _time1);
    return (passedTime.count());
}
