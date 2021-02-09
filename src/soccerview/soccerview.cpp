#include "soccerview.h"
#include "ui_soccerview.h"

#include <src/utils/text/text.h>
#include <QStyleFactory>
#include <QVariantAnimation>

SoccerView::SoccerView(Constants *constants, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SoccerView)
{
    // Set constants
    _constants = constants;

    // Setup UI
    ui->setupUi(this);
    setDarkTheme();
    setupTeams();
    setupGoals();
    setupButtons();
}

SoccerView::~SoccerView()
{
    delete ui;
}

FieldView* SoccerView::getFieldView() {
    return (ui->openGLWidget);
}

void SoccerView::setupTeams() {
    QString leftName, rightName;

    leftName = getConstants()->blueIsLeftSide() ? getConstants()->blueTeamName() : getConstants()->yellowTeamName();
    rightName = getConstants()->blueIsLeftSide() ? getConstants()->yellowTeamName() : getConstants()->blueTeamName();

    // Set names
    ui->leftTeamName->setText(leftName);
    ui->leftTeamName->setStyleSheet(getConstants()->blueIsLeftSide() ? "color: #779FFF;" : "color: #FCEE44;");
    ui->rightTeamName->setText(rightName);
    ui->rightTeamName->setStyleSheet(getConstants()->blueIsLeftSide() ? "color: #FCEE44;" : "color: #779FFF;");

    // Setup left logo
    QPixmap pixMap = QPixmap(":/teams/" + leftName.toLower() + ".png");
    if(!pixMap.isNull()) {
        ui->leftTeamLogo->setPixmap(pixMap);
    }
    else {
        std::cout << Text::blue("[SOCCERVIEW] ", true) + Text::red("Failed to setup left team logo, set default.", true) + '\n';
        ui->leftTeamLogo->setPixmap(QPixmap(":/teams/default.png"));
    }

    // Setup right logo
    pixMap = QPixmap(":/teams/" + rightName.toLower() + ".png");
    if(!pixMap.isNull()) {
        ui->rightTeamLogo->setPixmap(pixMap);
    }
    else {
        std::cout << Text::blue("[SOCCERVIEW] ", true) + Text::red("Failed to setup right team logo, set default.", true) + '\n';
        ui->rightTeamLogo->setPixmap(QPixmap(":/teams/default.png"));
    }

    // Set initial goals and call setupGoals
    _leftTeamGoals = 0;
    _rightTeamGoals = 0;
    setupGoals();
}

void SoccerView::setupGoals() {
    // Setup goals
    char leftGoal[5], rightGoal[5];
    sprintf(leftGoal, "%02d", _leftTeamGoals);
    sprintf(rightGoal, "%02d", _rightTeamGoals);

    ui->leftTeamGoals->setText(QString("%1").arg(leftGoal));
    ui->rightTeamGoals->setText(QString("%1").arg(rightGoal));
}

