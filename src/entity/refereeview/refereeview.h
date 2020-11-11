#ifndef REFEREEVIEW_H
#define REFEREEVIEW_H

#include <src/entity/entity.h>
#include <src/entity/refereeview/mainwindow/mainwindow.h>

class RefereeView : public Entity
{
public:
    RefereeView(Constants *constants);
    ~RefereeView();

    QString name();

    static void updateDetection(fira_message::Frame frame);
    static void addRefereeCommand(QString command);
    static void setCurrentTime(int time);
    static void setRefereeCommand(QString command);
    static bool getBlueIsLeftSide();

    // GUI
    static void drawText(vector2d pos, char *str);
    static void setTeams(QString teamLeft, VSSRef::Color leftColor, QString teamRight, VSSRef::Color rightColor);

    // UI
    static MainWindow* getUI();

private:
    // Entity inherited methods
    void initialization();
    void loop();
    void finalization();

    // UI
    static MainWindow *_refereeUI;

    // Constants
    Constants *_constants;
    Constants* getConstants();
};

#endif // REFEREEVIEW_H
