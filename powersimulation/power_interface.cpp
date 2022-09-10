#include "power_interface.h"
#include<string>
#include<stdio.h>
using namespace std;
PowerInterface::PowerInterface(){
    
}
PowerInterface::PowerInterface(string nodeId, int index){
    this->nodeId=nodeId;
    this->index=index;
    char indexStr[5];
    sprintf(indexStr,"%d",index);
    string indexS(indexStr);
    id=nodeId+indexS;
    color="white";
    is_boundary_=false;
}
string PowerInterface::getColor(){
    return color;
}
void PowerInterface::setColor(string color){
    this->color=color;
}
string PowerInterface::getNodeId(){
    return nodeId;
}
int PowerInterface::getIndex(){
    return index;
}
string PowerInterface::getId(){
    return id;
}
bool PowerInterface::operator ==(PowerInterface& other){
    string otherId=other.getNodeId();
    int otherIndex=other.getIndex();
    if(nodeId.compare(otherId)==0 && index==otherIndex){
        return true;
    }
    return false;
}
