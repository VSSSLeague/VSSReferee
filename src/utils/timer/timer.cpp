#include "timer.h"

#include <QDateTime>

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

QString Timer::getActualTime() {
    time_t now = time(0);
    tm *ltm = localtime(&now);

    char str[100];
    sprintf(str, "%04d-%02d-%02d|%02d:%02d:%02d", 1900+ltm->tm_year, 1+ltm->tm_mon, ltm->tm_mday,
                                                  ltm->tm_hour, ltm->tm_min, ltm->tm_sec);

    QString actualTime(str);

    return actualTime;
}

qint64 Timer::systemTime() {
    return QDateTime::currentMSecsSinceEpoch();
}
