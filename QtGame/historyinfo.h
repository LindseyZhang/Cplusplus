#ifndef HISTORYINFO_H
#define HISTORYINFO_H


#include <string>
using namespace std;

class historyInfo
{
public:
    historyInfo();
    historyInfo(string name,int step,int timeused);
    ~historyInfo();
    string getName() const;
    int getStep()const;
    int getTimeUsed() const;
    void setName(string);
    void setStep(int);
    void setTimeUsed(int);
    inline bool operator > ( const historyInfo &);
    inline bool operator < ( const historyInfo &);
    int getScore() const;
private:
    string name;
    int step;
    int timeUsed;
};

inline bool historyInfo::operator > (const historyInfo & other){
    //have to make getScore() const, const object can only call const function
    if(getScore()>other.getScore()){
        return true;
    }else{
        return false;
    }
}

inline bool historyInfo::operator < (const historyInfo & other){

    return !(*this>other);
}

#endif // HISTORYINFO_H
