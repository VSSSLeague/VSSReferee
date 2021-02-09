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

    // Suggestions
    QList<QWidget*> _widgets;
    QMutex _suggestionsMutex;
    void deleteSuggestions();

signals:
    void sendManualFoul(VSSRef::Foul foul, VSSRef::Color foulColor, VSSRef::Quadrant foulQuadrant, bool isToPlaceOutside = false);
    void addTime(int seconds);

public slots:
    void takeFoul(VSSRef::Foul foul, VSSRef::Color foulColor, VSSRef::Quadrant foulQuadrant);
    void takeTimeStamp(float timestamp, VSSRef::Half half);
    void addGoal(VSSRef::Color color);
    void removeGoal(VSSRef::Color color);
    void processButton(QWidget *button);
    void addSuggestion(QString suggestion, VSSRef::Color forColor = VSSRef::Color::NONE);
};

#endif // SOCCERVIEW_H
