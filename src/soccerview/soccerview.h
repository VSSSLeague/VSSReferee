#ifndef SOCCERVIEW_H
#define SOCCERVIEW_H

#include <QMainWindow>
#include <QPushButton>

#include <src/constants/constants.h>
#include <src/world/entities/referee/referee.h>
#include <include/vssref_common.pb.h>
#include <src/soccerview/fieldview/fieldview.h>

namespace Ui {
class SoccerView;
}

class SoccerView : public QMainWindow
{
    Q_OBJECT
public:
    explicit SoccerView(Constants *constants, QWidget *parent = nullptr);
    ~SoccerView();

    FieldView *getFieldView();
    QString getStage();
    int getLeftTeamGoals();
    int getRightTeamGoals();

private:
    Ui::SoccerView *ui;

    // Dark theme
    void setDarkTheme();

    // Teams
    void setupTeams();
    void setupGoals();
    void setupButtons();
    void setTeams();
    // Animations
    void animateWidget(QWidget *widget, QColor desiredColor, int animationTime);
    void animateFlag(bool toShow, int animationTime);

    // Mapping buttons
    QList<QPushButton*> _buttons;
    QSignalMapper *_buttonsMapper;
    QStringList _teams;

    // Constants
    Constants *_constants;
    Constants* getConstants();

    // Goals
    int _leftTeamGoals = 0;
    int _rightTeamGoals = 0;

    // Suggestions
    QList<QWidget*> _widgets;
    QMutex _suggestionsMutex;
    void showSuggestions();
    void deleteSuggestions();

signals:
    void sendCollisionDecision();
    void sendManualFoul(VSSRef::Foul foul, VSSRef::Color foulColor, VSSRef::Quadrant foulQuadrant, bool isToPlaceOutside = false);
    void addTime(int seconds);
    void sendTeleport(bool teleport);
    void changeVision(bool isFIRAvision);

public slots:
    void takeFoul(VSSRef::Foul foul, VSSRef::Color foulColor, VSSRef::Quadrant foulQuadrant);
    void takeTimeStamp(float halftime, float timestamp, VSSRef::Half half, bool isEndGame);
    void addGoal(VSSRef::Color color);
    void removeGoal(VSSRef::Color color);
    void processButton(QWidget *button);
    void addSuggestion(QString suggestion, VSSRef::Color forColor = VSSRef::Color::NONE, VSSRef::Quadrant atQuadrant = VSSRef::Quadrant::NO_QUADRANT);
};

#endif // SOCCERVIEW_H
