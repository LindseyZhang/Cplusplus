#ifndef HELP_H
#define HELP_H

#include <QWidget>
#include "mainwindow.h"

namespace Ui {
class help;
}

class help : public QWidget
{
    Q_OBJECT

public:
    explicit help(QWidget *parent = 0);
    explicit help(MainWindow *main,QWidget *parent = 0);
    ~help();

private slots:
    void on_pushButton_clicked();

private:
    Ui::help *ui;
    MainWindow *main;
};

#endif // HELP_H
