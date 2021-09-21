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
    qRegisterMetaType<VSSRef::Half>("VSSRef::Half");
    qRegisterMetaType<Position>("Position");
    qRegisterMetaType<Velocity>("Velocity");
}

RefereeCore::~RefereeCore() {
    // Deleting world module
    delete _world;

    // Deleting GUI
    delete _soccerView;

    // Delete field
    delete _field;
}

void RefereeCore::start() {
    // Create field
    _field = new Field(getConstants()->is5v5());

    // Setup utils
    Utils::setConstants(getConstants());
    Utils::setField(_field);

    // Creating vision pointer and adding it to world with priority 2
    _vision = new Vision(getConstants());
    _world->addEntity(_vision, 2);

    // Creating GUI
    _soccerView = new SoccerView(getConstants());

    // Setting Vision and Constants to FieldView
    _soccerView->getFieldView()->setVisionModule(_vision);
    _soccerView->getFieldView()->setField(_field);
    _soccerView->getFieldView()->setConstants(_constants);

    // Creating replacer pointer
    _replacer = new Replacer(QString(PROJECT_PATH) + "/src/world/entities/replacer/replacements.json", _vision, _field, getConstants());

    // Creating referee pointer and adding it to world with priority 1
    _referee = new Referee(_vision, _replacer, _soccerView, getConstants());
    _world->addEntity(_referee, 1);

    // Adding replacer to world with prio 0
    _world->addEntity(_replacer, 0);

    // Make GUI connections with modules
    QObject::connect(_soccerView, SIGNAL(sendCollisionDecision()), _referee, SLOT(processCollisionDecision()));
    QObject::connect(_referee, SIGNAL(sendFoul(VSSRef::Foul, VSSRef::Color, VSSRef::Quadrant)), _soccerView, SLOT(takeFoul(VSSRef::Foul, VSSRef::Color, VSSRef::Quadrant)));
    QObject::connect(_referee, SIGNAL(sendTimestamp(float, float, VSSRef::Half, bool)), _soccerView, SLOT(takeTimeStamp(float, float, VSSRef::Half, bool)));
    QObject::connect(_soccerView, SIGNAL(sendManualFoul(VSSRef::Foul, VSSRef::Color, VSSRef::Quadrant, bool)), _referee, SLOT(takeManualFoul(VSSRef::Foul, VSSRef::Color, VSSRef::Quadrant, bool)));
    QObject::connect(_vision, SIGNAL(visionUpdated()), _soccerView->getFieldView(), SLOT(updateField()));

    // Show GUI
    _soccerView->show();

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