void SoccerView::setupButtons() {
    // Setup buttons mapper
    _buttonsMapper = new QSignalMapper();

    // Adding buttons to list
    _buttons.push_back(ui->goalKick);
    _buttons.push_back(ui->freeball);
    _buttons.push_back(ui->penaltyKick);
    _buttons.push_back(ui->kickoff);
    _buttons.push_back(ui->startGame);
    _buttons.push_back(ui->stopGame);
    _buttons.push_back(ui->haltGame);

    // Connecting game control buttons
    connect(ui->startGame, &QPushButton::released, [this](){
        ui->startGame->setEnabled(false);
        ui->stopGame->setEnabled(true);
        ui->haltGame->setEnabled(true);
    });

    connect(ui->stopGame, &QPushButton::released, [this](){
        ui->stopGame->setEnabled(false);
        ui->startGame->setEnabled(true);
        ui->haltGame->setEnabled(true);
    });

    connect(ui->haltGame, &QPushButton::released, [this](){
        ui->startGame->setEnabled(false);
        ui->stopGame->setEnabled(true);
        ui->haltGame->setEnabled(false);
    });

    // Connecting goal buttons to lambda
    connect(ui->addGoal, &QPushButton::released, [this](){
        // Take team to add
        VSSRef::Color calledColor = VSSRef::Color();
        VSSRef::Color_Parse(ui->forButtons->checkedButton()->whatsThis().toUpper().toStdString(), &calledColor);

        // Add
        addGoal(calledColor);
    });

    connect(ui->rmvGoal, &QPushButton::released, [this](){
        // Take team to add
        VSSRef::Color calledColor = VSSRef::Color();
        VSSRef::Color_Parse(ui->forButtons->checkedButton()->whatsThis().toUpper().toStdString(), &calledColor);

        // Add
        removeGoal(calledColor);
    });

    // Connecting time buttons to lambda
    connect(ui->addMinute, &QPushButton::released, [this](){
        emit addTime(60);
    });

    connect(ui->addSecond, &QPushButton::released, [this](){
        emit addTime(1);
    });

    // Connecting command buttons that need to place outside
    connect(ui->penaltyShootout, &QPushButton::released, [this](){
        VSSRef::Color calledColor = VSSRef::Color();
        VSSRef::Color_Parse(ui->forButtons->checkedButton()->whatsThis().toUpper().toStdString(), &calledColor);

        emit sendManualFoul(VSSRef::Foul::PENALTY_KICK, calledColor, VSSRef::Quadrant::NO_QUADRANT, true);
    });

    connect(ui->rmvField, &QPushButton::released, [this](){
        VSSRef::Color calledColor = VSSRef::Color();
        VSSRef::Color_Parse(ui->forButtons->checkedButton()->whatsThis().toUpper().toStdString(), &calledColor);

        emit sendManualFoul(VSSRef::Foul::KICKOFF, calledColor, VSSRef::Quadrant::NO_QUADRANT, true);
    });

    // Connect foul buttons to signal mapper
    for(int i = 0; i < _buttons.size(); i++) {
        QPushButton *button = _buttons.at(i);

        connect(button, SIGNAL(released()), _buttonsMapper, SLOT(map()), Qt::UniqueConnection);
        _buttonsMapper->setMapping(button, button);
        connect(_buttonsMapper, SIGNAL(mapped(QWidget *)), this, SLOT(processButton(QWidget *)), Qt::UniqueConnection);
    }
}

void SoccerView::setDarkTheme() {
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
}

