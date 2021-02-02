#pragma GCC diagnostic ignored "-Wunused-parameter"

#include "exithandler.h"

#include <signal.h>
#include <src/utils/text/text.h>

QCoreApplication* ExitHandler::_app = NULL;
int ExitHandler::_counter = 0;

ExitHandler::ExitHandler() {

}

void ExitHandler::setApplication(QApplication *app) {
    ExitHandler::_app = app;
}

void ExitHandler::setup() {
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = ExitHandler::run;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
}

void ExitHandler::run(int s) {
    ExitHandler::_counter++;
    switch(ExitHandler::_counter) {
        case 1: {
            // Close application
            ExitHandler::_app->exit();
        }
        break;
        case 2: {
            // Force exit
            exit(EXIT_FAILURE);
        }
        break;
    }
}
