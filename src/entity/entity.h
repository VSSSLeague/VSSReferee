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

#ifndef ENTITY_H
#define ENTITY_H

#include <QThread>
#include <QMutex>
#include <iostream>
#include <include/timer.h>

class Entity : public QThread {
public:
    explicit Entity(QObject *parent = 0);

    // Loop time
    void setLoopTime(int t);
    void setLoopFrequency(int hz);
    int getLoopTime();
    int getLoopFrequency();

    // Run control
    void restart();
    bool isRunning();
    void stopRunning();

    // Entity name
    virtual QString name() = 0;
private:
    void run();

    // Thread methods -> this methods are not implemented and must be implemented by the inherited class
    virtual void initialization() = 0;
    virtual void loop() = 0;
    virtual void finalization() = 0;

    bool _running;
    int _loopTime;

    QMutex _mutexRunning;
    QMutex _mutexLoopTime;
};

#endif // ENTITY_H