void SoccerView::takeFoul(VSSRef::Foul foul, VSSRef::Color foulColor, VSSRef::Quadrant foulQuadrant) {
    // Take color to animate statusBoard and set its text
    QColor desiredColor;
    if(foul == VSSRef::Foul::GAME_ON) {
        desiredColor = QColor(53, 53, 53, 255);
        ui->statusColor->setText("Game is Running");

        // Enabling associated buttons
        ui->startGame->setEnabled(false);
        ui->stopGame->setEnabled(true);
        ui->haltGame->setEnabled(true);
    }
    else if(foul == VSSRef::Foul::STOP) {
        desiredColor = QColor(255, 112, 0, 255);
        ui->statusColor->setText("Game is Stopped");

        // Enabling associated buttons
        ui->startGame->setEnabled(true);
        ui->stopGame->setEnabled(false);
        ui->haltGame->setEnabled(true);
    }
    else if(foul == VSSRef::Foul::HALT) {
        desiredColor = QColor(238, 0, 34, 255);
        ui->statusColor->setText("Game is Halted");

        // Enabling associated buttons
        ui->startGame->setEnabled(false);
        ui->stopGame->setEnabled(true);
        ui->haltGame->setEnabled(false);
    }
    else {
        desiredColor = QColor(238, 0, 34, 255);
        if(foulColor != VSSRef::Color::NONE) {
            QString forBlue = QString("<font color=\"#0000CD\">%1</font>").arg(getConstants()->blueTeamName());
            QString forYellow = QString("<font color=\"#FCEE44\">%1</font>").arg(getConstants()->yellowTeamName());

            ui->statusColor->setText(QString("%1 for %2").arg(VSSRef::Foul_Name(foul).c_str()).arg(foulColor == VSSRef::Color::BLUE ? forBlue : forYellow));
        }
        else {
            ui->statusColor->setText(QString("%1 occurred at %2").arg(VSSRef::Foul_Name(foul).c_str()).arg(VSSRef::Quadrant_Name(foulQuadrant).c_str()));
        }
    }

    // Animate statusBoard
    QVariantAnimation *statusAnimation = new QVariantAnimation(this);
    statusAnimation->setStartValue(ui->statusColor->palette().background().color());
    statusAnimation->setEndValue(desiredColor);
    statusAnimation->setDuration(500);

    connect(statusAnimation, &QVariantAnimation::valueChanged, [this](const QVariant &value){
        QColor colorValue = QColor(value.value<QColor>());
        QString r = QString(std::to_string(colorValue.red()).c_str());
        QString g = QString(std::to_string(colorValue.green()).c_str());
        QString b = QString(std::to_string(colorValue.blue()).c_str());
        QString a = QString(std::to_string(colorValue.alpha()).c_str());
        QString rgba = QString("rgb(%1, %2, %3, %4)").arg(r).arg(g).arg(b).arg(a);
        ui->statusColor->setStyleSheet("border-radius: 10px; background-color: " + rgba + "; color: white;");
    });

    statusAnimation->start(QAbstractAnimation::DeleteWhenStopped);

    // Animate timer

    // Taking desired color
    if(foul == VSSRef::Foul::GAME_ON){
        desiredColor = QColor(48, 74, 48, 255);
    }
    else {
        desiredColor = QColor(238, 0, 34, 255);
    }

    QVariantAnimation *timerAnimation = new QVariantAnimation(this);
    timerAnimation->setStartValue(ui->gameTime->palette().background().color());
    timerAnimation->setEndValue(desiredColor);

    connect(timerAnimation, &QVariantAnimation::valueChanged, [this](const QVariant &value){
        QColor colorValue = QColor(value.value<QColor>());
        QString r = QString(std::to_string(colorValue.red()).c_str());
        QString g = QString(std::to_string(colorValue.green()).c_str());
        QString b = QString(std::to_string(colorValue.blue()).c_str());
        QString a = QString(std::to_string(colorValue.alpha()).c_str());
        QString rgba = QString("rgb(%1, %2, %3, %4)").arg(r).arg(g).arg(b).arg(a);
        ui->gameTime->setStyleSheet("border-radius:5px; color: white; background-color: " + rgba + ";");
    });

    timerAnimation->start(QAbstractAnimation::DeleteWhenStopped);

}

void SoccerView::takeTimeStamp(float timestamp, VSSRef::Half half) {
    int min = (getConstants()->halfTime() - timestamp) / 60.0;
    int sec = (getConstants()->halfTime() - timestamp) - (min * 60.0);

    // Seting gameTime
    char minStr[5], secStr[5];
    sprintf(minStr, "%02d", min);
    sprintf(secStr, "%02d", sec);
    ui->gameTime->setText(QString("%1:%2").arg(minStr).arg(secStr));

    // Setting gameHalf
    ui->halfTime->setText(QString(VSSRef::Half_Name(half).c_str()));
}

void SoccerView::addGoal(VSSRef::Color color) {
    if(color == VSSRef::Color::BLUE) {
        if(getConstants()->blueIsLeftSide()) {
            _leftTeamGoals++;
        }
        else{
            _rightTeamGoals++;
        }
    }
    else if(color == VSSRef::Color::YELLOW) {
        if(!getConstants()->blueIsLeftSide()) {
            _leftTeamGoals++;
        }
        else{
            _rightTeamGoals++;
        }
    }

    setupGoals();
}

