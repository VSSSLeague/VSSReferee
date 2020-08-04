#include "vssreferee.h"

QString VSSReferee::name(){
    return "VSSReferee";
}

VSSReferee::VSSReferee(VSSVisionClient *visionClient, const QString& refereeAddress, int refereePort)
{
    _visionClient   = visionClient;
    _refereeAddress = refereeAddress;
    _refereePort    = refereePort;

    connect(refereeAddress, refereePort);

    // Reset vars initially
    _placementIsSet = false;
    _blueSent       = false;
    _yellowSent     = false;
    _placementTimer.start();
}

VSSReferee::~VSSReferee(){

}

void VSSReferee::initialization(){

}

void VSSReferee::loop(){
    // Checking timer overextended if a foul is set
    if(_placementIsSet){
        _placementTimer.stop();
        if((_placementTimer.timensec() / 1E9) >= PLACEMENT_WAIT_TIME && (!_blueSent || !_yellowSent)){
            // If enters here, one of the teams didn't placed as required in the determined time
            if(!_blueSent) std::cout << "[VSSReferee] Team BLUE hasn't sent the placement command." << std::endl;
            if(!_yellowSent) std::cout << "[VSSReferee] Team YELLOW hasn't sent the placement command." << std::endl;
            _placementMutex.lock();
            _blueSent = false;
            _yellowSent = false;
            _placementIsSet = false;
            _placementMutex.unlock();

            // Do something here (an foul when one of the teams haven't placed ?)

        }
        else if(_blueSent && _yellowSent){
            _placementMutex.lock();
            _blueSent = false;
            _yellowSent = false;
            _placementIsSet = false;
            _placementMutex.unlock();
        }
    }
    else{
        /// TODO HERE
        /// Receive and process VSSVisionClient informations to check fouls

        // By default, send a FREE_BALL for team BLUE (foul packet test)
        _refereeCommand.set_foul(VSSRef::Foul::FREE_BALL);
        _refereeCommand.set_teamcolor(VSSRef::Color::BLUE);

        if(isConnected()){
            sendPacket(_refereeCommand);
            setFoul(_refereeCommand.foul());
        }
    }
}

void VSSReferee::finalization(){
    disconnect();
    std::cout << "[VSSReferee] Thread ended" << std::endl;
}

void VSSReferee::sendPacket(VSSRef::ref_to_team::VSSRef_Command command){
    std::string msg;
    command.SerializeToString(&msg);

    if(_socket.write(msg.c_str(), msg.length()) == -1){
        std::cout << "[VSSReferee] Failed to write to socket: " << _socket.errorString().toStdString() << std::endl;
    }
    else{
        _placementMutex.lock();
        _placementIsSet = true;
        _blueSent = false;
        _yellowSent = false;
        _placementTimer.start();
        _placementMutex.unlock();
    }
}

bool VSSReferee::connect(const QString &refereeAddress, int refereePort){
    // Connect to referee address and port
    if(_socket.isOpen())
        _socket.close();

    _socket.connectToHost(refereeAddress, refereePort, QIODevice::WriteOnly, QAbstractSocket::IPv4Protocol);

    std::cout << "[VSSReferee] Writing to referee system on port " << _refereePort << " and address = " << _refereeAddress.toStdString() << ".\n";

    return true;
}

void VSSReferee::disconnect() {
    // Close referee socket
    if(_socket.isOpen()){
        _socket.close();
    }
}

bool VSSReferee::isConnected() const {
    return (_socket.isOpen());
}

void VSSReferee::teamSent(VSSRef::Color color){
    /// TODO HERE
    if(color == VSSRef::Color::BLUE){
        _placementMutex.lock();
        _blueSent = true;
        _placementMutex.unlock();
    }
    else if(color == VSSRef::Color::YELLOW){
        _placementMutex.lock();
        _yellowSent = true;
        _placementMutex.unlock();
    }
}
