/* 
 * File:   entiry.h
 * Author: stan
 *
 * Created on November 4, 2014, 11:03 AM
 */

#ifndef ENTIRY_H
#define	ENTIRY_H
#include "power_interface.h"
#include "node.h"
#include "power_line.h"
#include "server.h"
class ConnectedComponent;
class entity{

public:
   
    virtual ~entity(){ };
    virtual void create_adj_list()=0;
    virtual void dfs_visit(string power_interface)=0;
    virtual string find_power_line_resistor(string node1, string node2)=0;
    
    virtual string find_neighbors(string node)=0;
    /*
     * find the connected components in a graph.
     */
    virtual void find_connected_components()=0;
  
    /*
     * update appliance desired current
     */
    virtual void update_appliance_desired_current(string node_id,double request_current)=0;
  
    /*
     * update the appliance current and its connected power line's current.
     */
  
    /*
     * debugging 
     */
    virtual void print_everything()=0;
    
    virtual  void initialize(char*)=0;
    /*
     * specify the output file using the full path.
     */
    virtual void set_out_file(string )=0;
    /*
     * get the output file path.
     */
    virtual string get_out_file()=0;
    virtual void ancillary_threads_start()=0;
    /*
     * return the power lines.
     */
    virtual map<string,PowerLine>& getPowerLines()=0;
    /*
     * return the nodes.
     */
    virtual vector<ConnectedComponent*>& getCs()=0;
    virtual map<string,Node>& getNodes()=0;
    /*
     * parse the input and put all the information in basic data structures.
     */
    virtual void parse()=0; 
    /*
     * calculate the current of each branches and each appliance. 
     */
    virtual string simulate()=0;
    /*
     * update the system based on the new current value or switch configuration
     */
    virtual string update(string data)=0;
    /*
     * Return the power line and power node info to the GUI during execution.
     */   
    virtual string sessionInfo()=0;
    
    /*
     * set exec_mode, mode =0, gui mode, mode=1, batch mode
     */
    virtual void setExecMode(int mode)=0;
    /*
     * get exec_mode
     */
    virtual int getExecMode()=0;
    /*
     * deconstructor
     */
    virtual string reply(string )=0;
    virtual void DynamicCoordinatedLoadUpdate(string data)=0;
    virtual void DynamicCoordinatedBoundaryUpdate(string data)=0;
    virtual string find_tunnel_power_line(string peer1)=0;
      
      
    virtual map<string,Server>& getServers()=0;
    virtual map<string,PowerInterface>& getPowerInterfaces()=0;
    virtual multimap<string,string>& get_adj_list()=0;
        /*
     * return the power line id based on two power interface peer.
     */
    virtual string find_power_line(string peer1, string peer2)=0;
    virtual string getLocalIp()=0;
    virtual bool find_tunnel(vector<string>& connected_component)=0;
    virtual bool update_appliance(string node_id,double new_current)=0;
 
    /*
     * return power_line id based on appliance node id.
     */
    virtual string find_app_power_line(string app_node_id)=0;
        /*
     * return the component index based on the power interface id.
     */
    virtual int find_component(string power_interface)=0;
    
    // deal with one connected component just for now.
    virtual string CoordinatorIpOfConnectedComponent(string power_interface)=0;
    
    virtual void DynamicConnectedSimulation(string data)=0;
    
    virtual void restore()=0;
};


#endif	/* ENTIRY_H */

