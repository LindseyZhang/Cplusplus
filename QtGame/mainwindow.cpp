#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QKeyEvent>
#include <QTextEdit>
#include <qstringlistmodel.h>
#include <QString.h>
#include <qmessagebox.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <QPixmap>
#include <QTimer>
#include "commonvalue.h"
#include "askname.h"
#include "help.h"
#include "showhistory.h"
#include <algorithm>


using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    player=new Player();
    roomItemModel=new QStringListModel();
    playerItemModel=new QStringListModel();
    timer=new QTimer(this);
    matchWithUi();
    createRoom();

    connect(timer,SIGNAL(timeout()),this,SLOT(timeDecrease()));
    //timer->start(1000);
    resetGame();
    startGame();
    readFromMap("F:\\QtProgram\\myzork\\map1.txt");
    currentmap=1;
    setRoomColor();
    showInfo();
}


MainWindow::~MainWindow()
{
    delete ui;
    delete roomItemModel;
    delete playerItemModel;
    delete player;
    delete timer;
}
void MainWindow::keyPressEvent(QKeyEvent *e){
    Room *room;
    room=NULL;
    if(ingame){
    switch (e->key()) {
    case Qt::Key_Up:
        room=currentRoom->getExits().find("north")->second;
        break;
    case Qt::Key_Down:
          room=currentRoom->getExits().find("south")->second;
        break;
    case Qt::Key_Left:
           room=currentRoom->getExits().find("west")->second;
        break;
    case Qt::Key_Right:
           room=currentRoom->getExits().find("east")->second;
        break;
    default:room=currentRoom;           //double click listView will triger keyPressEvent
        break;
    }
    if(room != NULL){
        if(room!=currentRoom){
        currentRoom=room;
        step++;
        setRoomColor();
        showInfo();
        if(roomList[goal]==currentRoom){

            if(player->getItemNumber()==totalItem){
          //  msgbox.setText("congratulation! you win.");
            stopGame();
            ui->actionPause->setEnabled(false);
            ui->actionRestart->setEnabled(false);
            askName *asknameDialog=new askName(step,time);
            asknameDialog->show();
            }else{

             QMessageBox msgbox;
             msgbox.setText("welcome. \n but you forget bring all the items here.");
             msgbox.exec();
            }

        }
        }
    }else{
        QMessageBox msgbox;
        msgbox.setText("you can't go there.\nyou can only go door or try to find the entry to castle.");
        msgbox.exec();
    }
    }
}

void MainWindow::setGoal(int roomnumber){
    goal=roomnumber;
}

void MainWindow::createRoom(){
    for(int i=0;i<roomNumber;i++){
        roomList[i]=new Room();
    }
    currentRoom=roomList[0];
}

void MainWindow::matchWithUi(){
    labelList[0] = ui->room_1;
    labelList[1] = ui->room_2;
    labelList[2] = ui->room_3;
    labelList[3] = ui->room_4;
    labelList[4] = ui->room_5;
    labelList[5] = ui->room_6;
    labelList[6] = ui->room_7;
    labelList[7] = ui->room_8;
    labelList[8] = ui->room_9;
    labelList[9] = ui->room_10;
    labelList[10] = ui->room_11;
    labelList[11] = ui->room_12;
    labelList[12] = ui->room_13;
    labelList[13] = ui->room_14;
    labelList[14] = ui->room_15;
    labelList[15] = ui->room_16;
}

void MainWindow::setRoomColor(){
    for(int i=0;i<roomNumber;i++){
       // labelList[i]->setStyleSheet("QLabel { background-color : blank; color : blue; }");
       // labelList[i]->setText(QString::fromStdString(roomList[i]->getDescription()));
        QPixmap ground=QPixmap("F:\\QtProgram\\myzork\\ground.jpg");
        labelList[i]->setPixmap(ground);
        if(currentRoom==roomList[i]){
            QPixmap character=QPixmap("F:\\QtProgram\\myzork\\character.jpg");
            labelList[i]->setPixmap(character);
        // labelList[i]->setStyleSheet("QLabel { background-color : green; color : blue; }");
        }
    }
    map<string, Room*> exits=currentRoom->getExits();
    for (map<string, Room*>::iterator j = exits.begin(); j != exits.end(); j++){
        for(int i=0;i<roomNumber;i++){
            if(j->second==roomList[i]){
                QPixmap door=QPixmap("F:\\QtProgram\\myzork\\closeDoor.jpg");
                labelList[i]->setPixmap(door);
               // labelList[i]->setStyleSheet("QLabel { background-color : white; color : blue; }");
                break;
            }
        }
    }

    if(currentRoom==roomList[goal]){
        QPixmap arrive=QPixmap("F:\\QtProgram\\myzork\\arrive.jpg");
        labelList[goal]->setPixmap(arrive);
    }else{
        QPixmap destination=QPixmap("F:\\QtProgram\\myzork\\goal.jpg");
        labelList[goal]->setPixmap(destination);
    }

}

void MainWindow::timeDecrease(){
    if(time>0){
        time--;
        showInfo();
    }else{
        QMessageBox msgbox;
        msgbox.setText("sorry,time out.\n you lose the game.");
        msgbox.exec();
        stopGame();
    }
}

