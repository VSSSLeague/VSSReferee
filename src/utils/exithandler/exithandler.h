#ifndef EXITHANDLER_H
#define EXITHANDLER_H

#include <QObject>
#include <QApplication>

class ExitHandler : public QObject
{
public:
    ExitHandler();
    static void setApplication(QApplication *app);
    static void setup();
    static void run(int s);

private:
    static QCoreApplication *_app;
    static int _counter;
};

#endif // EXITHANDLER_H
