#ifndef REFEREECORE_H
#define REFEREECORE_H

#include <src/world/world.h>
#include <src/world/entities/vision/vision.h>

class RefereeCore
{
public:
    RefereeCore();
    ~RefereeCore();

    // Internal
    void start();
    void stop();

private:
    World *_world;
    Vision *_vision;
};

#endif // REFEREECORE_H
