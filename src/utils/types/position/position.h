#ifndef POSITION_H
#define POSITION_H

class Position
{
public:
    Position();
    Position(bool isValid, float x, float y);

    // Setters
    void setPosition(bool isValid, float x, float y);
    void setInvalid();

    // Getterrs
    bool isInvalid() const;
    float x() const;
    float y() const;

private:
    // Params
    bool _isValid;
    float _x;
    float _y;
};

#endif // POSITION_H
