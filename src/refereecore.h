#ifndef REFEREECORE_H
#define REFEREECORE_H

#include <src/soccerview/soccerview.h>
#include <src/utils/utils.h>
#include <src/world/world.h>
#include <src/world/entities/vision/vision.h>
#include <src/world/entities/referee/referee.h>
#include <src/world/entities/replacer/replacer.h>

class RefereeCore
{
public:
    RefereeCore(Constants *constants);
    ~RefereeCore();

    // Internal
    void start();
    void stop();

private:
    // Modules
    World *_world;
    Vision *_vision;
    Referee *_referee;
    Replacer *_replacer;

    // GUI
    SoccerView *_soccerView;

    // Constants
    Constants *_constants;
    Constants* getConstants();
};

#endif // REFEREECORE_H
