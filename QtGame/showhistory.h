#ifndef SHOWHISTORY_H
#define SHOWHISTORY_H

#include <QWidget>
#include "historyinfo.h"
#include <vector>
#include "mainwindow.h"


namespace Ui {
class showHistory;
}

class showHistory : public QWidget
{
    Q_OBJECT

public:
    explicit showHistory(QWidget *parent = 0);
    ~showHistory();
    void readHistory();
    void sort();
    void showData(historyInfo* [],int size);
    void setAttribute();
private slots:
    void on_pushButton_clicked();

private:
    Ui::showHistory *ui;
    vector<historyInfo> history;
};

#endif // SHOWHISTORY_H
