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

    /// Creating constants pointer
    Constants* constants = new Constants(":/constants/constants.json");
    Utils::setConstants(constants);

    /// Parsing from constants
    // Vision
    QString visionAddress   = constants->getVisionAddress();
    int visionPort          = constants->getVisionPort();

    // Referee
    QString refereeAddress  = constants->getRefereeAddress();
    int refereePort         = constants->getRefereePort();

    // Replacer
    QString replacerAddress = constants->getReplacerAddress();
    int replacerPort        = constants->getReplacerPort();

    // FiraSim
    QString firaSimAddress  = constants->getFiraSimAddress();
    int firaSimPort         = constants->getFiraSimPort();

    /// Setup ExitHandler
    ExitHandler::setApplication(&a);
    ExitHandler::setup();

    /// Create modules
    VSSVisionClient *vssVisionClient = new VSSVisionClient(visionAddress, visionPort);
    VSSReferee *vssReferee = new VSSReferee(vssVisionClient, refereeAddress, refereePort, constants);
    VSSReplacer *vssReplacer = new VSSReplacer(replacerAddress, replacerPort, firaSimAddress, firaSimPort, constants);
    RefereeView *refView = new RefereeView();

    /// Make connections with signals and slots
    // Foul connetion
    QObject::connect(vssReferee, SIGNAL(setFoul(VSSRef::Foul, VSSRef::Color, VSSRef::Quadrant)), vssReplacer, SLOT(takeFoul(VSSRef::Foul, VSSRef::Color, VSSRef::Quadrant)), Qt::DirectConnection);
    QObject::connect(vssReferee, SIGNAL(stopReplacerWaiting()), vssReplacer, SLOT(stopWaiting()), Qt::DirectConnection);
    QObject::connect(vssReplacer, SIGNAL(teamPlaced(VSSRef::Color)), vssReferee, SLOT(teamSent(VSSRef::Color)), Qt::DirectConnection);

    // Half connection
    //QObject::connect(vssReferee, SIGNAL(halfPassed()), refView->getUI(), SLOT(switchSides()), Qt::DirectConnection);

    // Goalie connection
    QObject::connect(vssReferee, SIGNAL(sendGoalie(VSSRef::Color, int)), vssReplacer, SLOT(takeGoalie(VSSRef::Color, int)), Qt::DirectConnection);
    QObject::connect(vssReplacer, SIGNAL(requestGoalie(VSSRef::Color)), vssReferee, SLOT(requestGoalie(VSSRef::Color)), Qt::DirectConnection);

    // Goal connection
    QObject::connect(vssReferee, SIGNAL(goalMarked(VSSRef::Color)), refView->getUI(), SLOT(addGoal(VSSRef::Color)), Qt::DirectConnection);

    // Manual referee connection
    QObject::connect(refView->getUI(), SIGNAL(sendManualCommand(VSSRef::Foul, VSSRef::Color, VSSRef::Quadrant)), vssReferee, SLOT(takeManualCommand(VSSRef::Foul, VSSRef::Color, VSSRef::Quadrant)));

    /// Set team
    // Command line parser, get arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("VSSReferee application help.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("teamBlueName", "Sets the team blue name");
    parser.addPositionalArgument("teamYellowName", "Sets the team yellow name");
    parser.process(a);
    QStringList args = parser.positionalArguments();

    // Setting default (by json)
    refView->setTeams(constants->getLeftTeamName(), constants->getLeftTeamColor(), constants->getRightTeamName(), constants->getRightTeamColor());

    // Setting by args
    if(args.size() >= 2){
        refView->setTeams(args.at(0), VSSRef::Color::BLUE, args.at(1), VSSRef::Color::YELLOW);
    }

    /// Start all
    vssVisionClient->start();
    vssReferee->start();
    vssReplacer->start();
    refView->start();

    /// Run app
    bool exec = a.exec();

    /// Stop modules
    vssVisionClient->terminate();
    vssReferee->terminate();
    vssReplacer->terminate();
    refView->terminate();

    /// Wait for modules sync
    vssVisionClient->wait();
    vssReferee->wait();
    vssReplacer->wait();
    refView->wait();

    return exec;
}
