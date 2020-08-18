#ifndef UTILS_H
#define UTILS_H

#include <src/entity/refereeview/refereeview.h>
#include <src/entity/refereeview/soccerview/util/geometry.h>
#include <src/entity/refereeview/soccerview/util/field_default_constants.h>
#include <include/vssref_common.pb.h>

#include <constants/constants.h>

class Utils
{
public:
    static float scalarProduct(const vector2d &A, const vector2d &B);
    static float distance(const vector2d &a, const vector2d &b);
    static bool isPointAtLine(const vector2d &s1, const vector2d &s2, const vector2d &point);
    static bool isPointAtSegment(const vector2d &s1, const vector2d &s2, const vector2d &point);
    static vector2d projectPointAtLine(const vector2d &s1, const vector2d &s2, const vector2d &point);
    static vector2d projectPointAtSegment(const vector2d &s1, const vector2d &s2, const vector2d &point);
    static float distanceToLine(const vector2d &s1, const vector2d &s2, const vector2d &point);
    static float distanceToSegment(const vector2d &s1, const vector2d &s2, const vector2d &point);
    static bool isInsideGoalArea(VSSRef::Color teamColor, vector2d pos);
    static bool isBallInsideGoal(VSSRef::Color teamColor, vector2d pos);
    static vector2d rotatePoint(vector2d point, float angle);
    static VSSRef::Quadrant getBallQuadrant(vector2d ballPos);
    static void setConstants(Constants *constants);

private:
    static Constants *_constants;
    static Constants* getConstants();
};

#endif // UTILS_H
