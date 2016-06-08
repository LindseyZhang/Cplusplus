#ifndef ASKNAME_H
#define ASKNAME_H

#include <QDialog>
#include <string>
using namespace std;

namespace Ui {
class askName;
}

class askName : public QDialog
{
    Q_OBJECT

public:
    explicit askName(QWidget *parent = 0);
    askName(int ,int,QWidget *parent = 0);
    void loadPicture();
    ~askName();

private slots:
    void on_btnYes_clicked();

    void on_btnNo_clicked();

private:
    Ui::askName *ui;
     string name;
     int step;
     int timeUsed;
};

#endif // ASKNAME_H
