/* 
 * File:   power_simulator.h
 * Author: stan
 *
 * Created on January 8, 2013, 8:51 AM
 */

#ifndef POWER_SIMULATOR_H
#define	POWER_SIMULATOR_H
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netdb.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>
#include <map>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include<iostream>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sstream>
#include <algorithm>
#include "dynamic_connection.h"
#include "power_interface.h"
#include "node.h"
#include "power_line.h"
#include "server.h"
#include<strings.h>
#include "util.h"
#include <pthread.h>
#include "thread_functions.h"
#include "dynamic.h"
#include "entity.h"
//#include "ConnectedComponent.h"
#define SOCKET_BUFFER_SIZE 20000
#define OUTPUT_BUFFER_SIZE 10000
#define GUI_MODE 0
#define BATCH_MODE 1
using namespace std;
class ConnectedComponent;
//May need memory release for the whole object in the future
class PowerSimulator: public entity{
public:
    int exec_mode;
    char *input;
    int order; // node number
    int size;  // power line number
    int numberOfServer; // server number
    int numberOfPI; //power interface number
    map<string,Node> nodes;
    map<string,PowerLine> powerLines;
    map<string,Server> servers;
    map<string,PowerInterface> powerInterfaces;
    multimap<string,string> adj_list; //power interface and its adjacent.
    void create_adj_list();
    void dfs_visit(string power_interface);
    vector <vector <string> > connected_components;
    vector <ConnectedComponent*>Cs;

    // for distributed load balancing
    vector<double>desired_suppliess;
    vector<double>desired_demandss;
    
    pthread_t report_thread;
    pthread_t storage_thread;
    
    pthread_t tunnel_thread;
    string out_file;
    
    string localIp;
    
    string gui_client_ip;
    string find_power_line_resistor(string node1, string node2);
    
    string find_neighbors(string node);
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
    
   
   
  
    
//public:
    PowerSimulator();
    /*
     * initial the simulator based on input. Create the report thread;
     */
    void initialize(char* input){
        this->input=input;
        time_t rawtime;
        struct tm * timeinfo;
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        string s(asctime(timeinfo));
        s=s.erase(s.find_last_not_of(" \n\r\t")+1);
        out_file="/tmp/score_"+s+".log";
    
    }
    /*
     * specify the output file using the full path.
     */
    void set_out_file(string out_file){
        this->out_file=out_file;
    }
    /*
     * get the output file path.
     */
    string get_out_file(){
        return this->out_file;
    }
    void ancillary_threads_start(){
         int report_thread_create;
        report_thread_create = pthread_create(&report_thread, NULL, &real_time_report, this);
        if(report_thread_create != 0) {
            util::log("report thread create failed!");
        }
        // storage needs update. work later.
        int storage_thread_create;
        storage_thread_create=pthread_create(&storage_thread,NULL,&storage_monitor,this);
        if(storage_thread_create != 0) {
            util::log("storage create failed!");
        }
    }
    /*
     * return the power lines.
     */
    map<string,PowerLine>& getPowerLines();
    /*
     * return the nodes.
     */
    vector<ConnectedComponent*>& getCs(){
        return Cs;
    }
    map<string,Node>& getNodes();
    /*
     * parse the input and put all the information in basic data structures.
     */
    void parse(); 
    /*
     * calculate the current of each branches and each appliance. 
     */
    string simulate();
    /*
     * update the system based on the new current value or switch configuration
     */
    string update(string data);
    /*
     * Return the power line and power node info to the GUI during execution.
     */   
    string sessionInfo();
    
    /*
     * set exec_mode, mode =0, gui mode, mode=1, batch mode
     */
    void setExecMode(int mode);
    /*
     * get exec_mode
     */
    int getExecMode();
    /*
     * deconstructor
     */
    string reply(string data);
    ~PowerSimulator(){
         connected_components.clear();
         adj_list.clear();
         pthread_cancel(report_thread);
         pthread_cancel(storage_thread);
    }
      void DynamicCoordinatedLoadUpdate(string data);
      void DynamicCoordinatedBoundaryUpdate(string data);
    string find_tunnel_power_line(string peer1);
      
      
      map<string,Server>& getServers(){
          return servers;
      }
      map<string,PowerInterface>& getPowerInterfaces(){
          return powerInterfaces;
      }
      multimap<string,string>& get_adj_list(){
          return adj_list;
      }
        /*
     * return the power line id based on two power interface peer.
     */
    string find_power_line(string peer1, string peer2);
    string getLocalIp(){
      
        return localIp;
    }
     bool find_tunnel(vector<string>& connected_component);
    bool update_appliance(string node_id,double new_current);
 
    /*
     * return power_line id based on appliance node id.
     */
    string find_app_power_line(string app_node_id);
        /*
     * return the component index based on the power interface id.
     */
    int find_component(string power_interface);
    
    // deal with one connected component just for now.
    string CoordinatorIpOfConnectedComponent(string power_interface);
    
    void DynamicConnectedSimulation(string data);
    
    void restore();
};



#endif	/* POWER_SIMULATOR_H */