void MainWindow::showInfo(){
    roomItemList.clear();
    for(int i=0;i<currentRoom->getItems().size();i++){
        Item a=currentRoom->getItems().at(i);
         roomItemList<<QString::fromStdString(a.getShortDescription());
    }
     roomItemModel->setStringList(roomItemList);
     ui->listView->setModel(roomItemModel);
    ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->listView->setFocusPolicy(Qt::NoFocus);

    playerItemList.clear();
    for(int i=0;i<player->getItems().size();i++){
        Item a=player->getItems().at(i);
          playerItemList<<QString::fromStdString(a.getShortDescription());
    }
    playerItemModel->setStringList(playerItemList);
    ui->listView_2->setModel(playerItemModel);
    ui->listView_2->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->listView_2->setFocusPolicy(Qt::NoFocus);

    ui->lblStepValue->setText(QString::number(step));
    ui->lblTimeValue->setText(QString::number(time));
}

void MainWindow::resetGame(){
    step=0;
    time=TIMEPERGAME;
    totalItem=0;
  //  timer->start(1000);
    player->clearItemlist();
}

void MainWindow::startGame(){
    timer->start(1000);
    ingame=true;
    ui->actionPause->setEnabled(true);
    ui->actionRestart->setEnabled(false);
}

void MainWindow::stopGame(){
    timer->stop();
    ingame=false;
    ui->actionPause->setEnabled(false);
    ui->actionRestart->setEnabled(true);
}

void MainWindow::on_listView_doubleClicked(const QModelIndex &index)
{
    if(index.isValid()){
        player->takeItem(currentRoom->removeItemFromRoom(index.row()));
         showInfo();
    }

}

void MainWindow::on_listView_2_doubleClicked(const QModelIndex &index)
{
    if(index.isValid()){
        currentRoom->addItem(player->putItem(index.row()));
          showInfo();
    }
}


// In this function, I try to do read the infomation from file.
//the provide file will present in this format
//[room description][north exit number][east exit number][south exit number][west exit number] (items in room)
//things in [] is madatary and in () is optional and can be one or more

//every line is one piece of info about one room
// I read it from file line by line

//and I work on the string, split it by ' ' and put all these part into a vector
void MainWindow::readFromMap(const char * filename){
    ifstream infile;
    string line;
    infile.open(filename);
    resetGame();
    int currentRoomNumber=0;
    while(getline(infile,line)&&currentRoomNumber<roomNumber){
     vector<string> strings;
     istringstream l(line);
     string s;

     while (getline(l, s, ' ')) {
         strings.push_back(s);
     }
    setRoomInfo(currentRoomNumber,strings);
    currentRoomNumber++;
    strings.clear();
    }
    infile.close();
    setRoomColor();
    showInfo();

}


//In this function, I try to use the infomation in vector to specific the room info

void MainWindow::setRoomInfo(int roomNum,vector<string> & vectors){
    //every time before set the room info,clear old data
    roomList[roomNum]->clearRoom();
    roomList[roomNum]->setDescription(vectors.at(0));
    if(vectors.at(0).compare("start")==0){
        currentRoom=roomList[roomNum];
    }else if(vectors.at(0).compare("end")==0){
      goal=roomNum;
    }
    for(int j=1;j<5;j++){
        string n=vectors.at(j);
        string direction;
        switch(j){
        case 1:direction="north";break;
        case 2:direction="east";break;
        case 3:direction="south";break;
        case 4:direction="west";break;
        default:direction="";break;
        }
    if(n.compare("null")!=0){
        stringstream ss(n);
         int a;
         if((ss >> a).fail())
         {
             //ERROR
         }
        if(a<roomNumber)
         roomList[roomNum]->setExits(direction,roomList[a]);
    }
    }
    for(unsigned int i=5;i<vectors.size();i++){
        totalItem++;
        roomList[roomNum]->addItem(new Item(vectors.at(i), 1, 11));
    }
}


void MainWindow::on_actionLoad_map_1_triggered()
{
    readFromMap("F:\\QtProgram\\myzork\\map1.txt");
    currentmap=1;
}

void MainWindow::on_actionLoad_map_2_triggered()
{
    readFromMap("F:\\QtProgram\\myzork\\map2.txt");
    currentmap=2;
}

void MainWindow::on_actionNew_game_triggered()
{
    switch(currentmap){
    case 1:readFromMap("F:\\QtProgram\\myzork\\map1.txt");break;
    case 2:readFromMap("F:\\QtProgram\\myzork\\map2.txt");break;
    default:readFromMap("F:\\QtProgram\\myzork\\map1.txt");break;
    }
    startGame();
}

void MainWindow::on_actionHistory_triggered()
{
    stopGame();
    showHistory *showhistory=new showHistory();
    showhistory->show();
}

void MainWindow::on_actionHelp_triggered()
{
    stopGame();
    help *h=new help(this);
    h->show();
}

void MainWindow::on_actionPause_triggered()
{
    stopGame();

}

void MainWindow::on_actionRestart_triggered()
{
    startGame();
}
