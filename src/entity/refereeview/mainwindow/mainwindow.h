#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <src/entity/refereeview/soccerview/soccerview.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent);
    ~MainWindow();

    // Detection updates
    void updateDetection(fira_message::Frame frame);

    // Referee
    void addRefereeCommand(QString command);
    void addRefereeWarning(QString command);
    void setCurrentTime(int time);
    void setRefereeCommand(QString command);
    bool getBlueIsLeftSide() { return blueIsLeft; }

private:
    Ui::MainWindow *ui;

    // sides control
    bool blueIsLeft;
    QString leftTeamName;
    QString rightTeamName;

public slots:
    void switchSides();
};

#endif // MAINWINDOW_H
