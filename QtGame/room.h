
#ifndef ROOM_H_
#define ROOM_H_

#include <map>
#include <string>
#include <qlist.h>
#include "item.h"
using namespace std;


class Room {

private:
    string description;
    map<string, Room*> exits;
  //  string exitString();
    QList <Item> itemsInRoom;


public:
    int numberOfItems();
    //constructor overload
    Room(string description);
    Room();
    void setExits(Room *north, Room *east, Room *south, Room *west);
    void setExits(string direction,Room *);
    string getDescription();
    Room* nextRoom(string direction);
    void addItem(Item Item);
    void addItem(Item *Item);
    int isItemInRoom(string inString);
    Item removeItemFromRoom(int index);
    map<string, Room*> getExits();
    QList<Item> getItems();
    void setDescription(string des);
    inline void clearRoom();

};
//cause it is small code but will be called many times, to decrease the time consumeing of calling function
//make this functin inline.
//and put an inline function in a header file to avoid “unresolved external” errors
inline void Room::clearRoom(){
    description="";
    itemsInRoom.clear();
    exits["north"] = NULL;
    exits["east"] = NULL;
    exits["south"] = NULL;
    exits["west"] = NULL;
}

#endif
