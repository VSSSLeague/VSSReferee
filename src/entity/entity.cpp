/***
 * Maracatronics Robotics
 * Federal University of Pernambuco (UFPE) at Recife
 * http://www.maracatronics.com/
 *
 * This file is part of Armorial project.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***/

#include "entity.h"

Entity::Entity(QObject *parent) : QThread(parent) {
    _running = true; // set running by default
    _loopTime = 10;
}

void Entity::run() {
    initialization();

    Timer t;
    while(isRunning())	{
        t.start();
        loop();
        t.stop();

        if(getLoopTime()>0) {
            long rest = getLoopTime() - t.timemsec();
            if(rest >= 0)
                msleep(rest);
        }
    }

    finalization();
}

void Entity::restart() {
    if(isRunning())
        return;

    _mutexRunning.lock();
    _running = true;
    _mutexRunning.unlock();

    this->start();
}

bool Entity::isRunning() {
    bool result;

    _mutexRunning.lock();
    result = _running;
    _mutexRunning.unlock();

    return result;
}

void Entity::stopRunning() {
    _mutexRunning.lock();
    _running = false;
    _mutexRunning.unlock();
}

void Entity::setLoopTime(int t) {
    _mutexLoopTime.lock();
    _loopTime = t;
    _mutexLoopTime.unlock();
}

void Entity::setLoopFrequency(int hz) {
    _mutexLoopTime.lock();
    if(hz==0)
        _loopTime = 0;
    else
        _loopTime = 1000/hz;
    _mutexLoopTime.unlock();
}

int Entity::getLoopTime() {
    _mutexLoopTime.lock();
    int result = _loopTime;
    _mutexLoopTime.unlock();

    return result;
}

int Entity::getLoopFrequency() {
    _mutexLoopTime.lock();
    int result = 1000/_loopTime;
    _mutexLoopTime.unlock();

    return result;
}
