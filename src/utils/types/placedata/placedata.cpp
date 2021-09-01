#include "placedata.h"

PlaceData::PlaceData() {
    _position = Position(false, 0.0, 0.0);
    _orientation = Angle(false, 0.0);
}

PlaceData::PlaceData(Position position, Angle orientation) {
    _position = position;
    _orientation = orientation;
}

Position PlaceData::getPosition() {
    return _position;
}

Angle PlaceData::getOrientation() {
    return _orientation;
}

void PlaceData::reflect() {
    _position = Position(true, (-1.0) * _position.x(), _position.y());
    _orientation = Angle(true, (-1.0) * _orientation.value());
}
