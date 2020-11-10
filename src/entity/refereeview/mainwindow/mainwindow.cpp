#include "mainwindow.h"
#include <build/tmp/moc/ui_mainwindow.h>

#include <QStyle>
#include <QStyleFactory>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Dark theme
    this->setStyle(QStyleFactory::create("Fusion"));
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window,QColor(53,53,53));
    darkPalette.setColor(QPalette::WindowText,Qt::white);
    darkPalette.setColor(QPalette::Disabled,QPalette::WindowText,QColor(127,127,127));
    darkPalette.setColor(QPalette::Base,QColor(42,42,42));
    darkPalette.setColor(QPalette::AlternateBase,QColor(66,66,66));
    darkPalette.setColor(QPalette::ToolTipBase,Qt::white);
    darkPalette.setColor(QPalette::ToolTipText,Qt::white);
    darkPalette.setColor(QPalette::Text,Qt::white);
    darkPalette.setColor(QPalette::Disabled,QPalette::Text,QColor(127,127,127));
    darkPalette.setColor(QPalette::Dark,QColor(35,35,35));
    darkPalette.setColor(QPalette::Shadow,QColor(20,20,20));
    darkPalette.setColor(QPalette::Button,QColor(53,53,53));
    darkPalette.setColor(QPalette::ButtonText,Qt::white);
    darkPalette.setColor(QPalette::Disabled,QPalette::ButtonText,QColor(127,127,127));
    darkPalette.setColor(QPalette::BrightText,Qt::red);
    darkPalette.setColor(QPalette::Link,QColor(42,130,218));
    darkPalette.setColor(QPalette::Highlight,QColor(42,130,218));
    darkPalette.setColor(QPalette::Disabled,QPalette::Highlight,QColor(80,80,80));
    darkPalette.setColor(QPalette::HighlightedText,Qt::white);
    darkPalette.setColor(QPalette::Disabled,QPalette::HighlightedText,QColor(127,127,127));

    this->setPalette(darkPalette);

    // Side control
    connect(ui->pushButton, SIGNAL(released()), this, SLOT(switchSides()));

    // Manual score control
    connect(ui->addGoalLeft, SIGNAL(released()), this, SLOT(addGoalLeft()));
    connect(ui->addGoalRight, SIGNAL(released()), this, SLOT(addGoalRight()));
    connect(ui->removeGoalLeft, SIGNAL(released()), this, SLOT(removeGoalLeft()));
    connect(ui->removeGoalRight, SIGNAL(released()), this, SLOT(removeGoalRight()));

    /// Manual Referee
    quadrantButtons.push_back(ui->freeBall_q1);
    quadrantButtons.push_back(ui->freeBall_q2);
    quadrantButtons.push_back(ui->freeBall_q3);
    quadrantButtons.push_back(ui->freeBall_q4);

    /// Connects for manual ref
    // Free ball
    connect(ui->freeBall,   SIGNAL(released()), this, SLOT(sendFreeBall()));
    connect(ui->freeBall_2, SIGNAL(released()), this, SLOT(sendFreeBall()));

    // Goal kick
    connect(ui->goalKickYellow, SIGNAL(released()), this, SLOT(sendYellowGoalKick()));
    connect(ui->goalKickBlue,   SIGNAL(released()), this, SLOT(sendBlueGoalKick()));

    // Penalty
    connect(ui->penaltyKickYellow, SIGNAL(released()), this, SLOT(sendYellowPenalty()));
    connect(ui->penaltyKickBlue,   SIGNAL(released()), this, SLOT(sendBluePenalty()));

    // Kickoff
    connect(ui->kickoffYellow, SIGNAL(released()), this, SLOT(sendYellowKickoff()));
    connect(ui->kickoffBlue,   SIGNAL(released()), this, SLOT(sendBlueKickoff()));

    // Start and stop
    connect(ui->startGame, SIGNAL(released()), this, SLOT(sendStart()));
    connect(ui->stopGame,  SIGNAL(released()), this, SLOT(sendStop()));

    // Setting initial scores values and colors
    leftTeamGoalsScored = 0;
    rightTeamGoalsScored = 0;
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::setTeams(QString teamLeft, VSSRef::Color leftColor, QString teamRight, VSSRef::Color rightColor){
    leftTeamName = teamLeft;
    rightTeamName = teamRight;
    ui->teamLeftName->setText(teamLeft);
    ui->teamRightName->setText(teamRight);

    blueIsLeft = (leftColor == VSSRef::Color::BLUE);

    if(leftColor == VSSRef::Color::BLUE)  ui->teamLeftScore->setStyleSheet("QLabel { color : #417EFF; }");
    else                                  ui->teamLeftScore->setStyleSheet("QLabel { color : #FFF33E; }");

    if(rightColor == VSSRef::Color::BLUE) ui->teamRightScore->setStyleSheet("QLabel { color : #417EFF; }");
    else                                  ui->teamRightScore->setStyleSheet("QLabel { color : #FFF33E; }");
}

void MainWindow::updateDetection(fira_message::Frame frame){
    ui->openGLWidget->updateDetection(frame);
}

