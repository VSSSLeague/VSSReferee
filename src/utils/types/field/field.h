#ifndef FIELD_H
#define FIELD_H

#include <QString>
#include <QVector>
#include <src/constants/constants.h>

class FieldLine {
public:
    QString name;
    double p1_x;
    double p1_y;
    double p2_x;
    double p2_y;
    double thickness;

    FieldLine();
    FieldLine(const FieldLine& other);
    FieldLine(const std::string &marking_name, double p1_x_, double p1_y_, double p2_x_, double p2_y_, double thickness_);
};

class FieldCircularArc {
public:
    QString name;
    double center_x;
    double center_y;
    double radius;
    double a1;
    double a2;
    double thickness;

    FieldCircularArc();
    FieldCircularArc(const FieldCircularArc& other);
    FieldCircularArc(const std::string& marking_name, double center_x_, double center_y_, double radius_, double a1_, double a2_, double thickness_);
};

class FieldTriangle {
public:
    QString name;
    double p1_x;
    double p1_y;
    double p2_x;
    double p2_y;
    double p3_x;
    double p3_y;
    double thickness;

    FieldTriangle();
    FieldTriangle(const FieldTriangle& other);
    FieldTriangle(const std::string& marking_name, double p1_x_, double p1_y_, double p2_x_, double p2_y_, double p3_x_, double p3_y_, double thickness_);
};


class Field {
    public:
        Field(bool is5v5);
        double fieldLength() { return _fieldLength; }
        double fieldWidth() { return _fieldWidth; }
        double goalWidth() { return _goalWidth; }
        double goalDepth() { return _goalDepth; }
        double centerRadius() { return _centerRadius; }
        double defenseRadius() { return _defenseRadius; }
        double defenseStretch() { return _defenseStretch; }
        double bigDefenseRadius() { return _bigDefenseRadius; }
        double bigDefenseStretch() { return _bigDefenseStretch; }
        double boundaryWidth() { return _boundaryWidth; }
        double fieldFBMarkX() { return _kFieldFBMarkX; }
        double fieldFBMarkY() { return _kFieldFBMarkY; }
        QVector<FieldLine> leftGoalLines() { return _leftGoalLines; }
        QVector<FieldLine> rightGoalLines() { return _rightGoalLines; }
        QVector<FieldLine> fieldLines() { return _fieldLines; }
        QVector<FieldCircularArc> fieldArcs() { return _fieldArcs; }
        QVector<FieldTriangle> fieldTriangles() { return _fieldTriangles; }

    private:
        // Field macro informations
        double _fieldLength;
        double _fieldWidth;
        double _goalWidth;
        double _goalDepth;
        double _centerRadius;
        double _defenseRadius;
        double _defenseStretch;
        double _bigDefenseRadius;
        double _bigDefenseStretch;
        double _boundaryWidth;
        double _kFieldFBMarkX;
        double _kFieldFBMarkY;

        // Field lines / arcs / triangles
        QVector<FieldLine> _leftGoalLines;
        QVector<FieldLine> _rightGoalLines;
        QVector<FieldLine> _fieldLines;

        QVector<FieldCircularArc> _fieldArcs;
        QVector<FieldTriangle> _fieldTriangles;
};

#endif // FIELD_H
