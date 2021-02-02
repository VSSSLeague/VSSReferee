#include "entity.h"

int Entity::_id = 0;

Entity::Entity(EntityType type) {
    _entityType = type;
    _id++;               // global id +1
    _entityPriority = 0; // default priority is 0
    _loopFrequency = 60; // default loop frequency is 60
    _isEnabled = true;   // enabling by default
    _loopEnabled = true; // enabling loop by default
}

void Entity::run(){
    initialization();

    while(isEnabled()) {
        startTimer();
        if(isLoopEnabled()) {
            loop();
        }
        stopTimer();

        long rest = getRemainingTime();
        if(rest >= 0) {
            msleep(rest);
        }
    }

    finalization();
}

int Entity::entityId() {
    return _id;
}

void Entity::setLoopFrequency(int hz) {
    _mutexLoopTime.lock();
    _loopFrequency = hz;
    _mutexLoopTime.unlock();
}

void Entity::setPriority(int priority) {
    _mutexPriority.lock();
    _entityPriority = priority;
    _mutexPriority.unlock();
}

void Entity::enableEntity() {
    _mutexEnabled.lock();
    _isEnabled = true;
    _mutexEnabled.unlock();
}

void Entity::disableLoop() {
    _mutexEnabled.lock();
    _loopEnabled = false;
    _mutexEnabled.unlock();
}

void Entity::stopEntity() {
    _mutexEnabled.lock();
    _isEnabled = false;
    _mutexEnabled.unlock();
}

int Entity::loopFrequency() {
    _mutexLoopTime.lock();
    int loopFrequency = _loopFrequency;
    _mutexLoopTime.unlock();

    return loopFrequency;
}

int Entity::entityPriority() {
    _mutexPriority.lock();
    int entityPriority = _entityPriority;
    _mutexPriority.unlock();

    return entityPriority;
}

bool Entity::isEnabled() {
    _mutexEnabled.lock();
    bool isEnabled = _isEnabled;
    _mutexEnabled.unlock();

    return isEnabled;
}

bool Entity::isLoopEnabled() {
    _mutexEnabled.lock();
    bool loopEnabled = _loopEnabled;
    _mutexEnabled.unlock();

    return loopEnabled;
}

EntityType Entity::entityType() {
    return _entityType;
}

void Entity::startTimer() {
    _entityTimer.start();
}

void Entity::stopTimer() {
    _entityTimer.stop();
}

long Entity::getRemainingTime() {
    long remainingTime = (1000 / _loopFrequency) - _entityTimer.getMiliSeconds();

    return remainingTime;
}
