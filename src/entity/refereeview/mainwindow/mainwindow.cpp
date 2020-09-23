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
    QListWidgetItem *item = new QListWidgetItem();
    QPixmap itemIcon;
    itemIcon.load(":/ui/referee.png");

    item->setText(command);
    item->setIcon(itemIcon);

    ui->listWidget->insertItem(0, item);
    ui->referee_command->setText(command);
}

void MainWindow::addRefereeWarning(QString command){
    QListWidgetItem *item = new QListWidgetItem();
    QPixmap itemIcon;
    itemIcon.load(":/ui/warning.png");

    item->setText(command);
    item->setIcon(itemIcon);


    ui->listWidget->insertItem(0, item);
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
