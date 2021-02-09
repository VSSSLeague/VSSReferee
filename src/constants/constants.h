#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>
#include <QString>
#include <QFile>

#include <src/utils/text/text.h>

class Constants
{
public:
    Constants(QString fileName);

    // Entities constants getters
    int threadFrequency();

    // Referee constants getters
    QString refereeAddress();
    quint16 refereePort();
    float transitionTime();
    QString gameType();
    float ballRadius();
    float robotLength();
    float halfTime();
    float overtimeHalfTime();
    float ballMinSpeedForStuck();
    float stuckedBallTime();
    float ballInAreaMaxTime();
    bool useRefereeSuggestions();
    bool maintainSpeedAtSuggestions();

    // Vision constants getters
    QString visionAddress();
    quint16 visionPort();
    bool useKalman();
    int noiseTime();
    int lossTime();

    // Replacer constants getters
    QString replacerAddress();
    quint16 replacerPort();
    QString firaAddress();
    quint16 firaPort();

    // Teams constants getters
    int qtPlayers();
    QString blueTeamName();
    QString yellowTeamName();
    bool blueIsLeftSide();
    void swapSides();

protected:
    QVariantMap documentMap() { return _documentMap; }

private:
    // Internal file management
    QString _fileName;
    QString _fileBuffer;
    QFile _file;

    // Internal json parse vars
    QJsonDocument _document;
    QVariantMap _documentMap;

    // Entities constants
    int _threadFrequency;
    void readEntityConstants();

    // Referee
    QString _refereeAddress;
    quint16 _refereePort;
    float _transitionTime;
    QString _gameType;
    float _ballRadius;
    float _robotLength;
    float _halfTime;
    float _overtimeHalfTime;
    float _ballMinSpeedForStuck;
    float _stuckedBallTime;
    float _ballInAreaMaxTime;
    bool _useRefereeSuggestions;
    bool _maintainSpeedAtSuggestions;
    void readRefereeConstants();

    // Vision constants
    QString _visionAddress;
    quint16 _visionPort;
    bool _useKalman;
    int _noiseTime;
    int _lossTime;
    void readVisionConstants();

    // Replacer constants
    QString _replacerAddress;
    quint16 _replacerPort;
    QString _firaAddress;
    quint16 _firaPort;
    void readReplacerConstants();

    // Teams constants
    int _qtPlayers;
    QString _blueTeamName;
    QString _yellowTeamName;
    bool _blueIsLeftSide;
    void readTeamConstants();
};

#endif // CONSTANTS_H
