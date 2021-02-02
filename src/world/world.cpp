#include "world.h"

World::World() {

}

void World::addEntity(Entity *entity, int entityPriotity) {
    // Check if priority is already created
    if(!_worldEntities.keys().contains(entityPriotity)) {
        // Create it
        _worldEntities.insert(entityPriotity, new QHash<int, Entity*>());
    }

    // Take entities registered with 'entityPriority' priority
    QHash<int, Entity*> *prioEntities = _worldEntities.value(entityPriotity);

    // Inser them in hash using their id (unique in the software) as key
    prioEntities->insert(entity->entityId(), entity);

    // Set entity priority
    entity->setPriority(entityPriotity);
}

void World::startEntities() {
    // Get priorities in hash
    QList<int> priorities = _worldEntities.keys();

    // In each priority (decreasing, most priority first)
    const int prioritiesSize = priorities.size();
    for(int i = prioritiesSize - 1; i >= 0; i--) {
       const int priority = priorities.at(i);

       // Get associated entities
       const QList<Entity*> entities = _worldEntities.value(priority)->values();

       // Start those entities
       QList<Entity*>::const_iterator it;
       for(it = entities.constBegin(); it != entities.constEnd(); it++) {
           // Take entity
           Entity *entity = *it;

           // Set frequency
           /// TODO: create an constants class to get this frequency
           entity->setLoopFrequency(60);

           // Start entity
           entity->start();
       }
    }
}

void World::stopAndDeleteEntities() {
    // Get priorities in hash
    QList<int> priorities = _worldEntities.keys();

    // In each priority (increasing)
    const int prioritiesSize = priorities.size();
    for(int i = 0; i < prioritiesSize; i++) {
        const int priority = priorities.at(i);

        // Get associated entities
        const QList<Entity*> entities = _worldEntities.value(priority)->values();

        // Stop those entities
        QList<Entity*>::const_iterator it;
        for(it = entities.constBegin(); it != entities.constEnd(); it++) {
            // Take entity
            Entity *entity = *it;

            // Stop entity
            entity->stopEntity();

            // Wait entity
            entity->wait();
        }

        // Delete those entities
        for(it = entities.constBegin(); it != entities.constEnd(); it++) {
            // Take entity
            Entity *entity = *it;

            // Remove entity from hash
            _worldEntities.value(priority)->remove(entity->entityId());

            // Delete entity
            delete entity;
        }
    }
}
