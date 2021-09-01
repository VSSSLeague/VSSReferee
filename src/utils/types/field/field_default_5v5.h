#ifndef FIELD_DEFAULT_5V5_H
#define FIELD_DEFAULT_5V5_H

#include <src/utils/types/field/field.h>
#include <math.h>

namespace Field_Default_5v5 {
    const double kFieldLength = 2200.0;
    const double kFieldWidth = 1800.0;
    const double kGoalWidth = 400.0;
    const double kGoalDepth = 150.0;
    const double kBoundaryWidth = 400.0; //?

    const double kCenterRadius = 250.0;
    const double kDefenseRadius = 150.0;
    const double kDefenseStretch = 500.0;

    const double kBigDefenseRadius = 375.0;
    const double kBigDefenseStretch = 800.0;
    const double kLineThickness = 3.0;
    
    const double kXMax = kFieldLength/2;
    const double kXMin = -kXMax;
    const double kYMax = kFieldWidth/2;
    const double kYMin = -kYMax;
    const double kFieldCorner = kFieldLength*5/150;
    const double kMarkDistanceX = kFieldLength/4;
    const double kMarkDistanceY = 4*kFieldLength/15;
    const double kMarkLength = kFieldLength*5/150;
    const double kMarkCircleDistance = 2*kFieldLength/15;
    const double kMarkCircleRadius = kMarkLength/10;

    const double kFieldFBMarkX = 550.0;
    const double kFieldFBMarkY = 300.0;
    const double kRobotFBDistance = 0.3;

    const std::size_t kNumFieldLines = 29;

    const std::size_t kNumLeftGoalLines = 3;
    const FieldLine kLeftGoalLines[kNumLeftGoalLines] = {
        FieldLine("LeftGoalStretch", kXMin-kGoalDepth, -kGoalWidth/2, kXMin-kGoalDepth, kGoalWidth/2, kLineThickness),
        FieldLine("LeftGoalLeftLine", kXMin, kGoalWidth/2, kXMin-kGoalDepth-kLineThickness/2, kGoalWidth/2, kLineThickness),
        FieldLine("LeftGoalRightLine", kXMin, -kGoalWidth/2, kXMin-kGoalDepth-kLineThickness/2, -kGoalWidth/2, kLineThickness)
    };

    const std::size_t kNumRightGoalLines = 3;
    const FieldLine kRightGoalLines[kNumRightGoalLines] = {
        FieldLine("RightGoalStretch", kXMax+kGoalDepth, -kGoalWidth/2, kXMax+kGoalDepth, kGoalWidth/2, kLineThickness),
        FieldLine("RightGoalLeftLine", kXMax, kGoalWidth/2, kXMax+kGoalDepth+kLineThickness/2, kGoalWidth/2, kLineThickness),
        FieldLine("RightGoalRightLine", kXMax, -kGoalWidth/2, kXMax+kGoalDepth+kLineThickness/2, -kGoalWidth/2, kLineThickness),
    };

