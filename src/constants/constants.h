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

    // Vision constants getters

    // Referee constants getters

    // Replacer constants getters

    // Teams constants getters

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
    void readVisionConstants();

    // Replacer constants
    void readReplacerConstants();

    // Teams constants
    void readTeamConstants();
};

#endif // CONSTANTS_H
