#include "utils.h"

float Utils::distance(const vector2d &a, const vector2d &b) {
    return sqrt(pow(a.x - b.x ,2) + pow(a.y - b.y, 2));
}

bool Utils::isPointAtLine(const vector2d &s1, const vector2d &s2, const vector2d &point) {
    const vector2d projectedPoint = Utils::projectPointAtLine(s1, s2, point);
    const float dist = Utils::distance(point, projectedPoint);
    return (dist<=0.001f);
}

float Utils::scalarProduct(const vector2d &A, const vector2d &B) {
    return A.x * B.x + A.y * B.y;
}

bool Utils::isPointAtSegment(const vector2d &s1, const vector2d &s2, const vector2d &point) {
    const bool isAtLine = Utils::isPointAtLine(s1, s2, point);
    if(isAtLine==false)
        return false;
    else {
        vector2d min, max;
        if(s2.x>=s1.x) {
            min = vector2d(s1.x, min.y);
            max = vector2d(s2.x, max.y);
        } else {
            min = vector2d(s2.x, min.y);
            max = vector2d(s1.x, max.y);
        }
        if(s2.y >= s1.y) {
            min = vector2d(min.x, s1.y);
            max = vector2d(max.x, s2.y);
        } else {
            min = vector2d(min.x, s2.y);
            max = vector2d(max.x, s1.y);
        }
        return (point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y);
    }
}

vector2d Utils::projectPointAtLine(const vector2d &s1, const vector2d &s2, const vector2d &point) {
    const vector2d a(point.x - s1.x, point.y - s1.y);
    const vector2d b(s2.x - s1.x, s2.y - s1.y);
    const float bModule = sqrt(pow(b.x, 2) + pow(b.y, 2));
    const vector2d bUnitary(b.x / bModule, b.y / bModule);
    const float scalar = Utils::scalarProduct(a, bUnitary);
    return vector2d(s1.x + scalar * bUnitary.x, s1.y + scalar * bUnitary.y);
}

vector2d Utils::projectPointAtSegment(const vector2d &s1, const vector2d &s2, const vector2d &point) {
    const vector2d projectedPoint = Utils::projectPointAtLine(s1, s2, point);
    if(Utils::isPointAtSegment(s1, s2, projectedPoint))
        return projectedPoint;
    else {
        const float d1 = Utils::distance(projectedPoint, s1);
        const float d2 = Utils::distance(projectedPoint, s2);
        return (d1<=d2)? s1 : s2;
    }
}

float Utils::distanceToLine(const vector2d &s1, const vector2d &s2, const vector2d &point) {
    const vector2d projectedPoint = Utils::projectPointAtLine(s1, s2, point);
    const float distance = Utils::distance(point, projectedPoint);
    return (distance<=0.001f)? 0 : distance;
}

float Utils::distanceToSegment(const vector2d &s1, const vector2d &s2, const vector2d &point) {
    const vector2d projectedPoint = Utils::projectPointAtLine(s1, s2, point);
    if(Utils::isPointAtSegment(s1, s2, projectedPoint))
        return distanceToLine(s1, s2, point);
    else {
        const float d1 = Utils::distance(point, s1);
        const float d2 = Utils::distance(point, s2);
        return (d1<=d2)? d1 : d2;
    }
}

bool Utils::isInsideGoalArea(VSSRef::Color teamColor, vector2d pos){
    float goal_x = (FieldConstantsVSS::kFieldLength/2.0 - FieldConstantsVSS::kDefenseRadius) / 1000.0;
    float goal_y = (FieldConstantsVSS::kDefenseStretch / 2.0) / 1000.0;

    if(teamColor == VSSRef::Color::BLUE){
        if(RefereeView::getBlueIsLeftSide()){
            if(pos.x < -goal_x && abs(pos.y) < goal_y)
                return true;
        }
        else{
            if(pos.x > goal_x && abs(pos.y) < goal_y)
                return true;
        }
    }
    else if(teamColor == VSSRef::Color::YELLOW){
        if(RefereeView::getBlueIsLeftSide()){
            if(pos.x > goal_x && abs(pos.y) < goal_y)
                return true;
        }
        else{
            if(pos.x < -goal_x && abs(pos.y) < goal_y)
                return true;
        }
    }

    return false;
}
