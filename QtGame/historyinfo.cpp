#include "historyinfo.h"
#include "commonvalue.h"

historyInfo::historyInfo()
{

}
historyInfo::historyInfo(string name,int step,int timeused){
    this->name=name;
    this->step=step;
    this->timeUsed=timeused;
}

historyInfo::~historyInfo()
{

}

string historyInfo::getName() const{
    return name;
}
int historyInfo::getStep()const{
    return step;
}
int historyInfo::getTimeUsed() const{
    return timeUsed;
}
void historyInfo::setName(string name){
    this->name=name;
}

void historyInfo::setStep(int step){
    this->step=step;
}

void historyInfo::setTimeUsed(int timeused){
    this->timeUsed=timeused;
}

int historyInfo::getScore() const{
    return getStep()*((float)(TIMEPERGAME-getTimeUsed())/TIMEPERGAME+1);
}
