#include <QCommandLineParser>
#include <QCommandLineOption>

#include <src/utils/exithandler/exithandler.h>
#include <src/refereecore.h>
#include <src/recorder/recorder.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationVersion(APP_VERSION);

    // Showing banner
    std::cout << Text::bold(Text::center("__     ______ ____  ____       __                    ")) + '\n';
    std::cout << Text::bold(Text::center("\\ \\   / / ___/ ___||  _ \\ ___ / _| ___ _ __ ___  ___ ")) + '\n';
    std::cout << Text::bold(Text::center(" \\ \\ / /\\___ \\___ \\| |_) / _ \\ |_ / _ \\ \'__/ _ \\/ _ \\")) + '\n';
    std::cout << Text::bold(Text::center("  \\ V /  ___) |__) |  _ <  __/  _|  __/ | |  __/  __/")) + '\n';
    std::cout << Text::bold(Text::center("   \\_/  |____/____/|_| \\_\\___|_|  \\___|_|  \\___|\\___|")) + '\n';
    std::cout << Text::bold(Text::center("VSSLeague Software - Version " + app.applicationVersion().toStdString())) + '\n' + '\n';

    // Setup command line parser
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    // Setup options
    // 5v5
    QCommandLineOption use5v5Option("5v5", "Use referee in 5v5 mode");
    parser.addOption(use5v5Option);

    // 3v3
    QCommandLineOption use3v3Option("3v3", "Use referee in 3v3 mode");
    parser.addOption(use3v3Option);

    // Process parser in app
    parser.process(app);

    // Setup ExitHandler
    ExitHandler::setApplication(&app);
    ExitHandler::setup();

    // Initializing constants
    Constants *constants = new Constants(QString(PROJECT_PATH) + "/src/constants/constants.json");

    // Check if 3v3 or 5v5 option is set, otherwise close
    if(parser.isSet(use5v5Option) || parser.isSet(use3v3Option)) {
        constants->setIs5v5(parser.isSet(use5v5Option));
    }
    else {
        std::cout << Text::red("[ERROR] ", true) + Text::bold("You need to explicitly use --3v3 or --5v5 flags.") + '\n';
        return 0;
    }

    // Initializating referee core
    RefereeCore *refereeCore = new RefereeCore(constants);
    refereeCore->start();

    // Initializing log recorder
    QString logFileName = PROJECT_PATH + QString("/logs/") + Timer::getActualTime() + QString("|%1 - %2_%3").arg(constants->gameType()).arg(constants->blueTeamName()).arg(constants->yellowTeamName()) +  ".log";
    Recorder *recorder = new Recorder(logFileName, constants->visionAddress(), constants->visionPort(), constants->refereeAddress(), constants->refereePort());

    // Wait for app exec
    bool exec = app.exec();

    // Stopping and deleting referee core
    refereeCore->stop();
    delete refereeCore;

    // Delete recorder
    delete recorder;

    // Deleting constants
    delete constants;

    return exec;
}
