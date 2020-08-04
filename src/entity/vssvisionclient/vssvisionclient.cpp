#include "vssvisionclient.h"

QString VSSVisionClient::name(){
    return "VSS_Vision Client";
}

VSSVisionClient::VSSVisionClient(const QString &visionAddress, int visionPort)
{
    _visionAddress = visionAddress;
    _visionPort    = visionPort;

    // Create a VSS Client to listen to vision packets
    _vssClient = new VSSClient(_visionPort, visionAddress.toStdString());

    // Disable loopTime
    this->setLoopTime(0);

    // Update flags to default
    _hasDetectionUpdate = false;
    _hasGeometryUpdate  = false;

    // Clear packets
    _geometryPacket.Clear();
}

VSSVisionClient::~VSSVisionClient(){
    delete _vssClient;
}

void VSSVisionClient::initialization() {
    _portMutex.lock();

    // Vision system connection (firaSim)
    if(_vssClient->open(true))
        std::cout << "[VSSVisionClient] Listening to vision system on port " << _visionPort << " and address = " << _visionAddress.toStdString() << ".\n";
    else {
        std::cout << "[VSSVisionClient] Cannot listen to vision system on port " << _visionPort << " and address = " << _visionAddress.toStdString() << ".\n";
        this->stopRunning();
        _portMutex.unlock();
        return;
    }

    _portMutex.unlock();
}


void VSSVisionClient::loop(){
    fira_message::sim_to_ref::Environment packet;

    // Wait for packet
    if(_vssClient->receive(packet)){
        // Check if packet contains detection data
        if(packet.has_frame()){
            // Save detection data
            fira_message::Frame currDetection = packet.frame();

            // Store detection
            _packetsMutex.lockForWrite();
            _detectionPacket = currDetection;
            _packetsMutex.unlock();

            // Set flag
            _hasDetectionUpdate = true;
        }

        // Check if packet contains geometry data
        if(packet.has_field()){
            // Save detection data
            fira_message::Field currDetection = packet.field();

            // Store detection
            _packetsMutex.lockForWrite();
            _geometryPacket = currDetection;
            _packetsMutex.unlock();

            // Set flag
            _hasGeometryUpdate = true;
        }
    }
}

void VSSVisionClient::finalization(){
    _vssClient->close();
}

bool VSSVisionClient::hasDetectionUpdate() {
    bool retn = _hasDetectionUpdate;
    _hasDetectionUpdate = false;

    return retn;
}

bool VSSVisionClient::hasGeometryUpdate() {
    bool retn = _hasGeometryUpdate;
    _hasGeometryUpdate = false;

    return retn;
}

fira_message::Frame VSSVisionClient::getDetectionData() {
    _packetsMutex.lockForRead();
    fira_message::Frame retn = _detectionPacket;
    _packetsMutex.unlock();

    return retn;
}

fira_message::Field VSSVisionClient::getGeometryData() {
    _packetsMutex.lockForRead();
    fira_message::Field retn = _geometryPacket;
    _packetsMutex.unlock();

    return retn;
}

void VSSVisionClient::setVisionPort(int visionPort) {
    _portMutex.lock();
    _visionPort = visionPort;

    // Stop thread
    this->terminate();
    this->wait();

    // Delete old
    _vssClient->close();
    delete _vssClient;
    _vssClient = NULL;

    // Create new
    _vssClient = new VSSClient(_visionPort, _visionAddress.toStdString());

    // Restart thread
    this->start();
    _portMutex.unlock();
}
