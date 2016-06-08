#ifndef PLAYER_H
#define PLAYER_H

#include <qlist.h>
#include "item.h"
using namespace std;

class Player
{
public:
    Player();
    ~Player();
    void takeItem(Item Item);
    Item putItem(int index);
    QList <Item> getItems();
    int getItemNumber();
    void clearItemlist();
private:
    QList <Item> itemIhave;
};

#endif // PLAYER_H
