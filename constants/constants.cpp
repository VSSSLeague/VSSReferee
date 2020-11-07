#include "constants.h"

Constants::Constants(QString fileName){
    // Opening file and setting initial config
    QFile file(fileName);
    QString args;
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    // Reading and closing file
    args = file.readAll();
    file.close();

    // Now we'll parse file data
    QJsonDocument document = QJsonDocument::fromJson(args.toUtf8());

    // Creating variant map to map json parameters
    QJsonObject docObject = document.object();
    QVariantMap doc_map = docObject.toVariantMap();

    // Parsing field and robot data
    _qtPlayers       = doc_map["QtPlayers"].toInt();
    _ballRadius      = doc_map["BallRadius"].toFloat();
    _robotLength     = doc_map["RobotLength"].toFloat();

    // Parsing vision network
    QVariantMap visionMap = doc_map["VisionNetwork"].toMap();
    _visionAddress   = visionMap["address"].toString();
    _visionPort      = visionMap["port"].toInt();

    // Parsing referee network
    QVariantMap refereeMap = doc_map["RefereeNetwork"].toMap();
    _refereeAddress  = refereeMap["address"].toString();
    _refereePort     = refereeMap["port"].toInt();

    // Parsing replacer network
    QVariantMap replacerMap = doc_map["ReplacerNetwork"].toMap();
    _replacerAddress = replacerMap["address"].toString();
    _replacerPort    = replacerMap["port"].toInt();

    // Parsing firaSim network
    QVariantMap firaMap = doc_map["FiraSimNetwork"].toMap();
    _firaSimAddress  = firaMap["address"].toString();
    _firaSimPort     = firaMap["port"].toInt();

    // Parsing team left data
    QVariantMap teamLeftMap = doc_map["TeamLeft"].toMap();
    _teamLeftName    = teamLeftMap["name"].toString();
    _teamLeftColor   = (teamLeftMap["color"].toString() == "blue") ? VSSRef::Color::BLUE : VSSRef::Color::YELLOW;

    // Parsing team right data
    QVariantMap teamRightMap = doc_map["TeamRight"].toMap();
    _teamRightName   = teamRightMap["name"].toString();
    _teamRightColor  = (teamRightMap["color"].toString() == "blue") ? VSSRef::Color::BLUE : VSSRef::Color::YELLOW;

    // Parsing parameters
    QVariantMap parametersMap = doc_map["Parameters"].toMap();
    _ballMinimumVelocity = parametersMap["BallMinimumVelocity"].toFloat();
    _placementWaitTime   = parametersMap["PlacementWaitTime"].toFloat();
    _stopWaitTime        = parametersMap["StopWaitTime"].toFloat();
    _gkTakeoutTime       = parametersMap["GKTakeoutTime"].toFloat();
    _ballStuckTime       = parametersMap["BallStuckTime"].toFloat();
    _penaltyTime         = parametersMap["PenaltyTime"].toFloat();
    _gameHalfTime        = parametersMap["GameHalfTime"].toFloat();
}

int Constants::getQtPlayers() const{
    return _qtPlayers;
}

float Constants::getBallRadius() const{
    return _ballRadius;
}

float Constants::getRobotLength() const{
    return _robotLength;
}

QString Constants::getVisionAddress() const{
    return _visionAddress;
}

int Constants::getVisionPort() const{
    return _visionPort;
}

QString Constants::getRefereeAddress() const{
    return _refereeAddress;
}

int Constants::getRefereePort() const{
    return _refereePort;
}

QString Constants::getReplacerAddress() const{
    return _replacerAddress;
}

int Constants::getReplacerPort() const{
    return _replacerPort;
}

QString Constants::getFiraSimAddress() const{
    return _firaSimAddress;
}

int Constants::getFiraSimPort(){
    return _firaSimPort;
}

float Constants::getBallMinimumVelocity() const{
    return _ballMinimumVelocity;
}

float Constants::getPlacementWaitTime() const{
    return _placementWaitTime;
}

float Constants::getGKTakeoutTime() const{
    return _gkTakeoutTime;
}

float Constants::getBallStuckTime() const{
    return _ballStuckTime;
}

float Constants::getPenaltyTime() const{
    return _penaltyTime;
}

float Constants::getGameHalfTime() const{
    return _gameHalfTime;
}

float Constants::getStopWaitTime() const{
    return _stopWaitTime;
}

QString Constants::getLeftTeamName() const{
    return _teamLeftName;
}

VSSRef::Color Constants::getLeftTeamColor() const{
    return _teamLeftColor;
}

QString Constants::getRightTeamName() const{
    return _teamRightName;
}

VSSRef::Color Constants::getRightTeamColor() const{
    return _teamRightColor;
}
