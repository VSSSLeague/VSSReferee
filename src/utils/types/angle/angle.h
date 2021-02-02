#ifndef ANGLE_H
#define ANGLE_H

class Angle
{
public:
    // Pi constants
    static constexpr float pi    = 3.14159265358979323846;
    static constexpr float twoPi = 6.28318530717958647692;

private:
    // Conversion constants
    static constexpr float toRad =  0.01745329251994329577;
    static constexpr float toDeg = 57.29577951308232087680;

// Params
private:
    // Angle value
    float _value;

    // Info flags
    bool _unknown;
    bool _valid;

// Conversions
public:
    static float toRadians(float angle);
    static float toDegrees(const Angle& angle);
    static float toDegrees(float angle);
    static float difference(const Angle& a, const Angle& b);

// Constructors
public:
    Angle();
    Angle(bool known, float value);

// Setters
public:
    void setValue(float value);
    float value() const;

public:
    bool isValid() const;
    void setInvalid();
};

#endif // ANGLE_H
