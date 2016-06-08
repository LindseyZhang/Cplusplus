#include "player.h"

Player::Player()
{

}

Player::~Player()
{

}
QList <Item> Player::getItems(){
    return itemIhave;
}

void Player::clearItemlist(){
    itemIhave.clear();
}

Item Player::putItem(int index){
  //  if(index<itemIhave.size()){
        return itemIhave.takeAt(index);
  //  }else
   //     return NULL;
}

//http://bytes.com/topic/c/answers/507578-adding-local-variable-struct-container-problem
void Player::takeItem(Item Item){
    itemIhave.append(Item);
}
int Player::getItemNumber(){
    return itemIhave.size();
}
