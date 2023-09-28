#include "constants.h"

#include <src/world/entities/vision/filters/loss/lossfilter.h>
#include <src/world/entities/vision/filters/noise/noisefilter.h>

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
    readNetworkConstants();
    readRefereeConstants();
    readVisionConstants();
    readReplacerConstants();
    readTeamConstants();
    readWebhookConstants();
}

void Constants::readEntityConstants() {
    // Taking entity mapping in json
    QVariantMap threadMap = documentMap()["Entity"].toMap();

    // Filling vars
    _threadFrequency = threadMap["threadFrequency"].toInt();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded threadFrequency: " + std::to_string(_threadFrequency)) + '\n';
}

void Constants::readNetworkConstants() {
    // Taking entity mapping in json
    QVariantMap networkMap = documentMap()["Network"].toMap();

    // Filling vars
    _networkInterface = networkMap["networkInterface"].toString();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded network interface: " + _networkInterface.toStdString()) + '\n';
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
    _gameType = gameParamsMap["gameType"].toString();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded gameType: '" + _gameType.toStdString() + "'\n");

    _ballRadius = gameParamsMap["ballRadius"].toFloat();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded ballRadius: '" + std::to_string(_ballRadius) + "'\n");

    _robotLength = gameParamsMap["robotLength"].toFloat();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded robotLength: '" + std::to_string(_robotLength) + "'\n");

    _halfTime = gameParamsMap["halfTime"].toFloat();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded halfTime: '" + std::to_string(_halfTime) + "'\n");

    _overtimeHalfTime = gameParamsMap["overtimeHalfTime"].toFloat();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded overtimeHalfTime: '" + std::to_string(_overtimeHalfTime) + "'\n");

    // Fouls
    // Taking fouls mapping in json
    QVariantMap foulsMap = refereeMap["fouls"].toMap();

    // Filling vars
    _ballMinSpeedForStuck = foulsMap["ballMinSpeedForStuck"].toFloat();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded ballMinSpeedForStuck: '" + std::to_string(_ballMinSpeedForStuck) + "'\n");

    _stuckedBallTime = foulsMap["stuckedBallTime"].toFloat();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded stuckedBallTime: '" + std::to_string(_stuckedBallTime) + "'\n");

    _ballInAreaMaxTime = foulsMap["ballInAreaMaxTime"].toFloat();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded ballInAreaMaxTime: '" + std::to_string(_ballInAreaMaxTime) + "'\n");

    _useRefereeSuggestions = foulsMap["useRefereeSuggestions"].toBool();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded useRefereeSuggestions: '" + std::to_string(_useRefereeSuggestions) + "'\n");

    _maintainSpeedAtSuggestions = foulsMap["maintainSpeedAtSuggestions"].toBool();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded maintainSpeedAtSuggestions: '" + std::to_string(_maintainSpeedAtSuggestions) + "'\n");

}

void Constants::readVisionConstants() {
    // Taking vision mapping in json
    QVariantMap visionMap = documentMap()["Vision"].toMap();

    // Filling vars
    _visionAddress = visionMap["visionAddress"].toString();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded visionAddress: '" + _visionAddress.toStdString() + "'\n");

    _visionPort = visionMap["visionPort"].toUInt();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded visionPort: " + std::to_string(_visionPort)) + '\n';
    
    _firaVisionAddress = visionMap["firaVisionAddress"].toString();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded firaVisionAddress: '" + _firaVisionAddress.toStdString() + "'\n");

    _firaVisionPort = visionMap["firaVisionPort"].toUInt();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded firaVisionPort: " + std::to_string(_firaVisionPort)) + '\n';

    _isFIRAVision = visionMap["isFIRAVision"].toUInt();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded isFIRAVision: " + std::to_string(_isFIRAVision)) + '\n';

    // Filter constants
    QVariantMap filterMap = visionMap["filters"].toMap();

    _useKalman = filterMap["useKalman"].toBool();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded useKalman: " + std::to_string(_useKalman)) + '\n';

    _noiseTime = filterMap["noiseTime"].toInt();
    NoiseFilter::setNoiseTime(_noiseTime);
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded noiseTime: " + std::to_string(_noiseTime)) + '\n';

    _lossTime = filterMap["lossTime"].toInt();
    LossFilter::setLossTime(_lossTime);
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded lossTime: " + std::to_string(_lossTime)) + '\n';

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
    QVariantList teams = teamMap["teams"].toList();

    // Filling vars
    _qtPlayers = teamMap["qtPlayers"].toInt();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded qtPlayers: " + std::to_string(_qtPlayers)) + '\n';

    _teams.clear();

    foreach (const QVariant &t, teams) {
        if (t.canConvert<QString>()) {
            QString teamName = t.toString();
            _teams.append(teamName);
            std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded team: " + teamName.toStdString()) + '\n';
        }
    }

    _blueIsLeftSide = teamMap["blueIsLeftSide"].toInt();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded blueIsLeftSide: " + ((_blueIsLeftSide) ? QString("true").toStdString() : QString("false").toStdString()) + '\n');
}

void Constants::readWebhookConstants() {
    // Taking team mapping in json
    QVariantMap teamMap = documentMap()["Webhook"].toMap();

    // Filling vars
    _hID = teamMap["hID"].toString();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded hID: " + _hID.toStdString()) + '\n';

    _hToken = teamMap["hToken"].toString();
    std::cout << Text::purple("[CONSTANTS] ", true) << Text::bold("Loaded hToken: '" + _hToken.toStdString() + "'\n");
}

int Constants::threadFrequency() {
    return _threadFrequency;
}

QString Constants::networkInterface() {
    return _networkInterface;
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

QString Constants::gameType() {
    return _gameType;
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

float Constants::overtimeHalfTime() {
    return _overtimeHalfTime;
}

float Constants::ballMinSpeedForStuck() {
    return _ballMinSpeedForStuck;
}

float Constants::stuckedBallTime() {
    return _stuckedBallTime;
}

float Constants::ballInAreaMaxTime() {
    return _ballInAreaMaxTime;
}

bool Constants::useRefereeSuggestions() {
    return _useRefereeSuggestions;
}

bool Constants::maintainSpeedAtSuggestions() {
    return _maintainSpeedAtSuggestions;
}

void Constants::setIs5v5(bool is5v5) {
    _is5v5 = is5v5;
}

bool Constants::is5v5() {
    return _is5v5;
}

QString Constants::visionAddress() {
    return _visionAddress;
}

quint16 Constants::visionPort() {
    return _visionPort;
}

QString Constants::firaVisionAddress() {
    return _firaVisionAddress;
}

quint16 Constants::firaVisionPort() {
    return _firaVisionPort;
}

bool Constants::isFIRAVision() {
    return _isFIRAVision;
}

bool Constants::useKalman() {
    return _useKalman;
}

int Constants::noiseTime() {
    return _noiseTime;
}

int Constants::lossTime() {
    return _lossTime;
}

QStringList Constants::teams() {
    return _teams;
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

bool Constants::blueIsLeftSide() {
    return _blueIsLeftSide;
}

void Constants::swapSides() {
    _blueIsLeftSide = !_blueIsLeftSide;
}

QString Constants::getHID() {
    return _hID;
}

QString Constants::getHToken() {
    return _hToken;
}
