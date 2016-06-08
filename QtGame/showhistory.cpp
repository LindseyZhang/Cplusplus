#include "showhistory.h"
#include "ui_showhistory.h"
#include "historyinfo.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <QTableWidgetItem>
#include <QStringList>
#include <QTableWidget>
using namespace std;

showHistory::showHistory( QWidget *parent) :
    QWidget(parent),
    ui(new Ui::showHistory)
{
    ui->setupUi(this);
    readHistory();
    sort();
    setAttribute();
}

showHistory::~showHistory()
{
    delete ui;
}
void showHistory::readHistory(){
    ifstream infile;
    string line;
    infile.open("F:\\QtProgram\\myzork\\history.txt");
    while(getline(infile,line)){
     istringstream l(line);
     string name;
     int time,step;
     historyInfo hist;
     l>>time>>step;
     getline(l,name);  //There is an overloaded std::getline function in which a third parameter
                        //takes a delimiter upto which you can read the string
     hist.setName(name);
     hist.setStep(step);
     hist.setTimeUsed(time);
     history.push_back(hist);
    }
    infile.close();
}

//this is the sort method , I try to use pointer to do the sort.
void showHistory::sort(){
    int size=history.size();
    historyInfo *historyPtrList[size];
    for(int i=0;i<size;i++){
        historyPtrList[i]=&history.at(i);
    }
    for(int i=0;i<size;i++){
        for(int j=0;j<size-i-1;j++){
            if(*historyPtrList[j]> *historyPtrList[j+1]){
                historyInfo *temp;
                temp=historyPtrList[j];
                historyPtrList[j]=historyPtrList[j+1];
                historyPtrList[j+1]=temp;
            }
        }
    }
    showData(historyPtrList,size);

}
void showHistory::showData(historyInfo *historyPtrList[],int size){
        int row=10>size?size:10;
    ui->tbscore->setRowCount(row);
    ui->tbscore->setColumnCount(2);
    QStringList header;
   // header<<"Name"<<"Step"<<"Time Used"<<"score";
    header<<"Name"<<"score";
    ui->tbscore->setHorizontalHeaderLabels(header);
    for(int i=0;i<10&&i<size;i++){
        historyInfo *item=historyPtrList[i];
        ui->tbscore->setItem(i,0,new QTableWidgetItem(QString::fromStdString(item->getName())));
      //  ui->tbscore->setItem(i,1,new QTableWidgetItem(QString::number(item->getStep())));
      //  ui->tbscore->setItem(i,2,new QTableWidgetItem(QString::number(item->getTimeUsed())));
        ui->tbscore->setItem(i,1,new QTableWidgetItem(QString::number(item->getScore())));
    }

}
void showHistory::setAttribute(){
    ui->tbscore->setEditTriggers(QTableWidget::NoEditTriggers);
    ui->tbscore->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void showHistory::on_pushButton_clicked()
{
    this->close();
}
