#ifndef WORLD_H
#define WORLD_H

#include <QMap>
#include <QHash>

#include <src/world/entities/entity.h>
#include <src/constants/constants.h>

class World
{
public:
    World(Constants *constants);

    // Entities management
    void addEntity(Entity *entity, int entityPriority);

    // Internal
    void startEntities();
    void stopAndDeleteEntities();

private:
    // Hashtable for entities
    QMap<int, QHash<int, Entity*>*> _worldEntities;

    // Constants
    Constants *_constants;
    Constants* getConstants();
};

#endif // WORLD_H
