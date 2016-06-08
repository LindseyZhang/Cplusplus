#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <qstringlistmodel.h>
#include <QString.h>
#include <String>
#include <vector>
#include <QTimer>
#include <list>
#include "room.h"
#include "player.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void startGame();
    void stopGame();

    //roomNumber is non changeable in every window.
    //use static to make sure the variable is created before the main function
    const static int roomNumber=16;


protected:
    void createRoom();
    void setRoomColor();
    void matchWithUi();
    void setGoal(int);
    void keyPressEvent(QKeyEvent *);
    void showInfo();
    void readFromMap(const char *);
    void resetGame();

    // in this function, pass the vector variable by reference,pass by reference can avoid copy all the value in vector
    //it save time and space
    void setRoomInfo(int roomNum,vector<string> & vectors);

private slots:
    void on_listView_doubleClicked(const QModelIndex &index);

    void on_listView_2_doubleClicked(const QModelIndex &index);

    void on_actionLoad_map_1_triggered();

    void on_actionLoad_map_2_triggered();

    void timeDecrease();

    void on_actionNew_game_triggered();

    void on_actionHistory_triggered();

    void on_actionHelp_triggered();

    void on_actionPause_triggered();

    void on_actionRestart_triggered();

private:
    Ui::MainWindow *ui;
    Room *currentRoom;
    Room *roomList[roomNumber];
    QLabel *labelList[roomNumber];
    Player *player;
    QStringListModel *roomItemModel,*playerItemModel;
    QStringList roomItemList,playerItemList;
    int step;
    int goal;
    int time;
    int totalItem;
    QTimer *timer;
    int currentmap;
    bool ingame;

};

#endif // MAINWINDOW_H
