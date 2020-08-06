#include "soccerview.h"

#include <src/entity/refereeview/soccerview/util/field_default_constants.h>

SoccerView::SoccerView(QWidget *parent) : QGLWidget(QGLFormat(QGL::DoubleBuffer | QGL::DepthBuffer | QGL::SampleBuffers),parent)
{
    viewScale = (fieldDim.field_length + fieldDim.boundary_width) / sizeHint().width();
    viewScale = max(viewScale, (fieldDim.field_width + fieldDim.boundary_width) / sizeHint().height());

    viewXOffset = viewYOffset = 0.0;
    setAutoFillBackground(false); //Do not let painter auto fill the widget's background: we'll do it manually through openGl
    connect(this, SIGNAL(postRedraw()), this, SLOT(redraw()));
    blueRobotShape = GL_INVALID_VALUE;
    yellowRobotShape = GL_INVALID_VALUE;
    greyRobotShape = GL_INVALID_VALUE;
    blueCircleRobotShape = GL_INVALID_VALUE;
    yellowCircleRobotShape = GL_INVALID_VALUE;
    greyCircleRobotShape = GL_INVALID_VALUE;
    QFont RobotIDFont = this->font();
    RobotIDFont.setWeight(QFont::Bold);
    RobotIDFont.setPointSize(80);
    glText = GLText(RobotIDFont);
    tLastRedraw = 0;
    leftIsBlue = true;

    updateDefaultFieldDimensions();
    resetView();
}

void SoccerView::updateDefaultFieldDimensions() {
    fieldDim.field_length = FieldConstantsVSS::kFieldLength;
    fieldDim.field_width = FieldConstantsVSS::kFieldWidth;
    fieldDim.boundary_width = FieldConstantsVSS::kBoundaryWidth;

    fieldDim.lines.clear();

    for(size_t i=0; i<FieldConstantsVSS::kNumFieldLines; i++)
        fieldDim.lines.push_back(new FieldLine(FieldConstantsVSS::kFieldLines[i]));

    fieldDim.leftGoalLines.clear();

    for(size_t i=0; i<FieldConstantsVSS::kNumLeftGoalLines; i++)
        fieldDim.leftGoalLines.push_back(new FieldLine(FieldConstantsVSS::kLeftGoalLines[i]));

    fieldDim.rightGoalLines.clear();

    for(size_t i=0; i<FieldConstantsVSS::kNumRightGoalLines; i++)
        fieldDim.rightGoalLines.push_back(new FieldLine(FieldConstantsVSS::kRightGoalLines[i]));

    fieldDim.arcs.clear();

    for(size_t i=0; i<FieldConstantsVSS::kNumFieldArcs; i++)
        fieldDim.arcs.push_back(new FieldCircularArc(FieldConstantsVSS::kFieldArcs[i]));

    fieldDim.triangles.clear();

    for(size_t i=0; i<FieldConstantsVSS::kNumFieldTriangles; i++)
        fieldDim.triangles.push_back(new FieldTriangle(FieldConstantsVSS::kFieldTriangles[i]));
}

void SoccerView::redraw(){
    if(GetTimeSec()-tLastRedraw < MinRedrawInterval)
        return;

    graphicsMutex.lock();
    update();
    graphicsMutex.unlock();
    tLastRedraw = GetTimeSec();
}

void SoccerView::mousePressEvent(QMouseEvent* event)
{
    leftButton = event->buttons().testFlag(Qt::LeftButton);
    midButton = event->buttons().testFlag(Qt::MidButton);
    rightButton = event->buttons().testFlag(Qt::RightButton);
//  bool shiftKey = event->modifiers().testFlag(Qt::ShiftModifier);
//  bool ctrlKey = event->modifiers().testFlag(Qt::ControlModifier);

    if(leftButton)
        setCursor(Qt::ClosedHandCursor);
    if(midButton)
        setCursor(Qt::SizeVerCursor);
    if(leftButton || midButton) {
        // Start Pan / Zoom
        mouseStartX = event->x();
        mouseStartY = event->y();
        redraw();
    }
}

void SoccerView::mouseReleaseEvent(QMouseEvent* event)
{
//  bool shiftKey = event->modifiers().testFlag(Qt::ShiftModifier);
//  bool ctrlKey = event->modifiers().testFlag(Qt::ControlModifier);
    setCursor(Qt::ArrowCursor);
}

