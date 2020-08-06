#include "refereeview.h"

MainWindow* RefereeView::_refereeUI = NULL;

QString RefereeView::name(){
    return "Referee View";
}

RefereeView::RefereeView()
{
    _refereeUI = new MainWindow(nullptr);
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
