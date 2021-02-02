#include <src/utils/exithandler/exithandler.h>
#include <src/utils/text/text.h>
#include <src/constants/constants.h>
#include <src/refereecore.h>

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

    // Setup ExitHandler
    ExitHandler::setApplication(&app);
    ExitHandler::setup();

    // Initializing constants
    Constants *constants = new Constants(QString(PROJECT_PATH) + "/src/constants/constants.json");

    // Initializating referee core
    RefereeCore *refereeCore = new RefereeCore(constants);
    refereeCore->start();

    // Wait for app exec
    bool exec = app.exec();

    // Stopping and deleting referee core
    refereeCore->stop();
    delete refereeCore;

    // Deleting constants
    delete constants;

    return exec;
}
