#include "constants.h"

Constants::Constants(QString fileName) {
    // Taking fileName
    _fileName = fileName;

    // Opening and reading file content in buffer
    _file.setFileName(_fileName);
    _file.open(QIODevice::ReadOnly | QIODevice::Text);
    _fileBuffer = _file.readAll();
    _file.close();

    // Parsing buffer to json objects
    _document = QJsonDocument::fromJson(_fileBuffer.toUtf8());
    _documentMap = _document.object().toVariantMap();

    // Read data
    readEntityConstants();
    readRefereeConstants();
    readVisionConstants();
    readReplacerConstants();
    readTeamConstants();
}

void Constants::readEntityConstants() {
    // Taking entity mapping in json
    QVariantMap threadMap = documentMap()["Entity"].toMap();

    // Filling vars
    _threadFrequency = threadMap["threadFrequency"].toInt();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded threadFrequency: " + std::to_string(_threadFrequency)) + '\n';
}

void Constants::readRefereeConstants() {
    // Taking referee mapping in json
    QVariantMap refereeMap = documentMap()["Referee"].toMap();

    // Filling vars
    _refereeAddress = refereeMap["refereeAddress"].toString();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded refereeAddress: '" + _refereeAddress.toStdString() + "'\n");

    _refereePort = refereeMap["refereePort"].toUInt();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded refereePort: " + std::to_string(_refereePort)) + '\n';

    _transitionTime = refereeMap["transitionTime"].toFloat();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded transitionTime: '" + std::to_string(_transitionTime) + "'\n");

    // Game params
    // Taking fouls mapping in json
    QVariantMap gameParamsMap = refereeMap["game"].toMap();

    // Filling vars
    _ballRadius = gameParamsMap["ballRadius"].toFloat();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded ballRadius: '" + std::to_string(_ballRadius) + "'\n");

    _robotLength = gameParamsMap["robotLength"].toFloat();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded robotLength: '" + std::to_string(_robotLength) + "'\n");

    _halfTime = gameParamsMap["halfTime"].toFloat();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded halfTime: '" + std::to_string(_halfTime) + "'\n");

    // Fouls
    // Taking fouls mapping in json
    QVariantMap foulsMap = refereeMap["fouls"].toMap();

    // Filling vars
    _ballMinSpeedForStuck = foulsMap["ballMinSpeedForStuck"].toFloat();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded ballMinSpeedForStuck: '" + std::to_string(_ballMinSpeedForStuck) + "'\n");

    _stuckedBallTime = foulsMap["stuckedBallTime"].toFloat();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded stuckedBallTime: '" + std::to_string(_stuckedBallTime) + "'\n");
}

void Constants::readVisionConstants() {
    // Taking vision mapping in json
    QVariantMap visionMap = documentMap()["Vision"].toMap();

    // Filling vars
    _visionAddress = visionMap["visionAddress"].toString();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded visionAddress: '" + _visionAddress.toStdString() + "'\n");

    _visionPort = visionMap["visionPort"].toUInt();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded visionPort: " + std::to_string(_visionPort)) + '\n';
}

void Constants::readReplacerConstants() {
    // Taking replacer mapping in json
    QVariantMap replacerMap = documentMap()["Replacer"].toMap();

    // Filling vars
    _replacerAddress = replacerMap["replacerAddress"].toString();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded replacerAddress: '" + _replacerAddress.toStdString() + "'\n");

    _replacerPort = replacerMap["replacerPort"].toUInt();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded replacerPort: " + std::to_string(_replacerPort)) + '\n';

    _firaAddress = replacerMap["firaAddress"].toString();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded firaAddress: '" + _firaAddress.toStdString() + "'\n");

    _firaPort = replacerMap["firaPort"].toUInt();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded firaPort: " + std::to_string(_firaPort)) + '\n';

}

void Constants::readTeamConstants() {
    // Taking team mapping in json
    QVariantMap teamMap = documentMap()["Team"].toMap();

    // Filling vars
    _qtPlayers = teamMap["qtPlayers"].toInt();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded qtPlayers: " + std::to_string(_qtPlayers)) + '\n';

    _blueTeamName = teamMap["blueTeamName"].toString();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded blueTeamName: '" + _blueTeamName.toStdString() + "'\n");

    _yellowTeamName = teamMap["yellowTeamName"].toString();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded yellowTeamName: '" + _yellowTeamName.toStdString() + "'\n");

    _blueIsLeftSide = teamMap["blueIsLeftSide"].toInt();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded blueIsLeftSide: " + ((_blueIsLeftSide) ? QString("true").toStdString() : QString("false").toStdString()) + '\n');
}

int Constants::threadFrequency() {
    return _threadFrequency;
}

QString Constants::refereeAddress() {
    return _refereeAddress;
}

quint16 Constants::refereePort() {
    return _refereePort;
}

float Constants::transitionTime() {
    return _transitionTime;
}

float Constants::ballRadius() {
    return _ballRadius;
}

float Constants::robotLength() {
    return _robotLength;
}

float Constants::halfTime() {
    return _halfTime;
}

float Constants::ballMinSpeedForStuck() {
    return _ballMinSpeedForStuck;
}

float Constants::stuckedBallTime() {
    return _stuckedBallTime;
}

QString Constants::visionAddress() {
    return _visionAddress;
}

quint16 Constants::visionPort() {
    return _visionPort;
}

QString Constants::replacerAddress() {
    return _replacerAddress;
}

quint16 Constants::replacerPort() {
    return _replacerPort;
}

QString Constants::firaAddress() {
    return _firaAddress;
}

quint16 Constants::firaPort() {
    return _firaPort;
}

int Constants::qtPlayers() {
    return _qtPlayers;
}

QString Constants::blueTeamName() {
    return _blueTeamName;
}

QString Constants::yellowTeamName() {
    return _yellowTeamName;
}

bool Constants::blueIsLeftSide() {
    return _blueIsLeftSide;
}

void Constants::swapSides() {
    std::swap(_blueTeamName, _yellowTeamName);
    _blueIsLeftSide = !_blueIsLeftSide;
}
