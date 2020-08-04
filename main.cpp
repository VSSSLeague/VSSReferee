#include <QCoreApplication>

#include <src/entity/vssvisionclient/vssvisionclient.h>
#include <src/entity/vssreferee/vssreferee.h>
#include <src/entity/vssreplacer/vssreplacer.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Defines
    QString refereeAddress = "224.5.23.2";
    int visionPort = 10002;
    int refereePort = 10003;
    int replacerPort = 10004;
    QString firaSimAddress = "127.0.0.1";
    int firaSimCommandPort = 20011;

    /// TODO Here
    /// Parse these address and ports

    // Create modules
    VSSVisionClient *vssVisionClient = new VSSVisionClient(refereeAddress, visionPort);
    VSSReferee *vssReferee = new VSSReferee(vssVisionClient, refereeAddress, refereePort);
    VSSReplacer *vssReplacer = new VSSReplacer(refereeAddress, replacerPort, firaSimAddress, firaSimCommandPort);

    // Make connections with signals and slots
    QObject::connect(vssReferee, SIGNAL(setFoul(VSSRef::Foul)), vssReplacer, SLOT(takeFoul(VSSRef::Foul)), Qt::DirectConnection);
    QObject::connect(vssReplacer, SIGNAL(teamPlaced(VSSRef::Color)), vssReferee, SLOT(teamSent(VSSRef::Color)), Qt::DirectConnection);

    // Start all
    vssVisionClient->start();
    vssReferee->start();
    vssReplacer->start();

    // Run
    bool exec = a.exec();

    // Stop modules
    vssVisionClient->terminate();
    vssReferee->terminate();
    vssReplacer->terminate();

    // Wait for modules sync
    vssVisionClient->wait();
    vssReferee->wait();
    vssReplacer->wait();

    return exec;
}
