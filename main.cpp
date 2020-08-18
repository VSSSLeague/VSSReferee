#include <QApplication>

#include <src/entity/vssvisionclient/vssvisionclient.h>
#include <src/entity/vssreferee/vssreferee.h>
#include <src/entity/vssreplacer/vssreplacer.h>
#include <src/entity/refereeview/refereeview.h>

#include <constants/constants.h>

#include <exithandler.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Creating constants pointer
    Constants* constants = new Constants("../constants/constants.json");

    /// Parsing from constants
    // Vision
    QString visionAddress   = constants->getVisionAddress();
    int visionPort          = constants->getVisionPort();

    // Referee
    QString refereeAddress  = constants->getRefereeAddress();
    int refereePort         = constants->getRefereePort();

    // Replacer
    QString replacerAddress = constants->getReplacerAddress();
    int replacerPort        = constants->getRefereePort();

    // FiraSim
    QString firaSimAddress  = constants->getFiraSimAddress();
    int firaSimPort         = constants->getFiraSimPort();

    // Setup ExitHandler
    ExitHandler::setApplication(&a);
    ExitHandler::setup();

    // Create modules
    VSSVisionClient *vssVisionClient = new VSSVisionClient(visionAddress, visionPort);
    VSSReferee *vssReferee = new VSSReferee(vssVisionClient, refereeAddress, refereePort, constants);
    VSSReplacer *vssReplacer = new VSSReplacer(replacerAddress, replacerPort, firaSimAddress, firaSimPort);
    RefereeView *refView = new RefereeView();

    // Make connections with signals and slots
    QObject::connect(vssReferee, SIGNAL(setFoul(VSSRef::Foul)), vssReplacer, SLOT(takeFoul(VSSRef::Foul)), Qt::DirectConnection);
    QObject::connect(vssReplacer, SIGNAL(teamPlaced(VSSRef::Color)), vssReferee, SLOT(teamSent(VSSRef::Color)), Qt::DirectConnection);
    QObject::connect(vssReferee, SIGNAL(halfPassed()), refView->getUI(), SLOT(switchSides()));

    // Start all
    vssVisionClient->start();
    vssReferee->start();
    vssReplacer->start();
    refView->start();

    // Run
    bool exec = a.exec();

    // Stop modules
    vssVisionClient->terminate();
    vssReferee->terminate();
    vssReplacer->terminate();
    refView->terminate();

    // Wait for modules sync
    vssVisionClient->wait();
    vssReferee->wait();
    vssReplacer->wait();
    refView->wait();

    return exec;
}
