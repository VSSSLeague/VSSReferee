#ifndef REFEREEVIEW_H
#define REFEREEVIEW_H

#include <src/entity/entity.h>
#include <src/entity/refereeview/mainwindow/mainwindow.h>

class RefereeView : public Entity
{
public:
    RefereeView();
    ~RefereeView();

    QString name();

    static void updateDetection(fira_message::Frame frame);

private:
    // Entity inherited methods
    void initialization();
    void loop();
    void finalization();

    // UI
    static MainWindow *_refereeUI;
};

#endif // REFEREEVIEW_H
