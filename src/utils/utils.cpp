#include "utils.h"
#include <math.h>

Constants* Utils::_constants = nullptr;
Field* Utils::_field = nullptr;

float Utils::distance(const Position &a, const Position &b) {
    return sqrt(pow(a.x() - b.x() ,2) + pow(a.y() - b.y(), 2));
}

bool Utils::isPointAtLine(const Position &s1, const Position &s2, const Position &point) {
    const Position projectedPoint = Utils::projectPointAtLine(s1, s2, point);
    const float dist = Utils::distance(point, projectedPoint);

    return (dist <= 0.001f);
}

float Utils::scalarProduct(const Position &A, const Position &B) {
    return (A.x() * B.x() + A.y() * B.y());
}

bool Utils::isPointAtSegment(const Position &s1, const Position &s2, const Position &point) {
    const bool isAtLine = Utils::isPointAtLine(s1, s2, point);
    if(isAtLine == false) {
        return false;
    }
    else {
        Position min, max;
        if(s2.x() >= s1.x()) {
            min = Position(true, s1.x(), min.y());
            max = Position(true, s2.x(), max.y());
        } else {
            min = Position(true, s2.x(), min.y());
            max = Position(true, s1.x(), max.y());
        }
        if(s2.y() >= s1.y()) {
            min = Position(true, min.x(), s1.y());
            max = Position(true, max.x(), s2.y());
        } else {
            min = Position(true, min.x(), s2.y());
            max = Position(true, max.x(), s1.y());
        }
        return (point.x() >= min.x() && point.x() <= max.x() && point.y() >= min.y() && point.y() <= max.y());
    }
}

Position Utils::projectPointAtLine(const Position &s1, const Position &s2, const Position &point) {
    const Position a(true, point.x() - s1.x(), point.y() - s1.y());
    const Position b(true, s2.x() - s1.x(), s2.y() - s1.y());
    const float bModule = sqrt(pow(b.x(), 2) + pow(b.y(), 2));
    const Position bUnitary(true, b.x() / bModule, b.y() / bModule);
    const float scalar = Utils::scalarProduct(a, bUnitary);

    return Position(true, s1.x() + scalar * bUnitary.x(), s1.y() + scalar * bUnitary.y());
}

Position Utils::projectPointAtSegment(const Position &s1, const Position &s2, const Position &point) {
    const Position projectedPoint = Utils::projectPointAtLine(s1, s2, point);
    if(Utils::isPointAtSegment(s1, s2, projectedPoint)) {
        return projectedPoint;
    } else {
        const float d1 = Utils::distance(projectedPoint, s1);
        const float d2 = Utils::distance(projectedPoint, s2);
        return (d1 <= d2) ? s1 : s2;
    }
}

float Utils::distanceToLine(const Position &s1, const Position &s2, const Position &point) {
    const Position projectedPoint = Utils::projectPointAtLine(s1, s2, point);
    const float distance = Utils::distance(point, projectedPoint);

    return (distance <= 0.001f) ? 0 : distance;
}

float Utils::distanceToSegment(const Position &s1, const Position &s2, const Position &point) {
    const Position projectedPoint = Utils::projectPointAtLine(s1, s2, point);
    if(Utils::isPointAtSegment(s1, s2, projectedPoint)) {
        return distanceToLine(s1, s2, point);
    } else {
        const float d1 = Utils::distance(point, s1);
        const float d2 = Utils::distance(point, s2);
        return (d1 <= d2)? d1 : d2;
    }
}

bool Utils::isInsideGoalArea(VSSRef::Color teamColor, Position pos){
    float goal_x = (getField()->fieldLength()/2.0 - getField()->defenseRadius()) / 1000.0;
    float goal_y = (getField()->defenseStretch() / 2.0) / 1000.0;

    if(teamColor == VSSRef::Color::BLUE){
        if(getConstants()->blueIsLeftSide()){
            if(pos.x() < -goal_x && abs(pos.y()) < goal_y)
                return true;
        }
        else{
            if(pos.x() > goal_x && abs(pos.y()) < goal_y)
                return true;
        }
    }
    else if(teamColor == VSSRef::Color::YELLOW){
        if(getConstants()->blueIsLeftSide()){
            if(pos.x() > goal_x && abs(pos.y()) < goal_y)
                return true;
        }
        else{
            if(pos.x() < -goal_x && abs(pos.y()) < goal_y)
                return true;
        }
    }

    return false;
}

