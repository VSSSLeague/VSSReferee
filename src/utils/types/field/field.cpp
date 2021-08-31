#include <src/utils/types/field/field_default_3v3.h>
#include <src/utils/types/field/field_default_5v5.h>
#include "field.h"

Field::Field(bool is5v5) {
    if(!is5v5) {
        _fieldLength = Field_Default_3v3::kFieldLength;
        _fieldWidth = Field_Default_3v3::kFieldWidth;
        _goalWidth = Field_Default_3v3::kGoalWidth;
        _goalDepth = Field_Default_3v3::kGoalDepth;
        _centerRadius = Field_Default_3v3::kCenterRadius;
        _defenseRadius = Field_Default_3v3::kDefenseRadius;
        _defenseStretch = Field_Default_3v3::kDefenseStretch;
        _bigDefenseRadius = -1;
        _bigDefenseStretch = -1;
        _boundaryWidth = Field_Default_3v3::kBoundaryWidth;

        for(size_t i = 0; i < Field_Default_3v3::kNumLeftGoalLines; i++) {
            _leftGoalLines.push_back(Field_Default_3v3::kLeftGoalLines[i]);
        }

        for(size_t i = 0; i < Field_Default_3v3::kNumRightGoalLines; i++) {
            _rightGoalLines.push_back(Field_Default_3v3::kRightGoalLines[i]);
        }

        for(size_t i = 0; i < Field_Default_3v3::kNumFieldLines; i++) {
            _fieldLines.push_back(Field_Default_3v3::kFieldLines[i]);
        }

        for(size_t i = 0; i < Field_Default_3v3::kNumFieldArcs; i++) {
            _fieldArcs.push_back(Field_Default_3v3::kFieldArcs[i]);
        }

        for(size_t i = 0; i < Field_Default_3v3::kNumFieldTriangles; i++) {
            _fieldTriangles.push_back(Field_Default_3v3::kFieldTriangles[i]);
        }
    }
    else {
        _fieldLength = Field_Default_5v5::kFieldLength;
        _fieldWidth = Field_Default_5v5::kFieldWidth;
        _goalWidth = Field_Default_5v5::kGoalWidth;
        _goalDepth = Field_Default_5v5::kGoalDepth;
        _centerRadius = Field_Default_5v5::kCenterRadius;
        _defenseRadius = Field_Default_5v5::kDefenseRadius;
        _defenseStretch = Field_Default_5v5::kDefenseStretch;
        _bigDefenseRadius = Field_Default_5v5::kBigDefenseRadius;
        _bigDefenseStretch = Field_Default_5v5::kBigDefenseStretch;
        _boundaryWidth = Field_Default_5v5::kBoundaryWidth;

        for(size_t i = 0; i < Field_Default_5v5::kNumLeftGoalLines; i++) {
            _leftGoalLines.push_back(Field_Default_5v5::kLeftGoalLines[i]);
        }

        for(size_t i = 0; i < Field_Default_5v5::kNumRightGoalLines; i++) {
            _rightGoalLines.push_back(Field_Default_5v5::kRightGoalLines[i]);
        }

        for(size_t i = 0; i < Field_Default_5v5::kNumFieldLines; i++) {
            _fieldLines.push_back(Field_Default_5v5::kFieldLines[i]);
        }

        for(size_t i = 0; i < Field_Default_5v5::kNumFieldArcs; i++) {
            _fieldArcs.push_back(Field_Default_5v5::kFieldArcs[i]);
        }

        for(size_t i = 0; i < Field_Default_5v5::kNumFieldTriangles; i++) {
            _fieldTriangles.push_back(Field_Default_5v5::kFieldTriangles[i]);
        }
    }
}

FieldLine::FieldLine() {
    name = "";
    p1_x = 0.0f;
    p1_y = 0.0f;
    p2_x = 0.0f;
    p2_y = 0.0f;
}

FieldLine::FieldLine(const FieldLine& other) {
    name = other.name;
    p1_x = other.p1_x;
    p1_y = other.p1_y;
    p2_x = other.p2_x;
    p2_y = other.p2_y;
    thickness = other.thickness;
}

FieldLine::FieldLine(const std::string &marking_name, double p1_x_, double p1_y_, double p2_x_, double p2_y_, double thickness_) {
    name.fromStdString(marking_name);
    p1_x = p1_x_;
    p1_y = p1_y_;
    p2_x = p2_x_;
    p2_y = p2_y_;
    thickness = thickness_;
}

FieldCircularArc::FieldCircularArc() {
    name = "";
    center_x = 0.0f;
    center_y = 0.0f;
    radius = 0.0f;
    a1 = 0.0f;
    a2 = 0.0f;
    thickness = 0.0f;
}

FieldCircularArc::FieldCircularArc(const FieldCircularArc& other) {
    name = other.name;
    center_x = other.center_x;
    center_y = other.center_y;
    radius = other.radius;
    a1 = other.a1;
    a2 = other.a2;
    thickness = other.thickness;
}

FieldCircularArc::FieldCircularArc(const std::string &marking_name, double center_x_, double center_y_, double radius_, double a1_, double a2_, double thickness_) {
    name.fromStdString(marking_name);
    center_x = center_x_;
    center_y = center_y_;
    radius = radius_;
    a1 = a1_;
    a2 = a2_;
    thickness = thickness_;
}

FieldTriangle::FieldTriangle() {
    name = "";
    p1_x = 0.0f;
    p1_y = 0.0f;
    p2_x = 0.0f;
    p2_y = 0.0f;
    p3_x = 0.0f;
    p3_y = 0.0f;
    thickness = 0.0f;
}

FieldTriangle::FieldTriangle(const FieldTriangle &other) {
    name = other.name;
    p1_x = other.p1_x;
    p1_y = other.p1_y;
    p2_x = other.p2_x;
    p2_y = other.p2_y;
    p3_x = other.p3_x;
    p3_y = other.p3_y;
    thickness = other.thickness;
}

FieldTriangle::FieldTriangle(const std::string &marking_name, double p1_x_, double p1_y_, double p2_x_, double p2_y_, double p3_x_, double p3_y_, double thickness_) {
    name.fromStdString(marking_name);
    p1_x = p1_x_;
    p1_y = p1_y_;
    p2_x = p2_x_;
    p2_y = p2_y_;
    p3_x = p3_x_;
    p3_y = p3_y_;
    thickness = thickness_;
}
