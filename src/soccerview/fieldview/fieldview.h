#ifndef FIELDVIEW_H
#define FIELDVIEW_H

#include <QGLWidget>
#include <QMutex>
#include <QMouseEvent>

#include <src/soccerview/fieldview/gltext/gltext.h>
#include <src/world/entities/vision/vision.h>
#include <src/utils/types/field/field_default_3v3.h>
#include <src/utils/timer/timer.h>
#include <include/vssref_common.pb.h>
#include <include/packet.pb.h>

class FieldView : public QGLWidget
{
    Q_OBJECT
public:
    FieldView(QWidget *parent = 0);
    void setVisionModule(Vision *visionPointer);
    void setConstants(Constants *constantsPointer);

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
    QSize sizeHint() const { return QSize(PreferedWidth, PreferedHeight); }

private:
    // GL Constants
    static constexpr double minZValue = -10;
    static constexpr double maxZValue = 10;
    static constexpr double FieldZ = 1.0;
    static constexpr double BallZ = 2.0;
    static constexpr double RobotZ = 3.0;
    static const int PreferedWidth = 750; // widgetWidth * 1.23
    static const int PreferedHeight = 516; // widgetHeight * 1.23
    static constexpr double MinRedrawInterval = 0.01; ///Minimum time between graphics updates (limits the fps)

    // Mouse events
    double viewScale;
    double viewXOffset;
    double viewYOffset;
    int mouseStartX;
    int mouseStartY;

    // Vision
    Vision *vision;
    Vision* getVision();

    // Constants
    Constants *constants;
    Constants* getConstants();

    // Util functions
    void recomputeProjection();
    void drawFieldLines();
    void drawFieldLine(const FieldLine &fieldLine);
    void drawQuad(QVector2D point1, QVector2D point2, double z);
    void drawQuad(QVector2D vert1, QVector2D vert2, QVector2D vert3, QVector2D vert4, double z);
    void drawArc(QVector2D loc, double r1, double r2, double theta1, double theta2, double z = 0.0, double dTheta = -1);
    void drawTriangle(QVector2D v1, QVector2D v2, QVector2D v3, double z);
    void drawRobot(VSSRef::Color teamColor, quint8 robotId, QVector2D robotPosition, double robotOrientation);
    void drawFieldObjects();

    // Graphics mutex
    QMutex graphicsMutex;

    // GLText
    GLText glText;

    // Timer
    Timer timer;

signals:
    void postRedraw();

public slots:
    void resetView();
    void updateField();

private slots:
    void redraw();
};

#endif // FIELDVIEW_H