bool Utils::isInsideBigArea(VSSRef::Color teamColor, Position pos) {
    if(!getConstants()->is5v5()) return false;

    float goal_x = (getField()->fieldLength()/2.0 - getField()->bigDefenseRadius()) / 1000.0;
    float goal_y = (getField()->bigDefenseStretch() / 2.0) / 1000.0;

    if(teamColor == VSSRef::Color::BLUE){
        if(getConstants()->blueIsLeftSide()){
            if(pos.x() < -goal_x && abs(pos.y()) < goal_y)
                return true;
        }
        else{
            if(pos.x() > goal_x && abs(pos.y()) < goal_y)
                return true;
        }
    }
    else if(teamColor == VSSRef::Color::YELLOW){
        if(getConstants()->blueIsLeftSide()){
            if(pos.x() > goal_x && abs(pos.y()) < goal_y)
                return true;
        }
        else{
            if(pos.x() < -goal_x && abs(pos.y()) < goal_y)
                return true;
        }
    }

    return false;
}

bool Utils::isBallInsideGoal(VSSRef::Color teamColor, Position pos) {
    float goal_x = (getField()->fieldLength()/2.0) / 1000.0 + getConstants()->ballRadius();
    float goal_y = (getField()->defenseStretch() / 2.0) / 1000.0;

    if(teamColor == VSSRef::Color::BLUE){
        if(getConstants()->blueIsLeftSide()){
            if(pos.x() < -goal_x && abs(pos.y()) < goal_y)
                return true;
        }
        else{
            if(pos.x() > goal_x && abs(pos.y()) < goal_y)
                return true;
        }
    }
    else if(teamColor == VSSRef::Color::YELLOW){
        if(getConstants()->blueIsLeftSide()){
            if(pos.x() > goal_x && abs(pos.y()) < goal_y)
                return true;
        }
        else{
            if(pos.x() < -goal_x && abs(pos.y()) < goal_y)
                return true;
        }
    }

    return false;
}

Position Utils::rotatePoint(Position point, float angle){
    float xNew = point.x() * cos(angle) - point.y() * sin(angle);
    float yNew = point.x() * sin(angle) - point.y() * cos(angle);

    return Position(true, xNew, yNew);
}

VSSRef::Quadrant Utils::getBallQuadrant(Position ballPos){
    /// Quadrants
    // Quadrant 1 (top-right)
    // Quadrant 2 (top-left)
    // Quadrant 3 (bot-left)
    // Quadrant 4 (bot-right)

    if(ballPos.x() > 0.0 && ballPos.y() > 0.0)
        return VSSRef::Quadrant::QUADRANT_1;
    else if(ballPos.x() <= 0.0 && ballPos.y() > 0.0)
        return VSSRef::Quadrant::QUADRANT_2;
    else if(ballPos.x() <= 0.0 && ballPos.y() <= 0.0)
        return VSSRef::Quadrant::QUADRANT_3;
    else if(ballPos.x() > 0.0 && ballPos.y() <= 0.0)
        return VSSRef::Quadrant::QUADRANT_4;

    return VSSRef::Quadrant::NO_QUADRANT;
}

float Utils::getAngle(const Position &a, const Position &b)	{
    return atan2(b.y()-a.y(), b.x()-a.x());
}

void Utils::setConstants(Constants *constants){
    _constants = constants;
}

void Utils::setField(Field *field) {
    _field = field;
}

Constants* Utils::getConstants() {
    if(_constants == nullptr) {
        std::cout << Text::red("[ERROR] ", true) << Text::bold("Constants with nullptr value at Utils") + '\n';
    }
    else {
        return _constants;
    }

    return nullptr;
}

Field* Utils::getField() {
    if(_field == nullptr) {
        std::cout << Text::red("[ERROR] ", true) << Text::bold("Field with nullptr value at Utils") + '\n';
    }
    else {
        return _field;
    }

    return nullptr;
}