void MainWindow::switchSides(){
    blueIsLeft = !blueIsLeft;

    // Swapping names
    std::swap(leftTeamName, rightTeamName);
    ui->teamLeftName->setText(leftTeamName);
    ui->teamRightName->setText(rightTeamName);

    // Swapping scores
    std::swap(leftTeamGoalsScored, rightTeamGoalsScored);
    ui->teamLeftScore->setText(QString("%1").arg(leftTeamGoalsScored));
    ui->teamRightScore->setText(QString("%1").arg(rightTeamGoalsScored));

    // Adjusting colors
    if(blueIsLeft){
        ui->teamLeftScore->setStyleSheet("QLabel { color : #417EFF; }");
        ui->teamRightScore->setStyleSheet("QLabel { color : #FFF33E; } ");
    }
    else{
        ui->teamLeftScore->setStyleSheet("QLabel { color : #FFF33E; }");
        ui->teamRightScore->setStyleSheet("QLabel { color : #417EFF; }");
    }

    ui->openGLWidget->setLeftIsBlue(blueIsLeft);
}

void MainWindow::addRefereeCommand(QString command){
    ui->referee_command->setText(command);
}

void MainWindow::setCurrentTime(int time){
    int min = time / 60;
    int sec = time - (min * 60);

    char timeleft[100];

    sprintf(timeleft, "(%.2d:%.2d)", min, sec);

    QString textToSet(timeleft);
    ui->referee_timeleft->setText(textToSet);
}

void MainWindow::setRefereeCommand(QString command){
    ui->referee_command->setText(command);
}

void MainWindow::drawText(vector2d pos, char *str){
    ui->openGLWidget->drawText(pos, str);
}

void MainWindow::addGoal(VSSRef::Color team){
    if(team == VSSRef::Color::BLUE){
        if(blueIsLeft){
            leftTeamGoalsScored++;
            ui->teamLeftScore->setText(QString("%1").arg(leftTeamGoalsScored));
        }
        else{
            rightTeamGoalsScored++;
            ui->teamRightScore->setText(QString("%1").arg(rightTeamGoalsScored));
        }
    }
    else if(team == VSSRef::Color::YELLOW){
        if(blueIsLeft){
            rightTeamGoalsScored++;
            ui->teamRightScore->setText(QString("%1").arg(rightTeamGoalsScored));
        }
        else{
            leftTeamGoalsScored++;
            ui->teamLeftScore->setText(QString("%1").arg(leftTeamGoalsScored));
        }
    }
}

void MainWindow::addGoalLeft(){
    leftTeamGoalsScored++;
    ui->teamLeftScore->setText(QString("%1").arg(leftTeamGoalsScored));
}

void MainWindow::addGoalRight(){
    rightTeamGoalsScored++;
    ui->teamRightScore->setText(QString("%1").arg(rightTeamGoalsScored));
}

void MainWindow::removeGoalLeft(){
    leftTeamGoalsScored--;
    if(leftTeamGoalsScored < 0) leftTeamGoalsScored = 0;
    ui->teamLeftScore->setText(QString("%1").arg(leftTeamGoalsScored));
}

void MainWindow::removeGoalRight(){
    rightTeamGoalsScored--;
    if(rightTeamGoalsScored < 0) rightTeamGoalsScored = 0;
    ui->teamRightScore->setText(QString("%1").arg(rightTeamGoalsScored));
}

VSSRef::Quadrant MainWindow::requestQuadrant(){
    for(int x = 0; x < quadrantButtons.size(); x++){
        if(quadrantButtons.at(x)->isChecked())
            return VSSRef::Quadrant(x+1);
    }

    return VSSRef::Quadrant::NO_QUADRANT;
}

void MainWindow::sendFreeBall(){
    emit sendManualCommand(VSSRef::Foul::FREE_BALL, VSSRef::Color::NONE, requestQuadrant());
}

void MainWindow::sendYellowKickoff(){
    emit sendManualCommand(VSSRef::Foul::KICKOFF, VSSRef::Color::YELLOW);
}

void MainWindow::sendBlueKickoff(){
    emit sendManualCommand(VSSRef::Foul::KICKOFF, VSSRef::Color::BLUE);
}

void MainWindow::sendYellowPenalty(){
    emit sendManualCommand(VSSRef::Foul::PENALTY_KICK, VSSRef::Color::YELLOW);
}

void MainWindow::sendBluePenalty(){
    emit sendManualCommand(VSSRef::Foul::PENALTY_KICK, VSSRef::Color::BLUE);
}

void MainWindow::sendYellowGoalKick(){
    emit sendManualCommand(VSSRef::Foul::GOAL_KICK, VSSRef::Color::YELLOW);
}

void MainWindow::sendBlueGoalKick(){
    emit sendManualCommand(VSSRef::Foul::GOAL_KICK, VSSRef::Color::BLUE);
}

void MainWindow::sendStart(){
    emit sendManualCommand(VSSRef::Foul::GAME_ON);
}

void MainWindow::sendStop(){
    emit sendManualCommand(VSSRef::Foul::STOP);
}
