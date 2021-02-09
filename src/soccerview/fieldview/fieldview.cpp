#include "fieldview.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"
#define FIELD_COLOR 0.208, 0.208, 0.208, 1.0
#define FIELD_LINES_COLOR 1.0, 1.0, 1.0, 1.0

FieldView::FieldView(QWidget *parent) : QGLWidget(QGLFormat(QGL::DoubleBuffer | QGL::DepthBuffer | QGL::SampleBuffers), parent) {
    // Reset view
    resetView();

    // Avoid auto fill background
    setAutoFillBackground(false);

    // Connect postRedraw to redraw
    connect(this, SIGNAL(postRedraw()), this, SLOT(redraw()));
}

void FieldView::setVisionModule(Vision *visionPointer) {
    vision = visionPointer;
}

void FieldView::setConstants(Constants *constantsPointer) {
    constants = constantsPointer;
}

void FieldView::recomputeProjection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5 * viewScale * width() + viewXOffset, 0.5 * viewScale * width() + viewXOffset, -0.5 * viewScale * height() + viewYOffset, 0.5 * viewScale * height() + viewYOffset, minZValue, maxZValue);
    glMatrixMode(GL_MODELVIEW);
}

void FieldView::paintEvent(QPaintEvent* event) {
    graphicsMutex.lock();

    makeCurrent();
    glClearColor(FIELD_COLOR);
    glShadeModel(GL_SMOOTH);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable( GL_BLEND );
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    drawFieldLines();
    drawFieldObjects();

    glPopMatrix();
    swapBuffers();

    graphicsMutex.unlock();
}

void FieldView::wheelEvent(QWheelEvent* event) {
    double zoomRatio = -double(event->delta()) / 1000.0;
    viewScale = viewScale * (1.0 + zoomRatio);
    recomputeProjection();
    redraw();
}

void FieldView::mouseMoveEvent(QMouseEvent* event) {
    bool leftButton = event->buttons().testFlag(Qt::LeftButton);
    bool midButton = event->buttons().testFlag(Qt::MidButton);

    if(leftButton) {
        //Pan
        viewXOffset -= viewScale * double(event->x() - mouseStartX);
        viewYOffset += viewScale * double(event->y() - mouseStartY);
        mouseStartX = event->x();
        mouseStartY = event->y();
        recomputeProjection();
        redraw();
    }
    else if(midButton) {
        //Zoom
        double zoomRatio = double(event->y() - mouseStartY)/500.0;
        viewScale = viewScale * (1.0 + zoomRatio);
        recomputeProjection();
        mouseStartX = event->x();
        mouseStartY = event->y();
        redraw();
    }
}

void FieldView::mousePressEvent(QMouseEvent* event) {
    bool leftButton = event->buttons().testFlag(Qt::LeftButton);
    bool midButton = event->buttons().testFlag(Qt::MidButton);

    if(leftButton) {
        setCursor(Qt::ClosedHandCursor);
    }
    if(midButton) {
        setCursor(Qt::SizeVerCursor);
    }
    if(leftButton || midButton) {
        // Start Pan / Zoom
        mouseStartX = event->x();
        mouseStartY = event->y();
        redraw();
    }
}

void FieldView::keyPressEvent(QKeyEvent* event) {
    if(event->key() == Qt::Key_Space) {
        resetView();
    }
}

void FieldView::resizeEvent(QResizeEvent* event) {
    QGLWidget::resizeEvent(event);
    redraw();

    resetView();
}


void FieldView::initializeGL() {

}

void FieldView::resizeGL(int width, int height) {
    glViewport(0, 0, width, height);
    recomputeProjection();
}

void FieldView::mouseReleaseEvent(QMouseEvent* event) {
    setCursor(Qt::ArrowCursor);
}

