#include "recorder.h"

Recorder::Recorder(QString fileName, QString visionAddress, quint16 visionPort, QString refereeAddress, quint16 refereePort) {
    // Set objects as nullptr
    _file = nullptr;
    _visionSocket = nullptr;
    _refereeSocket = nullptr;

    // Take vars
    _fileName = fileName;
    _visionAddress = visionAddress;
    _visionPort = visionPort;
    _refereeAddress = refereeAddress;
    _refereePort = refereePort;

    // Open file
    openFile();

    // Connect to networks
    connectToVisionNetwork();
    connectToRefereeNetwork();
}

Recorder::~Recorder() {
    if(_file == nullptr) {
        // Closing file
        if(_file->isOpen()) {
            _file->close();
        }

        // Delete it
        delete _file;
    }

    if(_visionSocket != nullptr) {
        // Closing socket
        if(_visionSocket->isOpen()) {
            _visionSocket->close();
        }

        // Delete it
        delete _visionSocket;

        std::cout << Text::cyan("[VISION] " , true) + Text::bold("Client finished.") + '\n';
    }

    if(_refereeSocket != nullptr) {
        // Closing socket
        if(_refereeSocket->isOpen()) {
            _refereeSocket->close();
        }

        // Delete it
        delete _refereeSocket;

        std::cout << Text::cyan("[REFEREE] " , true) + Text::bold("Client finished.") + '\n';
    }
}

void Recorder::openFile() {
    // Create pointer
    _file = new QFile(_fileName);

    // Create path for logs
    QDir directory;
    directory.mkdir(PROJECT_PATH + QString("/logs/"));

    // Open file
    if(!_file->open(QIODevice::WriteOnly)) {
        std::cout << Text::cyan("[RECORDER] ", true) + Text::red("Error opening file '" + _fileName.toStdString() + "'", true) + '\n';
        return ;
    }

    // Debug
    std::cout << Text::cyan("[RECORDER] ", true) + Text::bold("Opened and logging in file '" + _fileName.toStdString() + "'") + '\n';
}

void Recorder::connectToVisionNetwork() {
    // Creating socket
    _visionSocket = new QUdpSocket();

    // Binding in defined network
    if(_visionSocket->bind(QHostAddress(_visionAddress), _visionPort, QUdpSocket::ShareAddress) == false) {
        std::cout << Text::purple("[VISION] ", true) + Text::red("Error while binding socket.", true) + '\n';
        return ;
    }

    // Joining multicast group
    if(_visionSocket->joinMulticastGroup(QHostAddress(_visionAddress)) == false) {
        std::cout << Text::purple("[VISION] ", true) + Text::red("Error while joining multicast group.") + '\n';
        return ;
    }

    // Connect readyRead signal
    QObject::connect(_visionSocket, &QUdpSocket::readyRead, [this](){
        // Creating datagram var
        QNetworkDatagram datagram;

        // Reading datagram and checking if it is valid
        datagram = _visionSocket->receiveDatagram();
        if(!datagram.isValid()) {
            return ;
        }

        // Prepare vars
        QByteArray data = datagram.data();
        qint64 timeStamp = Timer::systemTime();

        writeDatagram(MESSAGE_VISION, data, timeStamp);
    });

    std::cout << Text::cyan("[VISION] ", true) + Text::bold("Started at address '" + _visionAddress.toStdString() + "' and port '" + std::to_string(_visionPort) + "'.") + '\n';
}

void Recorder::connectToRefereeNetwork() {
    // Creating socket
    _refereeSocket = new QUdpSocket();

    // Binding in defined network
    if(_refereeSocket->bind(QHostAddress(_refereeAddress), _refereePort, QUdpSocket::ShareAddress) == false) {
        std::cout << Text::purple("[REFEREE] ", true) + Text::red("Error while binding socket.", true) + '\n';
        return ;
    }

    // Joining multicast group
    if(_refereeSocket->joinMulticastGroup(QHostAddress(_refereeAddress)) == false) {
        std::cout << Text::purple("[REFEREE] ", true) + Text::red("Error while joining multicast group.") + '\n';
        return ;
    }

    // Connect readyRead signal
    QObject::connect(_refereeSocket, &QUdpSocket::readyRead, [this](){
        // Creating datagram var
        QNetworkDatagram datagram;

        // Reading datagram and checking if it is valid
        datagram = _refereeSocket->receiveDatagram();
        if(!datagram.isValid()) {
            return ;
        }

        // Prepare vars
        QByteArray data = datagram.data();
        qint64 timeStamp = Timer::systemTime();

        writeDatagram(MESSAGE_REFEREE, data, timeStamp);
    });

    std::cout << Text::cyan("[REFEREE] ", true) + Text::bold("Started at address '" + _refereeAddress.toStdString() + "' and port '" + std::to_string(_refereePort) + "'.") + '\n';
}

void Recorder::writeDatagram(MessageType messageType, QByteArray data, qint64 timeStamp) {
    _fileMutex.lock();

    QDataStream stream(_file);
    stream << timeStamp;
    stream << (quint32) messageType;
    stream << data;

    _fileMutex.unlock();
}
