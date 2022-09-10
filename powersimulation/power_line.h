/* 
 * File:   power_line.h
 * Author: stan
 *
 * Created on January 8, 2013, 8:50 AM
 */

#ifndef POWER_LINE_H
#define	POWER_LINE_H

using namespace std;
struct PowerLine{
    string id;
    string lpeer; //node id
    string rpeer;
    int lpeerIf; //index
    int rpeerIf;
    double current;
    double resistor;
    int direction; // line direction is from left to right. If direction==1, current
                   // is the same direction as line. If ==-1, the opposite direction.
    string lpeerPI;
    string rpeerPI;
    bool isPeer(string powerInterface){
        if(powerInterface.compare(lpeerPI)==0 || powerInterface.compare(rpeerPI)==0)
            return true;
        else {
            return false;
        }
    }
    string otherPeer(string powerInterface){
        string result;
        if(isPeer(powerInterface)==true){
            if(powerInterface.compare(lpeerPI)==0)
                return rpeerPI;
            else
                return lpeerPI;
        }
        else{
            return result;
        }
    }
    
};


#endif	/* POWER_LINE_H */

