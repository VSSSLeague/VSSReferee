#ifndef VISION_H
#define VISION_H

#include <src/world/entities/entity.h>
#include <src/constants/constants.h>

class Vision : public Entity
{
public:
    Vision(Constants *constants);

private:
    // Entity inherited methods
    void initialization();
    void loop();
    void finalization();

    // Constants
    Constants *_constants;
    Constants* getConstants();
};

#endif // VISION_H
