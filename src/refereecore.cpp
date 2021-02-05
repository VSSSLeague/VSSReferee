#include "refereecore.h"

RefereeCore::RefereeCore(Constants *constants) {
    // Taking constants
    _constants = constants;

    // Creating world pointer
    _world = new World(getConstants());

    // Register Referee metatypes
    qRegisterMetaType<VSSRef::Color>("VSSRef::Color");
    qRegisterMetaType<VSSRef::Foul>("VSSRef::Foul");
    qRegisterMetaType<VSSRef::Quadrant>("VSSRef::Quadrant");
}

RefereeCore::~RefereeCore() {
    // Deleting world module
    delete _world;
}

void RefereeCore::start() {
    // Setup utils
    Utils::setConstants(getConstants());

    // Creating vision pointer and adding it to world with priority 0
    _vision = new Vision(getConstants());
    _world->addEntity(_vision, 0);

    // Creating replacer pointer
    _replacer = new Replacer(_vision, getConstants());

    // Creating referee pointer and adding it to world with priority 1
    _referee = new Referee(_vision, _replacer, getConstants());
    _world->addEntity(_referee, 1);

    // Adding replacer to world with prio 2
    _world->addEntity(_replacer, 2);

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
