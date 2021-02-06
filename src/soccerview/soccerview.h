#ifndef SOCCERVIEW_H
#define SOCCERVIEW_H

#include <QMainWindow>
#include <QPushButton>

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
    void setupGoals();
    void setupButtons();

    // Mapping buttons
    QList<QPushButton*> _buttons;
    QSignalMapper *_buttonsMapper;

    // Constants
    Constants *_constants;
    Constants* getConstants();

    // Goals
    int _leftTeamGoals;
    int _rightTeamGoals;

signals:
    void sendManualFoul(VSSRef::Foul foul, VSSRef::Color foulColor, VSSRef::Quadrant foulQuadrant);

public slots:
    void takeFoul(VSSRef::Foul foul, VSSRef::Color foulColor, VSSRef::Quadrant foulQuadrant);
    void takeTimeStamp(float timestamp, VSSRef::Half half);
    void addGoal(VSSRef::Color color);
    void processButton(QWidget *button);
};

#endif // SOCCERVIEW_H