void FieldView::drawFieldLines() {
    glColor4f(FIELD_LINES_COLOR);

    // Draw field lines
    for(size_t i = 0; i < Field_Default_3v3::kNumFieldLines; i++) {
        const FieldLine& line = Field_Default_3v3::kFieldLines[i];
        drawFieldLine(line);
    }

    // Draw left goal lines
    if(getConstants()->blueIsLeftSide()) {
        glColor3d(0.2549, 0.4941, 1.0);
    }
    else {
        glColor3d(1.0, 0.9529, 0.2431);
    }
    for(size_t i = 0; i < Field_Default_3v3::kNumLeftGoalLines; i++) {
        const FieldLine& line = Field_Default_3v3::kLeftGoalLines[i];
        drawFieldLine(line);
    }

    if(getConstants()->blueIsLeftSide()) {
        glColor3d(1.0, 0.9529, 0.2431);
    }
    else {
        glColor3d(0.2549, 0.4941, 1.0);
    }
    // Draw right goal lines
    for(size_t i = 0; i < Field_Default_3v3::kNumRightGoalLines; i++) {
        const FieldLine& line = Field_Default_3v3::kRightGoalLines[i];
        drawFieldLine(line);
    }

    // Draw field arcs
    glColor4f(FIELD_LINES_COLOR);
    for(size_t i = 0; i < Field_Default_3v3::kNumFieldArcs; i++) {
        const FieldCircularArc& arc = Field_Default_3v3::kFieldArcs[i];
        const double half_thickness = 0.5 * arc.thickness;
        const double radius = arc.radius;
        const QVector2D center(arc.center_x, arc.center_y);
        const double a1 = arc.a1;
        const double a2 = arc.a2;

        drawArc(center, radius - half_thickness, radius + half_thickness, a1, a2, FieldZ);
    }

    // Draw field triangles
    for(size_t i = 0; i < Field_Default_3v3::kNumFieldTriangles; i++) {
        const FieldTriangle& triangle = Field_Default_3v3::kFieldTriangles[i];
        const QVector2D v1(triangle.p1_x, triangle.p1_y);
        const QVector2D v2(triangle.p2_x, triangle.p2_y);
        const QVector2D v3(triangle.p3_x, triangle.p3_y);
        drawTriangle(v1, v2, v3, FieldZ);
    }
}

void FieldView::drawFieldLine(const FieldLine &line) {
    const double half_thickness = 0.5 * line.thickness;
    const QVector2D point1(line.p1_x, line.p1_y);
    const QVector2D point2(line.p2_x, line.p2_y);

    // Calc perpendicular
    const QVector2D perpendicular = QVector2D(-(point2 - point1).normalized().y(), (point2 - point1).normalized().x());

    // Get vertices
    const QVector2D v1 = point1 - (half_thickness * perpendicular);
    const QVector2D v2 = point1 + (half_thickness * perpendicular);
    const QVector2D v3 = point2 + (half_thickness * perpendicular);
    const QVector2D v4 = point2 - (half_thickness * perpendicular);

    // Draw quad
    drawQuad(v1, v2, v3, v4, FieldZ);
}

void FieldView::drawQuad(QVector2D point1, QVector2D point2, double z) {
    glBegin(GL_QUADS);
    glVertex3d(point1.x(), point1.y(), z);
    glVertex3d(point2.x(), point1.y(), z);
    glVertex3d(point2.x(), point2.y(), z);
    glVertex3d(point1.x(), point2.y(), z);
    glEnd();
}

void FieldView::drawQuad(QVector2D vert1, QVector2D vert2, QVector2D vert3, QVector2D vert4, double z) {
    glBegin(GL_QUADS);
    glVertex3d(vert1.x(), vert1.y(), z);
    glVertex3d(vert2.x(), vert2.y(), z);
    glVertex3d(vert3.x(), vert3.y(), z);
    glVertex3d(vert4.x(), vert4.y(), z);
    glEnd();
}

void FieldView::drawArc(QVector2D loc, double r1, double r2, double theta1, double theta2, double z, double dTheta) {
    static const double tesselation = 1.0;

    if(dTheta < 0.0) {
        dTheta = tesselation/r2;
    }

    glBegin(GL_QUAD_STRIP);

    for(double theta=theta1; theta<theta2; theta+=dTheta) {
        double c1 = cos(theta), s1 = sin(theta);
        glVertex3d(r2 * c1 + loc.x(), r2 * s1 + loc.y(), z);
        glVertex3d(r1 * c1 + loc.x(), r1 * s1 + loc.y(), z);
    }

    double c1 = cos(theta2), s1 = sin(theta2);
    glVertex3d(r2 * c1 + loc.x(),r2 * s1 + loc.y(), z);
    glVertex3d(r1 * c1 + loc.x(),r1 * s1 + loc.y(), z);

    glEnd();
}

void FieldView::drawTriangle(QVector2D v1, QVector2D v2, QVector2D v3, double z) {
    glBegin(GL_TRIANGLES);
    glVertex3d(v1.x(), v1.y(), z);
    glVertex3d(v2.x(), v2.y(), z);
    glVertex3d(v3.x(), v3.y(), z);
    glEnd();
}

