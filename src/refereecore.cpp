#include "refereecore.h"

RefereeCore::RefereeCore(Constants *constants) {
    // Taking constants
    _constants = constants;

    // Creating world pointer
    _world = new World(getConstants());
}

RefereeCore::~RefereeCore() {
    // Deleting world module
    delete _world;
}

void RefereeCore::start() {
    // Creating vision pointer and adding it to world with priority 0
    _vision = new Vision(getConstants());
    _world->addEntity(_vision, 0);

    // Starting entities
    _world->startEntities();
}

void RefereeCore::stop() {
    // Stopping and deleting entities
    _world->stopAndDeleteEntities();
}

Constants* RefereeCore::getConstants() {
    if(_constants == nullptr) {
        std::cout << Text::red("[ERROR] ", true) << Text::bold("Constants with nullptr value at RefereeCore") + '\n';
    }
    else {
        return _constants;
    }

    return nullptr;
}