void SoccerView::mouseMoveEvent(QMouseEvent* event)
{
    static const bool debug = false;
    bool leftButton = event->buttons().testFlag(Qt::LeftButton);
    bool midButton = event->buttons().testFlag(Qt::MidButton);
    bool rightButton = event->buttons().testFlag(Qt::RightButton);

    if(debug) printf("MouseMove Event, Left:%d Mid:%d Right:%d\n", leftButton?1:0, midButton?1:0, rightButton?1:0);
    if(leftButton) {
        //Pan
        viewXOffset -= viewScale*double(event->x() - mouseStartX);
        viewYOffset += viewScale*double(event->y() - mouseStartY);
        mouseStartX = event->x();
        mouseStartY = event->y();
        recomputeProjection();
        redraw();
    } else if(midButton) {
        //Zoom
        double zoomRatio = double(event->y() - mouseStartY)/500.0;
        //    double oldScale = viewScale;
        viewScale = viewScale*(1.0+zoomRatio);
        recomputeProjection();
        mouseStartX = event->x();
        mouseStartY = event->y();
        redraw();
    }
}

void SoccerView::wheelEvent(QWheelEvent* event)
{
  static const bool debug = false;
  double zoomRatio = -double(event->delta())/1000.0;
//  double oldScale = viewScale;
  viewScale = viewScale*(1.0+zoomRatio);
  recomputeProjection();
  if(debug) printf("Zoom: %5.3f\n",viewScale);
  redraw();
}

void SoccerView::keyPressEvent(QKeyEvent* event)
{
  static const bool debug = false;
  if(debug) printf("KeyPress: 0x%08X\n",event->key());
  if(event->key() == Qt::Key_Space)
    resetView();
  if(event->key() == Qt::Key_Escape)
    close();
}

void SoccerView::resetView()
{
    viewScale = (fieldDim.field_length + fieldDim.boundary_width) / width();
    viewScale = max(viewScale, (fieldDim.field_width + fieldDim.boundary_width) / height());

    viewXOffset = viewYOffset = 0.0;
    recomputeProjection();
    redraw();
}

void SoccerView::resizeEvent(QResizeEvent* event)
{
    QGLWidget::resizeEvent(event);
    redraw();

    resetView();
}

void SoccerView::recomputeProjection()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5*viewScale*width()+viewXOffset, 0.5*viewScale*width()+viewXOffset, -0.5*viewScale*height()+viewYOffset, 0.5*viewScale*height()+viewYOffset, minZValue, maxZValue);
    glMatrixMode(GL_MODELVIEW);
}

void SoccerView::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
    recomputeProjection();
}

void SoccerView::initializeGL()
{
    blueRobotShape = glGenLists(1);
    if(blueRobotShape==GL_INVALID_VALUE) {
        printf("Unable to create display list!\n");
        exit(1);
    }
    glNewList(blueRobotShape, GL_COMPILE);
    drawRobot(teamBlue,true,false);
    glEndList();

    yellowRobotShape = glGenLists(1);
    if(yellowRobotShape==GL_INVALID_VALUE) {
        printf("Unable to create display list!\n");
        exit(1);
    }
    glNewList(yellowRobotShape, GL_COMPILE);
    drawRobot(teamYellow,true,false);
    glEndList();

    greyRobotShape = glGenLists(1);
    if(greyRobotShape==GL_INVALID_VALUE) {
        printf("Unable to create display list!\n");
        exit(1);
    }
    glNewList(greyRobotShape, GL_COMPILE);
    drawRobot(teamUnknown,true,false);
    glEndList();

    blueCircleRobotShape = glGenLists(1);
    if(blueRobotShape==GL_INVALID_VALUE) {
        printf("Unable to create display list!\n");
        exit(1);
    }
    glNewList(blueCircleRobotShape, GL_COMPILE);
    drawRobot(teamBlue,false,false);
    glEndList();

    yellowCircleRobotShape = glGenLists(1);
    if(yellowRobotShape==GL_INVALID_VALUE) {
        printf("Unable to create display list!\n");
        exit(1);
    }
    glNewList(yellowCircleRobotShape, GL_COMPILE);
    drawRobot(teamYellow,false,false);
    glEndList();

    greyCircleRobotShape = glGenLists(1);
    if(greyRobotShape==GL_INVALID_VALUE) {
        printf("Unable to create display list!\n");
        exit(1);
    }
    glNewList(greyCircleRobotShape, GL_COMPILE);
    drawRobot(teamUnknown,false,false);
    glEndList();
}


