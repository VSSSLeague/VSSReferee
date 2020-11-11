#include "refereeview.h"

MainWindow* RefereeView::_refereeUI = NULL;

QString RefereeView::name(){
    return "Referee View";
}

RefereeView::RefereeView(Constants *constants)
{
    _constants = constants;
    _refereeUI = new MainWindow(nullptr, getConstants());
    _refereeUI->show();
}

RefereeView::~RefereeView(){
    _refereeUI->close();

    delete _refereeUI;
}

void RefereeView::initialization(){
    std::cout << "[Referee View] Thread started. " << std::endl;
}

void RefereeView::loop(){

}

void RefereeView::finalization(){
    std::cout << "[Referee View] Thread ended. " << std::endl;
}

void RefereeView::updateDetection(fira_message::Frame frame){
    _refereeUI->updateDetection(frame);
}

void RefereeView::addRefereeCommand(QString command){
    _refereeUI->addRefereeCommand(command);
}

void RefereeView::setRefereeCommand(QString command){
    _refereeUI->setRefereeCommand(command);
}

void RefereeView::setCurrentTime(int time){
    _refereeUI->setCurrentTime(time);
}

bool RefereeView::getBlueIsLeftSide(){
    return _refereeUI->getBlueIsLeftSide();
}

void RefereeView::drawText(vector2d pos, char *str){
    _refereeUI->drawText(pos, str);
}

void RefereeView::setTeams(QString teamLeft, VSSRef::Color leftColor, QString teamRight, VSSRef::Color rightColor){
    _refereeUI->setTeams(teamLeft, leftColor, teamRight, rightColor);

}

MainWindow* RefereeView::getUI(){
    return _refereeUI;
}

Constants* RefereeView::getConstants(){
    if(_constants == NULL){
        std::cout << "[ERROR] Referee is requesting constants, but it's NULL\n";
        return NULL;
    }

    return _constants;
}
