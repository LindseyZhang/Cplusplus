#include "askname.h"
#include "ui_askname.h"
#include "commonvalue.h"
#include <iostream>
#include <fstream>
using namespace std;

askName::askName(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::askName)
{
    ui->setupUi(this);
    loadPicture();
}

askName::askName(int step, int time,QWidget *parent):
                 QDialog(parent),
                 ui(new Ui::askName)
{
    ui->setupUi(this);
    this->step=step;
    timeUsed=TIMEPERGAME-time;
    loadPicture();
}

void askName::loadPicture(){
    QPixmap fire=QPixmap("F:\\QtProgram\\myzork\\fire.jpg");
    ui->lblfire->setPixmap(fire);
}

askName::~askName()
{
    delete ui;
}

void askName::on_btnYes_clicked()
{
  name=ui->nameInput->text().toStdString();
  ofstream outfile("F:\\QtProgram\\myzork\\history.txt",ios::app);
  if(outfile.is_open()){
      outfile<<timeUsed<<" "<<step<<" "<<name<<endl;
      outfile.close();
  }
    this->close();
}

void askName::on_btnNo_clicked()
{
    this->close();
}

