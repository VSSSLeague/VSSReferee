#include "vision.h"

#include <src/utils/text/text.h>

Vision::Vision(Constants *constants) : Entity(ENT_VISION) {
    // Taking constants
    _constants = constants;
}

void Vision::initialization() {
    std::cout << Text::blue("[VISION] ", true) + Text::bold("Module started.") + '\n';
}

void Vision::loop() {

}

void Vision::finalization() {
    std::cout << Text::blue("[VISION] ", true) + Text::bold("Module finished.") + '\n';
}

Constants* Vision::getConstants() {
    if(_constants == nullptr) {
        std::cout << Text::red("[ERROR] ", true) << Text::bold("Constants with nullptr value at Vision") + '\n';
    }
    else {
        return _constants;
    }

    return nullptr;
}
