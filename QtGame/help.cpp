#include "help.h"
#include "ui_help.h"
#include "mainwindow.h"

help::help(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::help)
{
    ui->setupUi(this);
}
help::help(MainWindow *main,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::help)
{
    ui->setupUi(this);
    this->main=main;
}

help::~help()
{
    delete ui;
}

void help::on_pushButton_clicked()
{
    this->close();
    main->startGame();
}