void SoccerView::vectorTextTest()
{
    #define TextTest(loc,angle,size,str,halign,valign) \
    {glText.drawString((loc),angle,size,str,halign,valign); \
    vector2d l1,l2; \
    l1.heading(M_PI/180.0*angle); \
    l1 = loc+l1*size*(glText.getWidth(str)); \
    l2.heading(M_PI/180.0*angle); \
    l2 = loc-l2*size*(glText.getWidth(str)); \
    glBegin(GL_LINES); \
    glVertex3d(l1.x,l1.y,9); \
    glVertex3d(l2.x,l2.y,9); \
    glEnd();}

    glColor3d(1,1,1);
    TextTest(vector2d(1,1)*353.6,45,500,"123agdo0",GLText::LeftAligned,GLText::MedianAligned)
    TextTest(vector2d(fieldDim.field_length*0.5,0),0,500,"123agdo0",GLText::RightAligned,GLText::BottomAligned)
    TextTest(vector2d(0,-fieldDim.field_width*0.5),0,500,"123agdo0",GLText::CenterAligned,GLText::TopAligned)
    TextTest(vector2d(-fieldDim.field_length*0.5,0),0,500,"1\ub023agdo0",GLText::CenterAligned,GLText::MiddleAligned)
}

void SoccerView::paintEvent(QPaintEvent* event)
{
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
    drawFieldLines(fieldDim);
    drawRobots();
    drawBalls();
    //vectorTextTest();
    glPopMatrix();
    swapBuffers();
    graphicsMutex.unlock();
}

void SoccerView::drawQuad(vector2d loc1, vector2d loc2, double z)
{
    glBegin(GL_QUADS);
    glVertex3d(loc1.x,loc1.y,z);
    glVertex3d(loc2.x,loc1.y,z);
    glVertex3d(loc2.x,loc2.y,z);
    glVertex3d(loc1.x,loc2.y,z);
    glEnd();
}

void SoccerView::drawQuad(vector2d v1, vector2d v2, vector2d v3, vector2d v4, double z)
{
    glBegin(GL_QUADS);
    glVertex3d(v1.x,v1.y,z);
    glVertex3d(v2.x,v2.y,z);
    glVertex3d(v3.x,v3.y,z);
    glVertex3d(v4.x,v4.y,z);
    glEnd();
}

void SoccerView::drawArc(vector2d loc, double r1, double r2, double theta1, double theta2, double z, double dTheta)
{
    static const double tesselation = 1.0;
    if(dTheta<0) {
        dTheta = tesselation/r2;
    }
    glBegin(GL_QUAD_STRIP);
    for(double theta=theta1; theta<theta2; theta+=dTheta) {
        double c1 = cos(theta), s1 = sin(theta);
        glVertex3d(r2*c1+loc.x,r2*s1+loc.y,z);
        glVertex3d(r1*c1+loc.x,r1*s1+loc.y,z);
    }
    double c1 = cos(theta2), s1 = sin(theta2);
    glVertex3d(r2*c1+loc.x,r2*s1+loc.y,z);
    glVertex3d(r1*c1+loc.x,r1*s1+loc.y,z);
    glEnd();
}

void SoccerView::drawTriangle(vector2d v1, vector2d v2, vector2d v3, double z) {
    glBegin(GL_TRIANGLES);
    glVertex3d(v1.x, v1.y, z);
    glVertex3d(v2.x, v2.y, z);
    glVertex3d(v3.x, v3.y, z);
    glEnd();
}

void SoccerView::drawRobot(int team, bool hasAngle, bool useDisplayLists)
{
  if(useDisplayLists) {
    switch (team) {
      case teamBlue: {
        if(hasAngle)
          glCallList(blueRobotShape);
        else
          glCallList(blueCircleRobotShape);
        break;
      }
      case teamYellow: {
        if(hasAngle)
          glCallList(yellowRobotShape);
        else
          glCallList(yellowCircleRobotShape);
        break;
      }
      default: {
        if(hasAngle)
          glCallList(greyRobotShape);
        else
          glCallList(greyCircleRobotShape);
        break;
      }
    }
    return;
  }

  switch (team) {
    case teamBlue: {
      glColor3d(0.2549, 0.4941, 1.0);
      break;
    }
    case teamYellow: {
      glColor3d(1.0, 0.9529, 0.2431);
      break;
    }
    default: {
      glColor3d(0.5882,0.5882,0.5882);
      break;
    }
  }
  double theta1 = hasAngle?RAD(40):0.0;
  double theta2 = 2.0*M_PI - theta1;
  drawQuad(-35,35,35,-35, RobotZ);

  switch ( team ){
    case teamBlue:{
      glColor3d(0.0706, 0.2314, 0.6275);
      break;
    }
    case teamYellow:{
      glColor3d(0.8, 0.6157, 0.0);
      break;
    }
    default:{
      glColor3d(0.2745,0.2745,0.2745);
      break;
    }
  }

  drawQuad(-40,40,40,35,RobotZ+0.01);
  drawQuad(-40,-40,40,-35,RobotZ+0.01);
  drawQuad(40,-40,35,40,RobotZ+0.01);
  drawQuad(-40,-40,-35,40,RobotZ+0.01);

  if(hasAngle){
      glColor3d(0.5, 0.5, 0.5);

      drawQuad(-20,47,20,40,RobotZ+0.02);
      drawQuad(-20,-47,20,-40,RobotZ+0.02);
  }
}

