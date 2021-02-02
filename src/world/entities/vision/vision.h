#ifndef VISION_H
#define VISION_H

#include <src/world/entities/entity.h>

class Vision : public Entity
{
public:
    Vision();

private:
    // Entity inherited methods
    void initialization();
    void loop();
    void finalization();
};

#endif // VISION_H
