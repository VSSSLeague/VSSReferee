#include "vssclient.h"

VSSClient::VSSClient(int port,
                     std::string net_address,
                     std::string net_interface)
{
    _port          = port;
    _net_address   = net_address;
    _net_interface = net_interface;
    in_buffer      = new char[65536];
}

VSSClient::~VSSClient(){
    delete[] in_buffer;
}

void VSSClient::close(){
    mc.close();
}

bool VSSClient::open(bool blocking){
    close();
    if(!mc.open(_port, true, true, blocking)){
        fprintf(stderr, "Unable to open UDP network port: %d\n", _port);
        fflush(stderr);
        return(false);
    }

    Net::Address multiaddr,interface;
    multiaddr.setHost(_net_address.c_str(), _port);
    if(_net_interface.length() > 0){
        interface.setHost(_net_interface.c_str(), _port);
    }else{
        interface.setAny();
    }

    if(!mc.addMulticast(multiaddr,interface)) {
        fprintf(stderr, "Unable to setup UDP multicast\n");
        fflush(stderr);
        return(false);
    }

    return(true);
}

bool VSSClient::receive(fira_message::sim_to_ref::Environment &packet){
    Net::Address src;
    int r = 0;
    r = mc.recv(in_buffer, MaxDatagramSize, src);
    if(r > 0){
        fflush(stdout);
        // decode packet:
        return packet.ParseFromArray(in_buffer, r);
    }

    return false;
}

bool VSSClient::receive(VSSRef::team_to_ref::VSSRef_Placement &packet){
    Net::Address src;
    int r = 0;
    r = mc.recv(in_buffer, MaxDatagramSize, src);
    if(r > 0){
        fflush(stdout);
        // decode packet:
        return packet.ParseFromArray(in_buffer, r);
    }

    return false;
}

bool VSSClient::receive(VSSRef::ref_to_team::VSSRef_Command &packet){
    Net::Address src;
    int r = 0;
    r = mc.recv(in_buffer, MaxDatagramSize, src);
    if(r > 0){
        fflush(stdout);
        // decode packet:
        return packet.ParseFromArray(in_buffer, r);
    }

    return false;
}
