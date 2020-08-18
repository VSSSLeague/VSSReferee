#ifndef EXITHANDLER_H
#define EXITHANDLER_H

#include <QObject>
#include <QApplication>

class ExitHandler : public QObject {
private:
    static QApplication *_app;
    static int _counter;

public:
    ExitHandler();
    static void setApplication(QApplication *app);
    static void setup();
    static void run(int s);
};

#endif // EXITHANDLER_H