void SoccerView::drawRobot(vector2d loc, double theta, double conf, int robotID, int team, bool hasAngle)
{
  glPushMatrix();
  glLoadIdentity();
  glTranslated(loc.x,loc.y,0);

  switch ( team ){
    case teamBlue:{
      glColor3d(0.2549, 0.4941, 1.0);
      break;
    }
    case teamYellow:{
      glColor3d(1.0, 0.9529, 0.2431);
      break;
    }
    default:{
      glColor3d(0.5882,0.5882,0.5882);
      break;
    }
  }


  glColor3d(0.0,0.0,0.0);
  char buf[1024];
  if(robotID!=unknownRobotID)
    snprintf(buf,1023,"%d",robotID);
  else
    snprintf(buf,1023,"?");
    glText.drawString(loc,0,50,buf,GLText::CenterAligned,GLText::MiddleAligned);
  switch ( team ){
    case teamBlue:{
      glColor3d(0.0706, 0.2314, 0.6275);
      break;
    }
    case teamYellow:{
      glColor3d(0.8, 0.6157, 0.0);
      break;
    }
    default:{
      glColor3d(0.2745,0.2745,0.2745);
      break;
    }
  }

  glRotated(theta,0,0,1.0);
  drawRobot(team, hasAngle, true);
  glPopMatrix();
}

void SoccerView::drawFieldLines(FieldDimensions& dimensions)
{
    glColor4f(FIELD_LINES_COLOR);
    for (size_t i = 0; i < fieldDim.lines.size(); ++i) {
        const FieldLine& line = *fieldDim.lines[i];
        const double half_thickness = 0.5 * line.thickness;
        const vector2d p1(line.p1_x, line.p1_y);
        const vector2d p2(line.p2_x, line.p2_y);
        const vector2d perp = (p2 - p1).norm().perp();
        //    const vector2d corner1 = p1 - half_thickness * perp;
        //    const vector2d corner2 = p2 + half_thickness * perp;
        //    drawQuad(corner1, corner2, FieldZ);
        const vector2d v1 = p1 - half_thickness * perp;
        const vector2d v2 = p1 + half_thickness * perp;
        const vector2d v3 = p2 + half_thickness * perp;
        const vector2d v4 = p2 - half_thickness * perp;
        drawQuad(v1, v2, v3, v4, FieldZ);
    }


    if(!leftIsBlue) glColor3d(1.0, 0.9529, 0.2431);
    else            glColor3d(0.2549, 0.4941, 1.0);
    for (size_t i = 0; i < fieldDim.leftGoalLines.size(); ++i) {
        const FieldLine& line = *fieldDim.leftGoalLines[i];

        const double half_thickness = 0.5 * line.thickness;
        const vector2d p1(line.p1_x, line.p1_y);
        const vector2d p2(line.p2_x, line.p2_y);
        const vector2d perp = (p2 - p1).norm().perp();
        //    const vector2d corner1 = p1 - half_thickness * perp;
        //    const vector2d corner2 = p2 + half_thickness * perp;
        //    drawQuad(corner1, corner2, FieldZ);
        const vector2d v1 = p1 - half_thickness * perp;
        const vector2d v2 = p1 + half_thickness * perp;
        const vector2d v3 = p2 + half_thickness * perp;
        const vector2d v4 = p2 - half_thickness * perp;
        drawQuad(v1, v2, v3, v4, FieldZ);
    }

    if(leftIsBlue) glColor3d(1.0, 0.9529, 0.2431);
    else           glColor3d(0.2549, 0.4941, 1.0);
    for (size_t i = 0; i < fieldDim.rightGoalLines.size(); ++i) {
        const FieldLine& line = *fieldDim.rightGoalLines[i];

        const double half_thickness = 0.5 * line.thickness;
        const vector2d p1(line.p1_x, line.p1_y);
        const vector2d p2(line.p2_x, line.p2_y);
        const vector2d perp = (p2 - p1).norm().perp();
        //    const vector2d corner1 = p1 - half_thickness * perp;
        //    const vector2d corner2 = p2 + half_thickness * perp;
        //    drawQuad(corner1, corner2, FieldZ);
        const vector2d v1 = p1 - half_thickness * perp;
        const vector2d v2 = p1 + half_thickness * perp;
        const vector2d v3 = p2 + half_thickness * perp;
        const vector2d v4 = p2 - half_thickness * perp;
        drawQuad(v1, v2, v3, v4, FieldZ);
    }

    glColor4f(FIELD_LINES_COLOR);
    for (size_t i = 0; i < fieldDim.arcs.size(); ++i) {
        const FieldCircularArc& arc = *fieldDim.arcs[i];
        const double half_thickness = 0.5 * arc.thickness;
        const double radius = arc.radius;
        const vector2d center(arc.center_x, arc.center_y);
        const double a1 = arc.a1;
        const double a2 = arc.a2;
        drawArc(center, radius - half_thickness, radius + half_thickness, a1, a2, FieldZ);
    }

    glColor4f(FIELD_LINES_COLOR);
    for(size_t i=0; i<fieldDim.triangles.size(); i++) {
        const FieldTriangle& triangle = *fieldDim.triangles[i];
        const vector2d v1(triangle.p1_x, triangle.p1_y);
        const vector2d v2(triangle.p2_x, triangle.p2_y);
        const vector2d v3(triangle.p3_x, triangle.p3_y);
        drawTriangle(v1, v2, v3, FieldZ);
    }
}

