#ifndef UTILS_H
#define UTILS_H

#include <include/vssref_common.pb.h>
#include <src/constants/constants.h>
#include <src/utils/types/position/position.h>
#include <src/utils/types/field/field.h>

class Utils
{
public:
    static float scalarProduct(const Position &A, const Position &B);
    static float distance(const Position &a, const Position &b);
    static bool isPointAtLine(const Position &s1, const Position &s2, const Position &point);
    static bool isPointAtSegment(const Position &s1, const Position &s2, const Position &point);
    static Position projectPointAtLine(const Position &s1, const Position &s2, const Position &point);
    static Position projectPointAtSegment(const Position &s1, const Position &s2, const Position &point);
    static float distanceToLine(const Position &s1, const Position &s2, const Position &point);
    static float distanceToSegment(const Position &s1, const Position &s2, const Position &point);
    static bool isInsideGoalArea(VSSRef::Color teamColor, Position pos);
    static bool isInsideBigArea(VSSRef::Color teamColor, Position pos);
    static bool isBallInsideGoal(VSSRef::Color teamColor, Position pos);
    static Position rotatePoint(Position point, float angle);
    static VSSRef::Quadrant getBallQuadrant(Position ballPos);
    static float getAngle(const Position &a, const Position &b);
    static void setConstants(Constants *constants);
    static void setField(Field *field);

private:
    static Constants *_constants;
    static Constants* getConstants();

    static Field *_field;
    static Field* getField();
};

#endif // UTILS_H