    const FieldLine kFieldLines[kNumFieldLines] = {
      FieldLine("LeftGoalLine", kXMin, kYMin, kXMin, kYMax, kLineThickness),
      FieldLine("RightGoalLine", kXMax, kYMin, kXMax, kYMax, kLineThickness),
      FieldLine("TopTouchLine", kXMin-kLineThickness/2, kYMax, kXMax+kLineThickness/2, kYMax, kLineThickness),
      FieldLine("BottomTouchLine", kXMin-kLineThickness/2, kYMin, kXMax+kLineThickness/2, kYMin, kLineThickness),
      FieldLine("HalfwayLine", 0, kYMin, 0, kYMax, kLineThickness),
      FieldLine("LeftPenaltyStretch", kXMin+kDefenseRadius, -kDefenseStretch/2, kXMin+kDefenseRadius, kDefenseStretch/2, kLineThickness),
      FieldLine("RightPenaltyStretch", kXMax-kDefenseRadius, -kDefenseStretch/2, kXMax-kDefenseRadius, kDefenseStretch/2, kLineThickness),
      FieldLine("LeftBigPenaltyStretch", kXMin+kBigDefenseRadius, -kBigDefenseStretch/2, kXMin+kBigDefenseRadius, kBigDefenseStretch/2, kLineThickness),
      FieldLine("RightBigPenaltyStretch", kXMax-kBigDefenseRadius, -kBigDefenseStretch/2, kXMax-kBigDefenseRadius, kBigDefenseStretch/2, kLineThickness),
      FieldLine("LeftFieldLeftDefenseLine", kXMin, kDefenseStretch/2, kXMin+kDefenseRadius+kLineThickness/2, kDefenseStretch/2, kLineThickness),
      FieldLine("LeftFieldRightDefenseLine", kXMin, -kDefenseStretch/2, kXMin+kDefenseRadius+kLineThickness/2, -kDefenseStretch/2, kLineThickness),
      FieldLine("RightFieldLeftDefenseLine", kXMax, kDefenseStretch/2, kXMax-kDefenseRadius-kLineThickness/2, kDefenseStretch/2, kLineThickness),
      FieldLine("RightFieldRightDefenseLine", kXMax, -kDefenseStretch/2, kXMax-kDefenseRadius-kLineThickness/2, -kDefenseStretch/2, kLineThickness),   
      FieldLine("LeftFieldLeftBigDefenseLine", kXMin, kBigDefenseStretch/2, kXMin+kBigDefenseRadius+kLineThickness/2, kBigDefenseStretch/2, kLineThickness),
      FieldLine("LeftFieldRightBigDefenseLine", kXMin, -kBigDefenseStretch/2, kXMin+kBigDefenseRadius+kLineThickness/2, -kBigDefenseStretch/2, kLineThickness),
      FieldLine("RightFieldLeftBigDefenseLine", kXMax, kBigDefenseStretch/2, kXMax-kBigDefenseRadius-kLineThickness/2, kBigDefenseStretch/2, kLineThickness),
      FieldLine("RightFieldRightBigDefenseLine", kXMax, -kBigDefenseStretch/2, kXMax-kBigDefenseRadius-kLineThickness/2, -kBigDefenseStretch/2, kLineThickness),        
      FieldLine("FBRightTopH", kMarkDistanceX-kMarkLength/2, kMarkDistanceY, kMarkDistanceX+kMarkLength/2, kMarkDistanceY, kLineThickness),
      FieldLine("FBRightTopV", kMarkDistanceX, kMarkDistanceY-kMarkLength/2, kMarkDistanceX, kMarkDistanceY+kMarkLength/2, kLineThickness),
      FieldLine("FBRightBottomH", kMarkDistanceX-kMarkLength/2, -kMarkDistanceY, kMarkDistanceX+kMarkLength/2, -kMarkDistanceY, kLineThickness),
      FieldLine("FBRightBottomV", kMarkDistanceX, -kMarkDistanceY-kMarkLength/2, kMarkDistanceX, -kMarkDistanceY+kMarkLength/2, kLineThickness),
      FieldLine("FBRightTopH", -kMarkDistanceX-kMarkLength/2, kMarkDistanceY, -kMarkDistanceX+kMarkLength/2, kMarkDistanceY, kLineThickness),
      FieldLine("FBRightTopV", -kMarkDistanceX, kMarkDistanceY-kMarkLength/2, -kMarkDistanceX, kMarkDistanceY+kMarkLength/2, kLineThickness),
      FieldLine("FBLeftBottomH", -kMarkDistanceX-kMarkLength/2, -kMarkDistanceY, -kMarkDistanceX+kMarkLength/2, -kMarkDistanceY, kLineThickness),
      FieldLine("FBLeftBottomV", -kMarkDistanceX, -kMarkDistanceY-kMarkLength/2, -kMarkDistanceX, -kMarkDistanceY+kMarkLength/2, kLineThickness),
      FieldLine("PKRightH", kMarkDistanceX-kMarkLength/2, 0, kMarkDistanceX+kMarkLength/2, 0, kLineThickness),
      FieldLine("PKRightV", kMarkDistanceX, -kMarkLength/2, kMarkDistanceX, kMarkLength/2, kLineThickness),
      FieldLine("PKLeftH", -kMarkDistanceX-kMarkLength/2, 0, -kMarkDistanceX+kMarkLength/2, 0, kLineThickness),
      FieldLine("PKLeftV", -kMarkDistanceX, -kMarkLength/2, -kMarkDistanceX, kMarkLength/2, kLineThickness),
    };

