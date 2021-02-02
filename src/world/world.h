#ifndef WORLD_H
#define WORLD_H

#include <QMap>
#include <QHash>

#include <src/world/entities/entity.h>

class World
{
public:
    World();

    // Entities management
    void addEntity(Entity *entity, int entityPriority);

    // Internal
    void startEntities();
    void stopAndDeleteEntities();

private:
    // Hashtable for entities
    QMap<int, QHash<int, Entity*>*> _worldEntities;
};

#endif // WORLD_H
