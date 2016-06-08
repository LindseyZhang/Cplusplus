#ifndef ITEM_H_
#define ITEM_H_

#include <map>
#include <string>
#include <iostream>
using namespace std;

class Item {
private:
    string description;
    string longDescription;
    int weightGrams;
    float value;

public:
    Item (string description, int inWeight, float inValue);
    Item (string inDescription);
    string getShortDescription() const;
    int getWeight();
    void setWeight(int weightGrams);
    float getValue();
    void setValue(float value);
    int getWeaponCheck();
    void setWeaponCheck(int weaponCheck);
    bool operator==(const Item &);
};

#endif /*ITEM_H_*/

