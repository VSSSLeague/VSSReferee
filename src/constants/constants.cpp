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

}

void Constants::readVisionConstants() {

}

void Constants::readReplacerConstants() {

}

void Constants::readTeamConstants() {

}

int Constants::threadFrequency() {
    return _threadFrequency;
}
