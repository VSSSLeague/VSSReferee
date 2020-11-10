#include <QtCore>
#include <QUdpSocket>
#include <iostream>

#include <random>
#include <chrono>

#include <include/vssclient/vssclient.h>

#define UDP_ADDRESS "224.5.23.2"
#define REFEREE_PORT 10003
#define REPLACER_PORT 10004

QString getFoulNameById(VSSRef::Foul foul){
    switch(foul){
        case VSSRef::Foul::FREE_BALL:    return "FREE_BALL";
        case VSSRef::Foul::FREE_KICK:    return "FREE_KICK";
        case VSSRef::Foul::GOAL_KICK:    return "GOAL_KICK";
        case VSSRef::Foul::PENALTY_KICK: return "PENALTY_KICK";
        case VSSRef::Foul::KICKOFF:      return "KICKOFF";
        case VSSRef::Foul::STOP:         return "STOP";
        case VSSRef::Foul::GAME_ON:      return "GAME_ON";
        default:                         return "FOUL NOT IDENTIFIED";
    }
}

QString getTeamColorNameById(VSSRef::Color color){
    switch(color){
        case VSSRef::Color::NONE:    return "NONE";
        case VSSRef::Color::BLUE:    return "BLUE";
        case VSSRef::Color::YELLOW:  return "YELLOW";
        default:                     return "COLOR NOT IDENTIFIED";
    }
}

QString getQuadrantNameById(VSSRef::Quadrant quadrant){
    switch(quadrant){
        case VSSRef::Quadrant::NO_QUADRANT: return "NO QUADRANT";
        case VSSRef::Quadrant::QUADRANT_1:  return "QUADRANT 1";
        case VSSRef::Quadrant::QUADRANT_2:  return "QUADRANT 2";
        case VSSRef::Quadrant::QUADRANT_3:  return "QUADRANT 3";
        case VSSRef::Quadrant::QUADRANT_4:  return "QUADRANT 4";
        default:                            return "QUADRANT NOT IDENTIFIED";
    }
}

