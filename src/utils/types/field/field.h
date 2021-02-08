#ifndef FIELD_H
#define FIELD_H

#include <QString>

class FieldLine {
public:
  QString name;
  double p1_x;
  double p1_y;
  double p2_x;
  double p2_y;
  double thickness;

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

  FieldTriangle(const FieldTriangle& other);
  FieldTriangle(const std::string& marking_name, double p1_x_, double p1_y_, double p2_x_, double p2_y_, double p3_x_, double p3_y_, double thickness_);
};

#endif // FIELD_H
