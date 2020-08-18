#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "exithandler.h"
#include "signal.h"
#include <iostream>

QApplication* ExitHandler::_app = NULL;
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
        case 1:
            std::cout << "\n[EXIT HANDLER] " <<  "Closing VSSReferee...\n";
            // Close application
            ExitHandler::_app->exit();
        break;
        case 2:
            std::cout << "\n[EXIT HANDLER] " << "Halting VSSReferee....\n";
            exit(EXIT_FAILURE);
        break;
    }
}
