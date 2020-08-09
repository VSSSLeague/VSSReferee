#ifndef SOCCERVIEW_H
#define SOCCERVIEW_H

#include <QMouseEvent>
#include <QWidget>
#include <QGLWidget>
#include <QMutex>
#include <QVector>
#include <GL/glu.h>

#include "util/timer.h"
#include "util/geometry.h"
#include "util/field.h"
#include "util/util.h"
#include "util/gltext.h"

#include <include/packet.pb.h>

using namespace std;

#define FIELD_COLOR 0.25, 0.25, 0.25, 1.0
#define FIELD_LINES_COLOR 1.0, 1.0, 1.0, 1.0

class SoccerView : public QGLWidget
{
    Q_OBJECT
public:
    SoccerView(QWidget *parent = 0);
    void updateDetection(fira_message::Frame frame);
    void setLeftIsBlue(bool _leftIsBlue) { leftIsBlue = _leftIsBlue; }

    struct FieldDimensions {
        vector<FieldLine*> lines;
        vector<FieldLine*> leftGoalLines;
        vector<FieldLine*> rightGoalLines;
        vector<FieldCircularArc*> arcs;
        vector<FieldTriangle*> triangles;
        double field_length;
        double field_width;
        double boundary_width;
    };

    struct SoccerViewRobot {
        bool hasAngle;
        vector2d loc;
        double angle;
        int id;
        int team;
    };

    typedef enum {
        teamUnknown = 0,
        teamBlue,
        teamYellow
    } TeamTypes;

    vector<std::pair<vector2d, char*>> texts;
    void drawText(vector2d pos, char* str);

private:
    static constexpr double minZValue = -10;
    static constexpr double maxZValue = 10;
    static constexpr double FieldZ = 1.0;
    static constexpr double RobotZ = 2.0;
    static constexpr double BallZ = 3.0;
    static const int PreferedWidth = 552;
    static const int PreferedHeight = 480;
    static constexpr double MinRedrawInterval = 0.01; ///Minimum time between graphics updates (limits the fps)
    static const int unknownRobotID = -1;

    QVector<SoccerViewRobot> robots;

    vector2d ball;
    vector2d ballVelocity;
    QMutex graphicsMutex;
    QMutex attackerMutex;
    GLText glText;

    GLuint blueRobotShape;
    GLuint yellowRobotShape;
    GLuint greyRobotShape;
    GLuint blueCircleRobotShape;
    GLuint yellowCircleRobotShape;
    GLuint greyCircleRobotShape;

    double viewScale; /// Ratio of world space to screen space coordinates
    double viewXOffset;
    double viewYOffset;

    bool leftButton;
    bool midButton;
    bool rightButton;
    int mouseStartX;
    int mouseStartY;

    bool leftIsBlue;

    double tLastRedraw;

    FieldDimensions fieldDim;

private:
    void drawFieldLines(FieldDimensions &dimensions);
    void drawRobots();
    void drawBalls();
    void drawTexts();
    void drawLine(vector2d v1, vector2d v2, double z);
    void drawQuad(vector2d loc1, vector2d loc2, double z=0.0);
    void drawQuad(double x1, double y1, double x2, double y2, double z=0.0){drawQuad(vector2d(x1,y1),vector2d(x2,y2),z);}
    void drawQuad(vector2d v1, vector2d v2, vector2d v3, vector2d v4, double z);
    void drawArc(vector2d loc, double r1, double r2, double theta1, double theta2, double z=0.0, double dTheta = -1);
    void drawTriangle(vector2d v1, vector2d v2, vector2d v3, double z);
    void recomputeProjection();
    void drawRobot(vector2d loc, double theta, double conf, int robotID, int team, bool hasAngle);
    void drawRobot(int team, bool hasAngle, bool useDisplayLists);
    void drawBall(vector2d loc);
    void drawVector(vector2d v1, vector2d v2, double z);
    void vectorTextTest();

    void updateDefaultFieldDimensions();

protected:
    void paintEvent(QPaintEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent *event);
    void initializeGL();
    void resizeGL(int width, int height);
    QSize sizeHint() const { return QSize(PreferedWidth,PreferedHeight); }

public slots:
    void resetView();
private slots:
    void redraw();
signals:
    void postRedraw();
};

#endif // SOCCERVIEW_H