void FieldView::drawRobot(VSSRef::Color teamColor, quint8 robotId, QVector2D robotPosition, double robotOrientation) {
    glPushMatrix();
    glLoadIdentity();
    glTranslated(robotPosition.x(), robotPosition.y(), 0.0);

    glRotated(robotOrientation, 0, 0, 1.0);

    // Robot id
    glColor3d(0.0,0.0,0.0);
    char buffer[8];
    sprintf(buffer, "%d", robotId);
    glText.drawString(robotPosition, 0, 50, buffer, GLText::CenterAligned, GLText::MiddleAligned);

    // Take team color (draw robot border)
    switch (teamColor) {
    case VSSRef::Color::BLUE: {
        glColor3d(0.2549, 0.4941, 1.0);
    }
    break;
    case VSSRef::Color::YELLOW: {
        glColor3d(1.0, 0.9529, 0.2431);
    }
    break;
    default: {
        glColor3d(0.5882,0.5882,0.5882);
    }
    break;
    }
    drawQuad(QVector2D(-35, 35), QVector2D(35,-35), RobotZ);

    // Take team color (draw shape)
    switch(teamColor) {
    case VSSRef::Color::BLUE:{
        glColor3d(0.0706, 0.2314, 0.6275);
    }
    break;
    case VSSRef::Color::YELLOW:{
        glColor3d(0.8, 0.6157, 0.0);
    }
    break;
    default:{
        glColor3d(0.2745,0.2745,0.2745);
    }
    break;
    }

    // Robot shape
    drawQuad(QVector2D(-35, 35), QVector2D(35, -35), RobotZ);
    drawQuad(QVector2D(-40, 40), QVector2D(40, 35), RobotZ+0.01);
    drawQuad(QVector2D(-40, -40), QVector2D(40, -35), RobotZ+0.01);
    drawQuad(QVector2D(40, -40), QVector2D(35, 40), RobotZ+0.01);
    drawQuad(QVector2D(-40, -40), QVector2D(-35, 40), RobotZ+0.01);

    // Wheels
    glColor3d(0.5, 0.5, 0.5);
    drawQuad(QVector2D(-20, 47), QVector2D(20, 40), RobotZ+0.02);
    drawQuad(QVector2D(-20, -47), QVector2D(20,-40), RobotZ+0.02);

    glPopMatrix();
}

void FieldView::drawFieldObjects() {
    // Draw ball
    QVector2D ballPosition = QVector2D(getVision()->getBallPosition().x() * 1000.0, getVision()->getBallPosition().y() * 1000.0);
    glColor3d(1.0,0.5059,0.0);
    drawArc(ballPosition, 0, 16, -M_PI, M_PI, BallZ);
    glColor3d(0.8706,0.3490,0.0);
    drawArc(ballPosition, 15, 21, -M_PI, M_PI, BallZ);

    // Draw teams robots
    for(int i = VSSRef::Color::BLUE; i <= VSSRef::Color::YELLOW; i++) {
        QList<quint8> avPlayers = getVision()->getAvailablePlayers(VSSRef::Color(i));
        for(int j = 0; j < avPlayers.size(); j++) {
            Position robotPosition = getVision()->getPlayerPosition(VSSRef::Color(i), avPlayers.at(j));
            Angle robotOrientation = getVision()->getPlayerOrientation(VSSRef::Color(i), avPlayers.at(j));
            drawRobot(VSSRef::Color(i), avPlayers.at(j), QVector2D(robotPosition.x() * 1000.0, robotPosition.y() * 1000.0), (robotOrientation.value() * (180.0/M_PI)));
        }
    }
}

void FieldView::resetView() {
    viewScale = (Field_Default_3v3::kFieldLength + Field_Default_3v3::kBoundaryWidth) / sizeHint().width();
    viewScale = std::max(viewScale, (Field_Default_3v3::kFieldWidth + Field_Default_3v3::kBoundaryWidth) / sizeHint().height());

    viewXOffset = viewYOffset = 0.0;
    recomputeProjection();
    redraw();
}

void FieldView::updateField() {
    emit postRedraw();
}

void FieldView::redraw() {
    // Stop timer
    timer.stop();

    // Check if passed timer is ok
    if(timer.getSeconds() < MinRedrawInterval) {
        return;
    }

    // Update graphics
    graphicsMutex.lock();
    update();
    graphicsMutex.unlock();

    // Re-start timer
    timer.start();
}

Vision* FieldView::getVision() {
    if(vision == nullptr) {
        std::cout << Text::red("[ERROR] ", true) << Text::bold("Vision with nullptr value at FieldView") + '\n';
    }
    else {
        return vision;
    }

    return nullptr;
}

Constants* FieldView::getConstants() {
    if(vision == nullptr) {
        std::cout << Text::red("[ERROR] ", true) << Text::bold("Constants with nullptr value at FieldView") + '\n';
    }
    else {
        return constants;
    }

    return nullptr;
}
