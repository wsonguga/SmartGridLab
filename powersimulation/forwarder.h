/* 
 * File:   forwarder.h
 * Author: stan
 *
 * Created on January 15, 2014, 9:02 AM
 */

#ifndef FORWARDER_H
#define	FORWARDER_H
#include <iostream>
#include <map>
#include <vector>
#include "node.h"
#include "power_interface.h"
#include "power_line.h"
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "entity.h"
#define NUMBER_OF_NODES 17 // interface node is special
using namespace std;

class ConnectedComponent;

class forwarder:public entity {
public:
    char *input; 
    int order; // node number
    int size;  // power line number
    int numberOfPI; //power interface number
    map<string,Node> nodes;
    map<string,PowerLine> powerLines;
    map<string,PowerInterface> powerInterfaces;
  
    pthread_t testbed_monitor_thread;
    multimap<string,string> adj_list; //power interface and its adjacent.
    void create_adj_list();
    void dfs_visit(string power_interface);
    vector <vector <string> > connected_components;
    vector <ConnectedComponent*>Cs;
    string localIp;
 
    // release the pre pointer in main problem may cause the mutex to be released permanently.??
    // the mutex is not available anymore??
    pthread_mutex_t *pMtx;
    /*
     * find the connected components in a graph.
     */
    void find_connected_components();
  
    /*
     * update appliance desired current
     */
    void update_appliance_desired_current(string node_id,double request_current);
  
    /*
     * update the appliance current and its connected power line's current.
     */
  
    /*
     * debugging 
     */
    void print_everything();
    
     /*
     * update the system based on the new current value or switch configuration
     */

    forwarder();
    void initialize(char* input);
    void parse();
    void forward();
    void getTestbedStatus(void* p);
    string simulate();
    
    map<string,PowerLine>& getPowerLines(){
    return powerLines;   
    }


    map<string,Node>& getNodes(){
         return nodes;
    }
    map<string,PowerInterface>& getPowerInterfaces(){
          return powerInterfaces;
      }
    multimap<string,string>& get_adj_list(){
          return adj_list;
    }
  
    string update(string data);
    
    string find_power_line(string peer1, string peer2);
    bool update_appliance(string node_id,double new_current);
 
    /*
     * return power_line id based on appliance node id.
     */
    string find_app_power_line(string app_node_id);
        /*
     * return the component index based on the power interface id.
     */
    int find_component(string power_interface);
    
    ~forwarder(){
          connected_components.clear();
         adj_list.clear();
       
       pthread_cancel(testbed_monitor_thread);   
    }
    
    //functions inherited but not implemented and used.
    
    string find_power_line_resistor(string node1, string node2){
        
    }
    
    string find_neighbors(string node){
        
    }
    
     void set_out_file(string ){
         
     };
    /*
     * get the output file path.
     */
    string get_out_file(){
        
    };
    void ancillary_threads_start(){
        
    };
    /*
     * return the power lines.
     */
    
    /*
     * Return the power line and power node info to the GUI during execution.
     */   
    string sessionInfo(){
        
    };
    
    /*
     * set exec_mode, mode =0, gui mode, mode=1, batch mode
     */
    void setExecMode(int mode){
        
    };
    /*
     * get exec_mode
     */
     int getExecMode(){
        
    };
  
    vector<ConnectedComponent*>& getCs(){
        
    };
     string reply(string ){
        
    };
    //same
    void DynamicCoordinatedLoadUpdate(string data);
    //same
    void DynamicCoordinatedBoundaryUpdate(string data);
    //same
    string find_tunnel_power_line(string peer1);
      
      
     map<string,Server>& getServers(){
         
     };
 
        /*
     * return the power line id based on two power interface peer.
     */
     
    //same
     string getLocalIp(){
         return localIp;
     };
     
     //not used
     bool find_tunnel(vector<string>& connected_component){
         
     };
   
   
    
    // same
     string CoordinatorIpOfConnectedComponent(string power_interface);
    // same
    void DynamicConnectedSimulation(string data);
    //same
    void restore();
    
    
};

#endif	/* FORWARDER_H */