    const std::size_t kNumFieldArcs = 11;

    const FieldCircularArc kFieldArcs[kNumFieldArcs] = {
      FieldCircularArc("CenterCircle", 0, 0, kCenterRadius-kLineThickness/2, 0, 2*M_PI, kLineThickness),
      FieldCircularArc("FBRightTopLeft", kMarkDistanceX-kMarkCircleDistance, kMarkDistanceY, kMarkCircleRadius, 0, 2*M_PI, 2*kMarkCircleRadius),
      FieldCircularArc("FBRightTopRight", kMarkDistanceX+kMarkCircleDistance, kMarkDistanceY, kMarkCircleRadius, 0, 2*M_PI, 2*kMarkCircleRadius),
      FieldCircularArc("FBRightBottomLeft", kMarkDistanceX-kMarkCircleDistance, -kMarkDistanceY, kMarkCircleRadius, 0, 2*M_PI, 2*kMarkCircleRadius),
      FieldCircularArc("FBRightBottomRight", kMarkDistanceX+kMarkCircleDistance, -kMarkDistanceY, kMarkCircleRadius, 0, 2*M_PI, 2*kMarkCircleRadius),
      FieldCircularArc("FBLeftTopLeft", -kMarkDistanceX-kMarkCircleDistance, kMarkDistanceY, kMarkCircleRadius, 0, 2*M_PI, 2*kMarkCircleRadius),
      FieldCircularArc("FBLeftTopRight", -kMarkDistanceX+kMarkCircleDistance, kMarkDistanceY, kMarkCircleRadius, 0, 2*M_PI, 2*kMarkCircleRadius),
      FieldCircularArc("FBLeftBottomLeft", -kMarkDistanceX-kMarkCircleDistance, -kMarkDistanceY, kMarkCircleRadius, 0, 2*M_PI, 2*kMarkCircleRadius),
      FieldCircularArc("FBLeftBottomRight", -kMarkDistanceX+kMarkCircleDistance, -kMarkDistanceY, kMarkCircleRadius, 0, 2*M_PI, 2*kMarkCircleRadius),
      FieldCircularArc("GoalRightArc", kXMax-kBigDefenseRadius/2-120, 0, 110, M_PI-0.9272, M_PI+0.9272, kLineThickness),
      FieldCircularArc("GoalLeftArc", kXMin+kBigDefenseRadius/2+120, 0, 110, -0.9272, +0.9272, kLineThickness),
    };

    const std::size_t kNumFieldTriangles = 4;

    const FieldTriangle kFieldTriangles[kNumFieldTriangles] = {
      FieldTriangle("RightTop", kXMax-kFieldCorner, kYMax, kXMax, kYMax-kFieldCorner, kXMax, kYMax, kLineThickness),
      FieldTriangle("RightBottom", kXMax-kFieldCorner, -kYMax, kXMax, -kYMax+kFieldCorner, kXMax, -kYMax, kLineThickness),
      FieldTriangle("LeftTop", -kXMax+kFieldCorner, kYMax, -kXMax, kYMax-kFieldCorner, -kXMax, kYMax, kLineThickness),
      FieldTriangle("LeftBottom", -kXMax+kFieldCorner, -kYMax, -kXMax, -kYMax+kFieldCorner, -kXMax, -kYMax, kLineThickness),
    };

}


#endif // FIELD_DEFAULT_5V5_H