QString getHalfNameById(VSSRef::Half half){
    switch(half){
        case VSSRef::Half::NO_HALF: return "NO_HALF";
        case VSSRef::Half::FIRST_HALF: return "FIRST HALF";
        case VSSRef::Half::SECOND_HALF: return "SECOND HALF";
        default: return "NO HALF DEFINED";
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QUdpSocket *replacerSocket = new QUdpSocket();
    QUdpSocket *refereeClient = new QUdpSocket();

    // Performing connection to send Replacer commands
    if(replacerSocket->isOpen())
        replacerSocket->close();

    replacerSocket->connectToHost(UDP_ADDRESS, REPLACER_PORT, QIODevice::WriteOnly, QAbstractSocket::IPv4Protocol);
    std::cout << "[Example] Connected to REPLACER socket in port " << REPLACER_PORT << " and address = " << UDP_ADDRESS << ".\n";

    // Perfoming connection to receive Referee commands
    // binding port
    if(refereeClient->bind(QHostAddress::AnyIPv4, REFEREE_PORT, QUdpSocket::ShareAddress) == false){
        std::cout << "[ERROR] Failed to bind referee client =(" << std::endl;
        exit(-1);
    }
    // connecting to multicast group in UDP_ADDRESS
    if(refereeClient->joinMulticastGroup(QHostAddress(UDP_ADDRESS)) == false){
        std::cout << "[ERROR] Failed to join VSSReferee multicast group =(" << std::endl;
        exit(-1);
    }

    std::cout << "[Example] Connected to REFEREE socket in port " << REFEREE_PORT << " and address = " << UDP_ADDRESS << ".\n";

    /// Reading messages from referee
    // Receiving packets
    while(true){
        VSSRef::ref_to_team::VSSRef_Command command;

        bool received = false;
        char *buffer = new char[65535];
        long long int packetLength = 0;

        while(refereeClient->hasPendingDatagrams()){
            // Parse message to protobuf
            packetLength = refereeClient->readDatagram(buffer, 65535);
            if(command.ParseFromArray(buffer, int(packetLength)) == false){
                std::cout << "[ERROR] Referee command parsing error =(" << std::endl;
                exit(-1);
            }

            // If received command, let's debug it
            std::cout << "[Example] Succesfully received an command from ref: " << getFoulNameById(command.foul()).toStdString() << " for team " << getTeamColorNameById(command.teamcolor()).toStdString() << std::endl;
            std::cout << getQuadrantNameById(command.foulquadrant()).toStdString() << std::endl;

            // Showing timestamp
            std::cout << "Timestamp: " << command.timestamp() << std::endl;

            // Showing half
            std::cout << "Half: " << getHalfNameById(command.gamehalf()).toStdString() << std::endl;

            // Now let's send an placement packet to it

            // First creating an placement command for the blue team
            VSSRef::team_to_ref::VSSRef_Placement placementCommandBlue;
            VSSRef::Frame *placementFrameBlue = new VSSRef::Frame();
            placementFrameBlue->set_teamcolor(VSSRef::Color::BLUE);
            for(int x = 0; x < 3; x++){
                if(x == 0){
                    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
                    std::mt19937 mt_rand(seed);

                    bool _gkAtTopSide = mt_rand() % 2;

                    VSSRef::Robot *robot = placementFrameBlue->add_robots();
                    robot->set_robot_id(static_cast<uint32_t>(x));
                    robot->set_x(-0.65);
                    robot->set_orientation(0.0);

                    if(_gkAtTopSide)
                        robot->set_y(0.2);
                    else
                        robot->set_y(-0.2);

                }else{
                    VSSRef::Robot *robot = placementFrameBlue->add_robots();
                    robot->set_robot_id(static_cast<uint32_t>(x));
                    robot->set_x(-0.5);
                    robot->set_y(-0.2 + (0.2 * x));
                    robot->set_orientation(0.0);
                }

            }
            placementCommandBlue.set_allocated_world(placementFrameBlue);

            // Sending blue
            std::string msgBlue;
            placementCommandBlue.SerializeToString(&msgBlue);
            if(replacerSocket->write(msgBlue.c_str(), static_cast<qint64>(msgBlue.length())) == -1){
                std::cout << "[Example] Failed to write to replacer socket: " << replacerSocket->errorString().toStdString() << std::endl;
            }

            // Now creating an placement command for the yellow team
            VSSRef::team_to_ref::VSSRef_Placement placementCommandYellow;
            VSSRef::Frame *placementFrameYellow = new VSSRef::Frame();
            placementFrameYellow->set_teamcolor(VSSRef::Color::YELLOW);
            for(int x = 0; x < 3; x++){
                if(x == 0){
                    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
                    std::mt19937 mt_rand(seed);

                    bool _gkAtTopSide = mt_rand() % 2;

                    VSSRef::Robot *robot = placementFrameYellow->add_robots();
                    robot->set_robot_id(static_cast<uint32_t>(x));
                    robot->set_x(0.65);
                    robot->set_orientation(180.0);

                    if(_gkAtTopSide)
                        robot->set_y(0.2);
                    else
                        robot->set_y(-0.2);

                }
                else{
                    VSSRef::Robot *robot = placementFrameYellow->add_robots();
                    robot->set_robot_id(static_cast<uint32_t>(x));
                    robot->set_x(0.5);
                    robot->set_y(-0.2 + (0.2 * x));
                    robot->set_orientation(180.0);
                }

            }
            placementCommandYellow.set_allocated_world(placementFrameYellow);

            // Sending yellow
            std::string msgYellow;
            placementCommandYellow.SerializeToString(&msgYellow);
            if(replacerSocket->write(msgYellow.c_str(), static_cast<qint64>(msgYellow.length())) == -1){
                std::cout << "[Example] Failed to write to replacer socket: " << replacerSocket->errorString().toStdString() << std::endl;
            }

            received = true;
        }

        delete [] buffer;
        if(received) break;
    }

    // Closing sockets
    if(replacerSocket->isOpen())
        replacerSocket->close();

    if(refereeClient->isOpen())
        refereeClient->close();

    bool exec = a.exec();

    return exec;
}
