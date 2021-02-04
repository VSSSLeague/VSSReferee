#ifndef CHECKER_H
#define CHECKER_H

#include <QObject>

#include <src/world/entities/vision/vision.h>
#include <src/utils/utils.h>

// Abstract referee
class Referee;

class Checker : public QObject
{
    Q_OBJECT
public:
    Checker(Vision *vision/*, Referee *referee*/, Constants *constants);

    // Internal
    virtual QString name() = 0;
    virtual void configure() = 0;
    virtual void run() = 0;

    // Foul penalties info
    VSSRef::Foul penalty();
    VSSRef::Color teamColor();
    VSSRef::Quadrant quadrant();

protected:
    Vision* getVision();
    //Referee* getReferee();
    Constants* getConstants();

    // Getters
    //bool isGameOn();

    // Foul penalties setter
    void setPenaltiesInfo(VSSRef::Foul penalty, VSSRef::Color teamColor = VSSRef::Color::NONE, VSSRef::Quadrant quadrant = VSSRef::Quadrant::NO_QUADRANT);

private:
    // Vision module
    Vision *_vision;

    // Referee module
    //Referee *_referee;

    // Constants
    Constants *_constants;

    // Penalties info
    VSSRef::Foul _penalty;
    VSSRef::Color _teamColor;
    VSSRef::Quadrant _quadrant;

signals:
    void foulOccured();
};

#endif // CHECKER_H