void SoccerView::removeGoal(VSSRef::Color color) {
    if(color == VSSRef::Color::BLUE) {
        if(getConstants()->blueIsLeftSide()) {
            _leftTeamGoals = std::max(0, _leftTeamGoals - 1);
        }
        else{
            _rightTeamGoals = std::max(0, _rightTeamGoals - 1);
        }
    }
    else if(color == VSSRef::Color::YELLOW) {
        if(!getConstants()->blueIsLeftSide()) {
            _leftTeamGoals = std::max(0, _leftTeamGoals - 1);
        }
        else{
            _rightTeamGoals = std::max(0, _rightTeamGoals - 1);
        }
    }

    setupGoals();
}

void SoccerView::processButton(QWidget *button) {
    QPushButton *castedButton = static_cast<QPushButton*>(button);

    // Parsing foul
    VSSRef::Foul calledFoul = VSSRef::Foul();
    VSSRef::Foul_Parse(castedButton->whatsThis().toUpper().toStdString(), &calledFoul);

    // Parsing color
    VSSRef::Color calledColor = VSSRef::Color();
    VSSRef::Color_Parse(ui->forButtons->checkedButton()->whatsThis().toUpper().toStdString(), &calledColor);

    // Parsing quadrant
    VSSRef::Quadrant calledQuadrant = VSSRef::Quadrant();
    VSSRef::Quadrant_Parse(ui->quadrantButtons->checkedButton()->whatsThis().toUpper().toStdString(), &calledQuadrant);

    // Emit manual foul
    emit sendManualFoul(calledFoul, (calledFoul == VSSRef::Foul::FREE_BALL) ? VSSRef::Color::NONE : calledColor, (calledFoul != VSSRef::Foul::FREE_BALL) ? VSSRef::Quadrant::NO_QUADRANT : calledQuadrant);
}

void SoccerView::addSuggestion(QString suggestion, VSSRef::Color forColor) {
    _suggestionsMutex.lock();

    int qtWidgets = _widgets.size() / 2;

    QString forSuggestionName = QString("%1 for <font color=\"%2\">%3</font>").arg(suggestion).arg((forColor == VSSRef::Color::BLUE) ? "#1E90FF" : "#FCEE44").arg(VSSRef::Color_Name(forColor).c_str());
    QLabel *label = new QLabel((forColor != VSSRef::Color::NONE) ? forSuggestionName : suggestion);
    label->setWhatsThis(suggestion);

    _widgets.push_back(label);
    ui->suggestionGrid->addWidget(label, qtWidgets, 0, Qt::AlignCenter);

    QPushButton *accept = new QPushButton("Accept");
    connect(accept, &QPushButton::released, [this, forColor, label](){
        if(label->whatsThis() == "GOAL") {
            addGoal(forColor);

            emit sendManualFoul(VSSRef::Foul::KICKOFF, (forColor == VSSRef::Color::BLUE) ? VSSRef::Color::YELLOW : VSSRef::Color::BLUE, VSSRef::Quadrant::NO_QUADRANT);
        }
        else {
            VSSRef::Foul suggestedFoul = VSSRef::Foul();
            VSSRef::Foul_Parse(label->whatsThis().toStdString(), &suggestedFoul);

            emit sendManualFoul(suggestedFoul, forColor, VSSRef::Quadrant::NO_QUADRANT);
        }

        deleteSuggestions();
    });

    _widgets.push_back(accept);
    ui->suggestionGrid->addWidget(accept, qtWidgets, 1, Qt::AlignCenter);

    _suggestionsMutex.unlock();
}

void SoccerView::deleteSuggestions() {
    _suggestionsMutex.lock();

    for(int i = 0; i < _widgets.size(); i++) {
        QWidget *widget = _widgets.at(i);
        ui->suggestionGrid->removeWidget(widget);

        delete widget;
    }

    _widgets.clear();

    _suggestionsMutex.unlock();
}

Constants* SoccerView::getConstants() {
    if(_constants == nullptr) {
        std::cout << Text::red("[ERROR] ", true) << Text::bold("Constants with nullptr value at SoccerView") + '\n';
    }
    else {
        return _constants;
    }

    return nullptr;
}
