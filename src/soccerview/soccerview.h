#ifndef SOCCERVIEW_H
#define SOCCERVIEW_H

#include <QMainWindow>
#include <src/constants/constants.h>
#include <src/world/entities/referee/referee.h>
#include <include/vssref_common.pb.h>

namespace Ui {
class SoccerView;
}

class SoccerView : public QMainWindow
{
    Q_OBJECT

public:
    explicit SoccerView(Constants *constants, QWidget *parent = nullptr);
    ~SoccerView();

private:
    Ui::SoccerView *ui;

    // Dark theme
    void setDarkTheme();

    // Teams
    void setupTeams();

    // Constants
    Constants *_constants;
    Constants* getConstants();

public slots:
    void takeFoul(VSSRef::Foul foul, VSSRef::Color foulColor, VSSRef::Quadrant foulQuadrant);
    void takeTimeStamp(float timestamp, VSSRef::Half half);
};

#endif // SOCCERVIEW_H
