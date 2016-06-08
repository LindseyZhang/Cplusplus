#include "Room.h"

Room::Room(string description) {
    this->description = description;
    exits["north"] = NULL;
    exits["east"] = NULL;
    exits["south"] = NULL;
    exits["west"] = NULL;
}

//instead write the exactly same code
// make a constructor call another constructor
Room::Room(){
    Room("");
}
map<string, Room*> Room::getExits(){
    return exits;
}

void Room::setDescription(string des){
    if(!des.empty()){
    this->description=des;
    }
}


//the two function below, I use the function overload
// so they have the same functin name but different parameter.
void Room::setExits(Room *north, Room *east, Room *south, Room *west) {
  //  if (north != NULL)
        exits["north"] = north;
  //  if (east != NULL)
        exits["east"] = east;
  //  if (south != NULL)
        exits["south"] = south;
  //  if (west != NULL)
        exits["west"] = west;
}

void Room::setExits(string direction,Room * room){
    exits[direction]=room;
}

string Room::getDescription() {
    return description;
}


Room* Room::nextRoom(string direction) {
    map<string, Room*>::iterator next = exits.find(direction); //returns an iterator for the "pair"
    if (next == exits.end())
        return NULL; // if exits.end() was returned, there's no room in that direction.
    return next->second; // If there is a room, remove the "second" (Room*)
                // part of the "pair" (<string, Room*>) and return it.
}


int Room::numberOfItems() {
    return itemsInRoom.size();
}

int Room::isItemInRoom(string inString)
{
    int sizeItems = (itemsInRoom.size());
    if (itemsInRoom.size() < 1) {
        return false;
        }
    else if (itemsInRoom.size() > 0) {
       int x = (0);
        for (int n = sizeItems; n > 0; n--) {
            // compare inString with short description
            int tempFlag = inString.compare( itemsInRoom[x].getShortDescription());
            if (tempFlag == 0) {
                itemsInRoom.erase(itemsInRoom.begin()+x);
                return x;
            }
            x++;
            }
        }
    return -1;
}
QList<Item> Room::getItems(){
    return itemsInRoom;
}
void Room::addItem(Item Item){
    itemsInRoom.append(Item);
}
void Room::addItem(Item *Item){
    addItem(*Item);
}

 Item Room::removeItemFromRoom(int index){
        return itemsInRoom.takeAt(index);
 }
  /*  Item Room::removeItemFromRoom(int index){
       if(index<itemsInRoom.size()){
       item=Item(itemsInRoom.at(index).getShortDescription());
       }
       else item=NULL;
}*/


