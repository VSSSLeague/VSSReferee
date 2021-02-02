#include "refereecore.h"

RefereeCore::RefereeCore() {
    // Creating world pointer
    _world = new World();
}

RefereeCore::~RefereeCore() {
    // Deleting world module
    delete _world;
}

void RefereeCore::start() {
    // Creating vision pointer and adding it to world with priority 0
    _vision = new Vision();
    _world->addEntity(_vision, 0);

    // Starting entities
    _world->startEntities();
}

void RefereeCore::stop() {
    // Stopping and deleting entities
    _world->stopAndDeleteEntities();
}
