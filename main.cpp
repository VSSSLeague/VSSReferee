#include <src/utils/exithandler/exithandler.h>
#include <src/utils/text/text.h>

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

    bool exec = app.exec();

    return exec;
}