void SoccerView::drawBall(vector2d loc) {
    glColor3d(1.0,0.5059,0.0);
    drawArc(loc,0,16,-M_PI,M_PI,BallZ);
    glColor3d(0.8706,0.3490,0.0);
    drawArc(loc,15,21,-M_PI,M_PI,BallZ);
}

void SoccerView::drawVector(vector2d v1, vector2d v2, double z) {
    glColor3d(1.0, 0.0, 0.0);

    const vector2d norm = (v2 - v1).norm();
    const vector2d perp = (v2 - v1).norm().perp();

    // Line
    const double quad_half_thickness = 0.5 * 10;
    const vector2d qv1 = v1 - quad_half_thickness * perp;
    const vector2d qv2 = v1 + quad_half_thickness * perp;
    const vector2d qv3 = v2 + quad_half_thickness * perp;
    const vector2d qv4 = v2 - quad_half_thickness * perp;
    drawQuad(qv1, qv2, qv3, qv4, z);

    // Triangle
    const double triangle_thickness = 50;
    const vector2d tv1 = v2 + 2*triangle_thickness*norm;
    const vector2d tv2 = v2 + triangle_thickness*perp;
    const vector2d tv3 = v2 - triangle_thickness*perp;
    drawTriangle(tv1, tv2, tv3, z);
}

void SoccerView::drawBalls() {
    drawBall(ball);
}

void SoccerView::drawRobots() {
    if(robots.empty())
        return;

    for(int j=0; j<robots.size(); j++){
      SoccerViewRobot r = robots.at(j);
      drawRobot(r.loc, r.angle, 100.0, r.id, r.team, r.hasAngle);
    }
}

void SoccerView::updateDetection(fira_message::Frame frame) {
    graphicsMutex.lock();

    robots.clear();
    ballVelocity.set(0,0);

    vector2d velocity;
    SoccerViewRobot robot;

    for(int x = 0; x < frame.robots_blue().size(); x++){
        robot.loc.set(frame.robots_blue(x).x() * 1000.0, frame.robots_blue(x).y() * 1000.0);
        robot.id = frame.robots_blue(x).robot_id();
        robot.hasAngle = true;
        robot.angle = frame.robots_blue(x).orientation() * 180.0/M_PI;
        robot.team = teamBlue;
        robots.append(robot);
    }

    for(int x = 0; x < frame.robots_yellow().size(); x++){
        robot.loc.set(frame.robots_yellow(x).x() * 1000.0, frame.robots_yellow(x).y() * 1000.0);
        robot.id = frame.robots_yellow(x).robot_id();
        robot.hasAngle = true;
        robot.angle = frame.robots_yellow(x).orientation() * 180.0/M_PI;
        robot.team = teamYellow;
        robots.append(robot);
    }

    ball.set(frame.ball().x() * 1000.0, frame.ball().y() * 1000.0);

    graphicsMutex.unlock();
    emit postRedraw();
}

