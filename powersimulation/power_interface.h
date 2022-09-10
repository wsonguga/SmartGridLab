/* 
 * File:   power_interface.h
 * Author: stan
 *
 * Created on January 8, 2013, 8:49 AM
 */

#ifndef POWER_INTERFACE_H
#define	POWER_INTERFACE_H
#include<string>
using namespace std;
class PowerInterface {
private:
    string nodeId;
    int index;
    string id;
    string color;
    
public:
    bool is_boundary_;
    PowerInterface();
    PowerInterface(string nodeId, int index);
    string getNodeId();
    int getIndex();
    string getId();
    string getColor();
    void setColor(string color);
    bool operator== (PowerInterface& other);
};


#endif	/* POWER_INTERFACE_H */

