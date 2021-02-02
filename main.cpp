#include <QCoreApplication>
#include <src/utils/text/text.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    a.setApplicationVersion(APP_VERSION);

    std::cout << Text::bold(Text::center("__     ______ ____  ____       __                    ")) + '\n';
    std::cout << Text::bold(Text::center("\\ \\   / / ___/ ___||  _ \\ ___ / _| ___ _ __ ___  ___ ")) + '\n';
    std::cout << Text::bold(Text::center(" \\ \\ / /\\___ \\___ \\| |_) / _ \\ |_ / _ \\ \'__/ _ \\/ _ \\")) + '\n';
    std::cout << Text::bold(Text::center("  \\ V /  ___) |__) |  _ <  __/  _|  __/ | |  __/  __/")) + '\n';
    std::cout << Text::bold(Text::center("   \\_/  |____/____/|_| \\_\\___|_|  \\___|_|  \\___|\\___|")) + '\n';
    std::cout << Text::bold(Text::center("VSSLeague Software - Version " + a.applicationVersion().toStdString())) + '\n' + '\n';


    return a.exec();
}
