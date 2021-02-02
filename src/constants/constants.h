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

    // Vision constants getters
    QString visionAddress();
    quint16 visionPort();

    // Replacer constants getters

    // Teams constants getters
    int qtPlayers();

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
    void readRefereeConstants();

    // Vision constants
    QString _visionAddress;
    quint16 _visionPort;
    void readVisionConstants();

    // Replacer constants
    void readReplacerConstants();

    // Teams constants
    int _qtPlayers;
    void readTeamConstants();
};

#endif // CONSTANTS_H
