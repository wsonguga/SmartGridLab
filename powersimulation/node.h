/* 
 * File:   node.h
 * Author: stan
 *
 * Created on January 8, 2013, 8:47 AM
 */

#ifndef NODE_H
#define	NODE_H
#include<stdio.h>
#include<stdlib.h>    
using namespace std;
class Node {
public:
    string id;
    string role;
    double current;
    double desired_current;
    double max_current;
    int power_interface_flag;
    string location;
    int power_interface_num;
    vector<string>power_interfaces;
    //storage specific fields:
    double e_stored;
    double capacity;
    string operation_mode;
    Node(){
        e_stored=0;
        capacity=0;
        operation_mode="";
        current=0;
        max_current=0;
        desired_current=0;
    }
    //return the power interface peer list within a node
    void pi_neighbors(string power_interface,vector<string>& neighbors){
        if(power_interface.substr(0,power_interface.size()-1).compare(id)!=0){
            cout<<"Wrong node to be searched!"<<endl;
            return; 
        }
        int index=atoi(power_interface.substr(power_interface.size()-1,1).c_str());
        if(isUsed(index)==false){
            cout<<"The power interface is not used!"<<endl;
            return;
        }
    
        for(int i=0;i<power_interfaces.size();i++){
         
            if(power_interfaces[i].compare(power_interface)!=0){
              
                int index1=pi_to_index(power_interface);
                int index2=pi_to_index(power_interfaces[i]);
                if(isConnected(index1,index2)){
                    string temp=index_to_pi(index2);
                   
                    neighbors.push_back(temp);
                }
                
            }
        }
        
        
        
    }
    //whether the index power interface is used.
private:
    bool isUsed (int index){
          char indexStr[5];
          sprintf(indexStr,"%d",index);
          string indexS(indexStr);
          string pi_id=id+indexS;
          for(int i=0;i<power_interfaces.size();i++){
              if(pi_id.compare(power_interfaces[i])==0)
                  return true;
          }
          return false;
    }
    int pi_to_index(string power_interface){
        return atoi(power_interface.substr(power_interface.size()-1,1).c_str());
    }
    string index_to_pi(int index){
          char indexStr[5];
          sprintf(indexStr,"%d",index);
          string indexS(indexStr);
          string pi=id+indexS;
          return pi;
    }
    bool isConnected(int index1, int index2){
        if(index1>index2){
            int temp;
            temp=index1;
            index1=index2;
            index2=temp;
        }
        if(index1==1 && index2==2){
            int temp=power_interface_flag;
            if(temp%2==1)
                return true;
        }
        if(index1==1 && index2==3){
            int temp=power_interface_flag;
            temp=temp>>1;
            if(temp%2==1)
                return true;
        }
          if(index1==1 && index2==4){
            int temp=power_interface_flag;
            temp=temp>>2;
            if(temp%2==1)
                return true;
        }
          if(index1==1 && index2==5){
            int temp=power_interface_flag;
            temp=temp>>3;
            if(temp%2==1)
                return true;
        }
        if(index1==1 && index2==6){
            int temp=power_interface_flag;
            temp=temp>>4;
            if(temp%2==1)
                return true;
        }
        if(index1==2 && index2==3){
            int temp=power_interface_flag;
            temp=temp>>5;
            if(temp%2==1)
                return true;
        }
        if(index1==2 && index2==4){
            int temp=power_interface_flag;
            temp=temp>>6;
            if(temp%2==1)
                return true;
        }
        if(index1==2 && index2==5){
            int temp=power_interface_flag;
            temp=temp>>7;
            
            if(temp%2==1)
                return true;
        }
        if(index1==2 && index2==6){
            int temp=power_interface_flag;
            temp=temp>>8;
            if(temp%2==1)
                return true;
        }
        if(index1==3 && index2==4){
            int temp=power_interface_flag;
            temp=temp>>9;
            if(temp%2==1)
                return true;
        }
        if(index1==3 && index2==5){
            int temp=power_interface_flag;
            temp=temp>>10;
            if(temp%2==1)
                return true;
        }
        if(index1==3 && index2==6){
            int temp=power_interface_flag;
            temp=temp>>11;
            if(temp%2==1)
                return true;
        }
        if(index1==4 && index2==5){
            int temp=power_interface_flag;
            temp=temp>>12;
            if(temp%2==1)
                return true;
        }
        if(index1==4 && index2==6){
            int temp=power_interface_flag;
            temp=temp>>13;
            if(temp%2==1)
                return true;
        }
        if(index1==5 && index2==6){
            int temp=power_interface_flag;
            temp=temp>>14;
            if(temp%2==1)
                return true;
        }
        return false;
    }
};


#endif	/* NODE_H */

