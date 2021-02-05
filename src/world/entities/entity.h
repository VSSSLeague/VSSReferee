#ifndef ENTITY_H
#define ENTITY_H

#include <QObject>
#include <QThread>
#include <QMutex>

#include <src/utils/timer/timer.h>

enum EntityType {
    ENT_VISION,
    ENT_REFEREE,
    ENT_REPLACER,
    ENT_GUI
};

class Entity : public QThread
{
public:
    Entity(EntityType type);

    // Setters
    void setLoopFrequency(int hz);
    void setPriority(int priority);
    void enableEntity();
    void disableLoop();
    void stopEntity();

    // Getters
    int loopFrequency();
    int entityPriority();
    int entityId();
    bool isEnabled();
    bool isLoopEnabled();
    EntityType entityType();

private:
    // Main run method
    void run();

    // Virtual methods
    virtual void initialization() = 0;
    virtual void loop() = 0;
    virtual void finalization() = 0;

    // Entity info
    int _loopFrequency;
    int _entityPriority;
    bool _isEnabled;
    bool _loopEnabled;
    EntityType _entityType;
    static int _id;

    // Entity timer
    Timer _entityTimer;
    void startTimer();
    void stopTimer();
    long getRemainingTime();

    // Entity mutexes
    QMutex _mutexRunning;
    QMutex _mutexEnabled;
    QMutex _mutexPriority;
    QMutex _mutexLoopTime;
};

#endif // ENTITY_H
