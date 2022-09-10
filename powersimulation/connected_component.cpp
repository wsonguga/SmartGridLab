#include <map>
#include"connected_component.h"
#include "power_simulator.h"
#include "ipaddress.h"
#include <math.h>
#include <unistd.h>
ConnectedComponent::ConnectedComponent(){
    
}
ConnectedComponent::ConnectedComponent(forwarder* p,vector<string>& connected_component){
    this->connected_component=connected_component;
    this->ps=p;
    isTestbed=1;
    A=NULL; // switch_pi*switch_pi
    A_inv=NULL;//(switch_pi-1)*(switch_pi-1)
    B=NULL;
    U=NULL;
    I=NULL;
    y_L=NULL;
    N=NULL;
    A1_inv=NULL;
    A1B=NULL;
    AB1_A1_inv=NULL;
    y_L_inv_inv=NULL;
    is_distributed=false;
    is_balanced=false;
    is_dynamic_=false;
    pis_on_servers=new vector<string>[MAX_SERVER_NUMBER];
    servers_pis_num=new int[MAX_SERVER_NUMBER];
    servers_start_index=new int[MAX_SERVER_NUMBER];
    desired_supplies=0;
    desired_demands=0;
    switch_pi=0; //number of pi of switches
    coordinator_ip_="localhost";
}
ConnectedComponent::ConnectedComponent(PowerSimulator* p,vector<string>& connected_component){
    this->connected_component=connected_component;
    this->ps=p;
    isTestbed=0;
    A=NULL; // switch_pi*switch_pi
    A_inv=NULL;//(switch_pi-1)*(switch_pi-1)
    B=NULL;
    U=NULL;
    I=NULL;
    y_L=NULL;
    N=NULL;
    A1_inv=NULL;
    A1B=NULL;
    AB1_A1_inv=NULL;
    y_L_inv_inv=NULL;
    is_distributed=false;
    is_balanced=false;
    is_dynamic_=false;
    pis_on_servers=new vector<string>[MAX_SERVER_NUMBER];
    servers_pis_num=new int[MAX_SERVER_NUMBER];
    servers_start_index=new int[MAX_SERVER_NUMBER];
    desired_supplies=0;
    desired_demands=0;
    switch_pi=0; //number of pi of switches
    coordinator_ip_="localhost";
}
void ConnectedComponent::simulate(){
    
 
    is_balanced=load_balancing(); 
    
    if(is_balanced){
        for(int i=0;i<connected_component.size();i++){
          
             map<string,PowerInterface>::iterator it;
             it=ps->getPowerInterfaces().find(connected_component[i]);
             string nodeId=it->second.getNodeId();
             map<string,Node>::iterator it1;
             it1=ps->getNodes().find(nodeId);
             string role=it1->second.role;
             string location = it1->second.location;
             bool is_boundary=it->second.is_boundary_;
             bool is_server_taken=false;
             if(role=="Switch"){
                 /*See whether it is a boundary switch power interface*/
//                  multimap<string,string>::iterator it4;
//                  pair<multimap<string,string>::iterator,multimap<string,string>::iterator> ret;
//                    ret = ps->get_adj_list().equal_range(connected_component[i]);
//                    for (it4=ret.first; it4!=ret.second; ++it4){
//                        string neighbor=it4->second;
//                        map<string,PowerInterface>::iterator it5;
//                        it5=ps->getPowerInterfaces().find(neighbor);
//                        string neighbor_node_id=it5->second.getNodeId();
//                        map<string,Node>::iterator it6;
//                        it6=ps->getNodes().find(neighbor_node_id);
//                        string role=it6->second.role;
//                        if(role=="Tunnel"){
//                            it->second.is_boundary_=true;
//                            is_boundary=true;
//                            break;
//                        }
//                    }
                 /*boundary checking ends*/
                    
                    
                 for(int k=0;k<component_servers.size();k++){
                     if (location == component_servers[k]){
                       
                         //push the power interfaces into the exsiting power interface vector of that server.
                         is_server_taken=true;
                     //    if(!is_boundary)
                            pis_on_servers[k].push_back(connected_component[i]);
                    //     else
                      //      boundary_set.push_back(connected_component[i]);
                     }
                         
                 }
                 if(!is_server_taken){
                     //adding a new server
                     component_servers.push_back(location);
                     int size=component_servers.size();
                   //  if(!is_boundary)
                         pis_on_servers[size-1].push_back(connected_component[i]);
                   //  else
                   //      boundary_set.push_back(connected_component[i]);
                 }
               
                 switch_pi++;
             }
                 
        }
        // this assumes that there is only one server in the tunnel mode
     //   for(int i=0;i<boundary_set.size();i++){
      //      pis_on_servers[0].push_back(boundary_set[i]);
      //  }
        
        for (int i=0;i<component_servers.size();i++){
            for(int j=0;j<pis_on_servers[i].size();j++){
                int pre_index=0;
                 for(int k=0;k<i;k++)
                    pre_index+=pis_on_servers[k].size();
                pi_index_mapping[pis_on_servers[i][j]]=pre_index+j;
                index_pi_mapping[pre_index+j]=pis_on_servers[i][j];
            }
            int pre_size=0;
            for(int j=0;j<i;j++){
                  pre_size+=pis_on_servers[j].size();
            }
            servers_start_index[i]=pre_size;
            servers_pis_num[i]=pis_on_servers[i].size();
           
        }
        
        //no calculation is needed then.
        if(switch_pi<=1)
            return;
        else{
                A= gsl_matrix_alloc(switch_pi,switch_pi);
                 gsl_matrix_set_zero(A);
                A_inv=gsl_matrix_alloc(switch_pi-1,switch_pi-1);
                gsl_matrix_set_zero(A_inv);
                U=gsl_vector_alloc(switch_pi-1);
                 gsl_vector_set_zero(U);
                I=gsl_vector_alloc(switch_pi);
                 gsl_vector_set_zero(I);
                for(int i=0;i<switch_pi;i++){
                    map<int,string>::iterator it;
                    double current=0;
                    it=index_pi_mapping.find(i);
                    string power_interface=it->second;
                    map<string,PowerInterface>::iterator it3;
                    it3=ps->getPowerInterfaces().find(power_interface);
                    string node_id=it3->second.getNodeId();
                    
                    
                    multimap<string,string>::iterator it1;
                    pair<multimap<string,string>::iterator,multimap<string,string>::iterator> ret;
                    ret = ps->get_adj_list().equal_range(power_interface);
                    for (it1=ret.first; it1!=ret.second; ++it1){
                        string neighbor=it1->second;
                        
                        map<string,PowerInterface>::iterator it2;
                        it2=ps->getPowerInterfaces().find(neighbor);
                        string neighbor_node_id=it2->second.getNodeId();
                        map<string,Node>::iterator it3;
                        it3=ps->getNodes().find(neighbor_node_id);
                        string role=it3->second.role;
                        
                        if(role=="WindTurbine" || role == "SolarPanel" || role=="PowerPlant"){  
                              current+=it3->second.current; 
                              gsl_vector_set(I,i,current);
                            }
                        if(role=="Demander"){
                              current-=it3->second.current;
                              gsl_vector_set(I,i,current);
                        }
                        if(role=="Storage"){
                            string op_mode=it3->second.operation_mode;
                            if(op_mode=="charging"){
                                current-=it3->second.current;
                                gsl_vector_set(I,i,current);
                            }else if(op_mode=="discharging"){
                                current+=it3->second.current;
                                gsl_vector_set(I,i,current);
                            }else{
                                gsl_vector_set(I,i,0);
                            }
                            
                        }
                        if(role=="Switch"){
                               int sw2;
                               map<string,int>::iterator it;
                               it=pi_index_mapping.find(neighbor);
                               sw2=it->second;
                               gsl_matrix_set(A,i,sw2,-1);// the negative value of conductance.
                               //if the two power interfaces belong to two different
                               // switch nodes, then the power line between them is 
                               //transmission power line.
                               if(neighbor_node_id.compare(node_id)!=0){    
                                   string power_line=ps->find_power_line(power_interface,neighbor);
                                   map<string,PowerLine>::iterator pit;
                                   pit=ps->getPowerLines().find(power_line);
                                   double resistor=pit->second.resistor;
                                   gsl_matrix_set(A,i,sw2,-1.0/resistor);
                                   bool is_exist=false;
                                   for(int i=0;i<trans_power_line.size();i++){
                                       if(power_line.compare(trans_power_line[i])==0)
                                           is_exist=true;
                                   }
                                   if(!is_exist)
                                   trans_power_line.push_back(power_line);
                                   
                                   //edging lines between two servers.
                                   map<string,Node>::iterator nit;
                                   nit=ps->getNodes().find(node_id);
                                   string location1 = nit->second.location;
                                   nit=ps->getNodes().find(neighbor_node_id);
                                   string location2 = nit->second.location;
                                   if(location1!=location2){
                                       bool is=false;
                                       for(int i=0;i<edging_power_line.size();i++){
                                           if(power_line.compare(edging_power_line[i])==0)
                                               is=true;
                                       }
                                       if(!is)
                                           edging_power_line.push_back(power_line);
                                       
                                   }
                                   
                                   
                               }
                               
                          }
                       
                          
                        
                        
                    }
                    
                    
                    
                }
                 
             
      
               double sum=0.0;
               for(int i=0;i<switch_pi;i++){
                        sum=0.0;
                   for(int j=0;j<switch_pi;j++)
                       sum+=gsl_matrix_get(A,i,j);
                    gsl_matrix_set(A,i,i,-1.0*sum);
               }
             
             // parallel processing part, all the additional data
               // structure needed.
         int edging_power_line_num = edging_power_line.size();
         if( edging_power_line_num>0 ){
                is_distributed=true;
                N = gsl_matrix_alloc(switch_pi,edging_power_line_num);
                gsl_matrix_set_zero(N);
                y_L=gsl_matrix_alloc(edging_power_line_num,edging_power_line_num);
                gsl_matrix_set_zero(y_L);
                
                
                // the resistor of all power lines are hardcoded as 1
                for(int i=0;i<edging_power_line_num;i++){
                    string edging_line=edging_power_line[i];
                    map<string,PowerLine>::iterator pit;
                    pit=ps->getPowerLines().find(edging_line);
                    double resistor=pit->second.resistor;
                   gsl_matrix_set(y_L,i,i,1.0/resistor);
                }
                
                //init N matrix in chapter 6 of advanced power analysis book
                for(int i=0;i<edging_power_line_num;i++){
                    map<string,PowerLine>::iterator it;
                    it=ps->getPowerLines().find(edging_power_line[i]);
                    string lpeerPI=it->second.lpeerPI;
                    string rpeerPI=it->second.rpeerPI;
                    map<string,int>::iterator it1;
                    it1=pi_index_mapping.find(lpeerPI);
                    int index1=it1->second;
                    map<string,int>::iterator it2;
                    it2=pi_index_mapping.find(rpeerPI);
                    int index2=it2->second;
                    gsl_matrix_set(N,index1,i,1);
                    gsl_matrix_set(N,index2,i,1);
                } 
                
         }else{
               N = gsl_matrix_alloc(1,1);
               gsl_matrix_set_zero(N);
               gsl_matrix* y_L_inv_inv1=gsl_matrix_alloc(1,1);
         }
               int trans_power_line_num=trans_power_line.size();
               if(trans_power_line_num<=0){
                    //there should not be matrix B for this component. However,
                    //in order to make the index stay matching for all the other components,
                    //put a small matrix to increase the index.
                     B=gsl_matrix_alloc(1,1);
                     gsl_matrix_set_zero(B);
                     return;
                }else{
                   
                  B=gsl_matrix_alloc(trans_power_line_num,switch_pi);
                  gsl_matrix_set_zero(B);
                
                //The trans_power_line direction from the GUI is the same as the presumed current direction.
                for(int i=0;i<trans_power_line.size();i++){
                    map<string,PowerLine>::iterator it;
                    it=ps->getPowerLines().find(trans_power_line[i]);
                    string lpeer=it->second.lpeerPI;
                    string rpeer=it->second.rpeerPI;
                    map<string,int>::iterator it1;
                    it1=pi_index_mapping.find(lpeer);
                    int sw1=it1->second;
                    it1=pi_index_mapping.find(rpeer);
                    int sw2=it1->second;
                    gsl_matrix_set(B,i,sw1,1);
                    gsl_matrix_set(B,i,sw2,-1);
                }
               }     
               // calculation begins                   
        if(!is_distributed){   
               LocalCalculation();                 
        }      
        else{
             DistributedCalculation();
        }
      }  
    }
     return;
    
}
vector<string>& ConnectedComponent::getConnectedComponent(){
    return connected_component;
}
bool ConnectedComponent::load_balancing(){
    if(isTestbed ==0){
    double supplies=0;
    double demands=0;
    double desired_supplies=0;
    double desired_demands=0;
    double loads=0;
    for(int i=0;i<connected_component.size();i++){
        map<string,PowerInterface>::iterator it;
        it=ps->getPowerInterfaces().find(connected_component[i]);
        string nodeId=it->second.getNodeId();
        map<string,Node>::iterator it1;
        it1=ps->getNodes().find(nodeId);
        string role=it1->second.role;
        double desired_current=it1->second.desired_current;
        if(role.compare("WindTurbine")==0 || role=="SolarPanel" || role=="PowerPlant")
            desired_supplies+=desired_current;
        if(role.compare("Demander")==0){
            desired_demands+=desired_current;
        }
        if(role=="Storage"){
            string op_mode=it1->second.operation_mode;
            if(op_mode=="charging"){
                desired_demands+=desired_current;
            }
            else if (op_mode=="discharging"){
                desired_supplies+=desired_current;
            }
            
        }
            
    }
    
    // check is done above. continue the following.
    if(desired_supplies>=desired_demands){
            loads=desired_demands;
            supplies=desired_demands;
            demands=desired_demands;
            for(int i=0;i<connected_component.size();i++){
                string power_interface=connected_component[i];
                map<string,PowerInterface>::iterator it;
                it=ps->getPowerInterfaces().find(power_interface);
                string node_id=it->second.getNodeId();
                map<string,Node>::iterator it1;
                it1=ps->getNodes().find(node_id);
                 double ratio;
                 double current;
                 string role=it1->second.role;
                if(role=="WindTurbine" || role=="SolarPanel" || role=="PowerPlant" ){
                    ratio=(it1->second.desired_current)/desired_supplies;
                    current=loads*ratio;
                    ps->update_appliance(it1->second.id,current);

                }else if(role=="Demander"){
                      ps->update_appliance(it1->second.id,it1->second.desired_current);
                }else if(role=="Storage"){
                    string op_mode=it1->second.operation_mode;
                    if(op_mode=="charging"){
                        ps->update_appliance(it1->second.id,it1->second.desired_current);
                    }else if(op_mode=="discharging"){
                        ratio=(it1->second.desired_current)/desired_supplies;
                        current=loads*ratio;
                        ps->update_appliance(it1->second.id,current);
                    }else{
                        ps->update_appliance(it1->second.id,0);
                    }
                    
                }
            }
    }
    else{
            loads=desired_supplies;
            demands=desired_supplies;
            supplies=desired_supplies;
            
            for(int i=0;i<connected_component.size();i++){
                string power_interface=connected_component[i];
                map<string,PowerInterface>::iterator it;
                it=ps->getPowerInterfaces().find(power_interface);
                string node_id=it->second.getNodeId();
                map<string,Node>::iterator it1;
                it1=ps->getNodes().find(node_id);
                double ratio;
                double current;
                string role=it1->second.role;
                if(role=="Demander"){
                    ratio=(it1->second.desired_current)/desired_demands;
                    current=loads*ratio;
                    ps->update_appliance(it1->second.id,current);

                }else if(role=="WindTurbine" || role=="SolarPanel" || role=="PowerPlant"){
                    ps->update_appliance(it1->second.id,it1->second.desired_current);
                }else if(role=="Storage"){
                    string op_mode=it1->second.operation_mode;
                    if(op_mode=="charging"){
                        ratio=(it1->second.desired_current)/desired_demands;
                        current=loads*ratio;
                        ps->update_appliance(it1->second.id,current);
                    }else if(op_mode=="discharging"){
                       ps->update_appliance(it1->second.id,it1->second.desired_current);  
                    }else{
                         ps->update_appliance(it1->second.id,0);
                    }
                }

            }
            

        }
        if(supplies==demands && demands==loads)
            return true;
        else
            return false;
    }
    else{ //it is testbed
         for(int i=0;i<connected_component.size();i++){
                string power_interface=connected_component[i];
                map<string,PowerInterface>::iterator it;
                it=ps->getPowerInterfaces().find(power_interface);
                string node_id=it->second.getNodeId();
                map<string,Node>::iterator it1;
                it1=ps->getNodes().find(node_id);
                 string role=it1->second.role;
                if(role=="WindTurbine" || role=="SolarPanel" || role=="PowerPlant" ){
                    ps->update_appliance(it1->second.id,it1->second.current);
                }else if(role=="Demander"){
                      ps->update_appliance(it1->second.id,it1->second.current);
                }else if(role=="Storage"){
                    string op_mode=it1->second.operation_mode;
                    if(op_mode=="charging"){
                        ps->update_appliance(it1->second.id,it1->second.current);
                    }else if(op_mode=="discharging"){
                        ps->update_appliance(it1->second.id,it1->second.current);
                    }else{
                        ps->update_appliance(it1->second.id,0);
                    }
                    
                }
            }
            return true;
    }
}
bool ConnectedComponent::DynamicLoadBalancing(){
    cout<<"Dynamic load balancing:"<<endl;
    
    double demands=0;
    double supplies=0;
    desired_supplies=0;
    desired_demands=0;
    for(int i=0;i<connected_component.size();i++){
        map<string,PowerInterface>::iterator it;
        it=ps->getPowerInterfaces().find(connected_component[i]);
        string nodeId=it->second.getNodeId();
        map<string,Node>::iterator it1;
        it1=ps->getNodes().find(nodeId);
        string role=it1->second.role;
        double desired_current=it1->second.desired_current;
        if(role.compare("WindTurbine")==0 || role=="SolarPanel" || role=="PowerPlant")
            desired_supplies+=desired_current;
        if(role.compare("Demander")==0){
            desired_demands+=desired_current;
        }
        if(role=="Storage"){
            string op_mode=it1->second.operation_mode;
            if(op_mode=="charging"){
                desired_demands+=desired_current;
            }
            else if (op_mode=="discharging"){
                desired_supplies+=desired_current;
            }
            
        }
            
    }
    string host=coordinator_ip_;
    int port = MASTER_PORT;
    char *data=new char[1024];
    bzero(data,1024);
    std::ostringstream oss(data);
    
   
    oss<<"Desired demands and supplies:"<<endl;
    oss<<ps->getLocalIp()<<":"<<desired_demands<<":"<<desired_supplies<<endl;
    string result=util::exec_on_server(host,port,oss.str().c_str());
    
  if(isTestbed==0){
    string line;
    std::istringstream iss(result);
    getline(iss,line);
    string value;
    std::istringstream ssline(line);
    int flag;
    int field=0;
    while (getline ( ssline, value, ':' )){
        if(field==0)
            flag=atoi(value.c_str());
        if(field==1)
            demands=atof(value.c_str());
        if(field==2)
            supplies=atof(value.c_str());
        field++;
    }
   string node_id;
   string power_interface;
   string role;
      if(flag==1){
          
            for(int i=0;i<connected_component.size();i++){
                power_interface=connected_component[i];
                map<string,PowerInterface>::iterator it;
                it=ps->getPowerInterfaces().find(power_interface);
                node_id=it->second.getNodeId();
                map<string,Node>::iterator it1;
                it1=ps->getNodes().find(node_id);
                 double ratio;
                 double current;
                 role=it1->second.role;
                if(role=="WindTurbine" || role=="SolarPanel" || role=="PowerPlant" ){
                    ratio=(it1->second.desired_current)/desired_supplies;
                    current=supplies*ratio;
                    ps->update_appliance(it1->second.id,current);

                }else if(role=="Demander"){
                      ps->update_appliance(it1->second.id,it1->second.desired_current);
                }else if(role=="Storage"){
                    string op_mode=it1->second.operation_mode;
                    if(op_mode=="charging"){
                        ps->update_appliance(it1->second.id,it1->second.desired_current);
                    }else if(op_mode=="discharging"){
                        ratio=(it1->second.desired_current)/desired_supplies;
                        current=supplies*ratio;
                        ps->update_appliance(it1->second.id,current);
                    }else{
                        ps->update_appliance(it1->second.id,0);
                    }
                    
                }
            }
    }
    else{
            
            
            for(int i=0;i<connected_component.size();i++){
                power_interface=connected_component[i];
                map<string,PowerInterface>::iterator it;
                it=ps->getPowerInterfaces().find(power_interface);
                node_id=it->second.getNodeId();
                map<string,Node>::iterator it1;
                it1=ps->getNodes().find(node_id);
                double ratio;
                double current;
                role=it1->second.role;
                if(role=="Demander"){
                    ratio=(it1->second.desired_current)/desired_demands;
                    current=demands*ratio;
                    ps->update_appliance(it1->second.id,current);

                }else if(role=="WindTurbine" || role=="SolarPanel" || role=="PowerPlant"){
                    ps->update_appliance(it1->second.id,it1->second.desired_current);
                }else if(role=="Storage"){
                    string op_mode=it1->second.operation_mode;
                    if(op_mode=="charging"){
                        ratio=(it1->second.desired_current)/desired_demands;
                        current=demands*ratio;
                        ps->update_appliance(it1->second.id,current);
                    }else if(op_mode=="discharging"){
                       ps->update_appliance(it1->second.id,it1->second.desired_current);  
                    }else{
                         ps->update_appliance(it1->second.id,0);
                    }
                }

            }
            

        }
    
    
    
    return true; }
    else{
        for(int i=0;i<connected_component.size();i++){
                string power_interface=connected_component[i];
                map<string,PowerInterface>::iterator it;
                it=ps->getPowerInterfaces().find(power_interface);
                string node_id=it->second.getNodeId();
                map<string,Node>::iterator it1;
                it1=ps->getNodes().find(node_id);
                 string role=it1->second.role;
                if(role=="WindTurbine" || role=="SolarPanel" || role=="PowerPlant" ){
                    ps->update_appliance(it1->second.id,it1->second.current);
                }else if(role=="Demander"){
                      ps->update_appliance(it1->second.id,it1->second.current);
                }else if(role=="Storage"){
                    string op_mode=it1->second.operation_mode;
                    if(op_mode=="charging"){
                        ps->update_appliance(it1->second.id,it1->second.current);
                    }else if(op_mode=="discharging"){
                        ps->update_appliance(it1->second.id,it1->second.current);
                    }else{
                        ps->update_appliance(it1->second.id,0);
                    }
                    
                }
            }
            return true;
    }
    
}
void ConnectedComponent::Update(){
    bool is_balanced=load_balancing(); 
    if(is_balanced){
      if(switch_pi<=1)
            return; 
      for(int i=0;i<switch_pi;i++){
            map<int,string>::iterator it;
            double current=0;
            it=index_pi_mapping.find(i);
            string power_interface=it->second;
            map<string,PowerInterface>::iterator it3;
            it3=ps->getPowerInterfaces().find(power_interface);
            string node_id=it3->second.getNodeId();
                 
            multimap<string,string>::iterator it1;
            pair<multimap<string,string>::iterator,multimap<string,string>::iterator> ret;
            ret = ps->get_adj_list().equal_range(power_interface);
            for (it1=ret.first; it1!=ret.second; ++it1){
            string neighbor=it1->second;
                        
            map<string,PowerInterface>::iterator it2;
            it2=ps->getPowerInterfaces().find(neighbor);
            string neighbor_node_id=it2->second.getNodeId();
            map<string,Node>::iterator it3;
            it3=ps->getNodes().find(neighbor_node_id);
            string role=it3->second.role;
                        
            if(role =="WindTurbine" || role=="SolarPanel" || role=="PowerPlant"){  
                    current+=it3->second.current; 
                    gsl_vector_set(I,i,current);
            }
            if(role=="Demander"){
                    current-=it3->second.current;
                    gsl_vector_set(I,i,current);
            }
            if(role=="Storage"){
                string op_mode=it3->second.operation_mode;
                if(op_mode=="charging"){
                    current-=it3->second.current;
                    gsl_vector_set(I,i,current);
                }else if(op_mode=="discharging"){
                                current+=it3->second.current; 
                                gsl_vector_set(I,i,current);
                            }else{
                                gsl_vector_set(I,i,0);
                            }
                        }
          
                    }
            }                        
          if(!is_distributed){
                gsl_vector* U=gsl_vector_alloc(switch_pi-1);
                gsl_vector_view I1=gsl_vector_subvector(I,0,switch_pi-1);
                gsl_blas_dgemv(CblasNoTrans,1,A_inv,&I1.vector,0,U);
                
                for(int i=0;i<switch_pi-1;i++){
                    if(gsl_vector_get(U,i)<0.00001 && gsl_vector_get(U,i)>-0.0001){
                       gsl_vector_set(U,i,0);
                    }
                }
                
                //if there is no matrix B and no trans_power_line for that connected component
                int trans_power_line_num=trans_power_line.size();
                if(trans_power_line_num<=0){
                    return;
                }
                gsl_matrix_view B1=gsl_matrix_submatrix(B,0,0,trans_power_line.size(),switch_pi-1);
 
                gsl_vector *I_b=gsl_vector_alloc(trans_power_line.size());
                gsl_blas_dgemv(CblasNoTrans,1,&B1.matrix,U,0,I_b);
                for(int i=0;i<trans_power_line.size();i++){
                    map<string,PowerLine>::iterator it;
                    it=ps->getPowerLines().find(trans_power_line[i]);
                    double resistor=it->second.resistor;
                    double current=gsl_vector_get(I_b,i)/resistor;
                    if(current<0.0000001 && current>-0.0000001){
                        current=0;
                        
                    }
                    it->second.current=current;
                }
                  gsl_vector_free(U);
                  gsl_vector_free(I_b);
          }else{
            
              
                 int edging_power_line_num=edging_power_line.size();
                 int trans_power_line_num=trans_power_line.size();
                 if(trans_power_line_num<=0){
                    return;
                 }
                  gsl_vector* second_part_first_phase_sum=gsl_vector_alloc(edging_power_line_num);
                  gsl_vector_set_zero (second_part_first_phase_sum);
                  gsl_vector_view I_1=gsl_vector_subvector(I,0,switch_pi-1);
                  gsl_matrix_view N1=gsl_matrix_submatrix(N,0,0,switch_pi-1,edging_power_line_num);
                  // should use multi threads in the future.
                  for(int i=0;i<component_servers.size();i++){
                      
                      map<string,Server>::iterator it;
                      it=ps->getServers().find(component_servers[i].substr(0,component_servers[i].length()));
                      string host=it->second.host;
                      int port=it->second.port;
                      char *data=new char[SOCKET_BUFFER_SIZE];
                      bzero(data,SOCKET_BUFFER_SIZE);
                      std::ostringstream oss1(data);
                     
                      int server_start_index=servers_start_index[i];
                      int server_pis_num=servers_pis_num[i];
                        
                      if((server_start_index+server_pis_num)==switch_pi){
                          server_pis_num-=1;
                      }
                      oss1<<"Update First Phase:"<<endl;
                      for(int k=server_start_index;k<server_start_index+server_pis_num;k++){
                          double data1=gsl_vector_get(&I_1.vector,k);
                          if(data1 !=0)
                              oss1<<"I:"<<k-server_start_index<<":"<<data1<<endl;
                      }
                      string result_first_phase=util::exec_on_server(host,port,oss1.str().c_str());
                      delete[] data;
                      std::istringstream iss(result_first_phase);
                      string line;
                      while(getline(iss,line)){
                           string value;
                           istringstream ssline(line);
                           int field=0;
                           int flag=-1;
                           while(getline(ssline,value,':')){
                               if(field==0){
                                   if(value=="1")
                                       flag=1;//first part result of first phase;
                                   if(value=="2")
                                       flag=2;//second part result of first phase;
                                   field++;
                                   continue;
                               }
                               if(flag==2){
                                   int row;
                                   double val;
                                   if(field==1){
                                       row=atoi(value.c_str());
                                   }
                                   if(field==2){
                                       val=atof(value.c_str());
                                       val*=-1;
                                       double pre=gsl_vector_get(second_part_first_phase_sum,row);
                                       val+=pre;
                                       gsl_vector_set(second_part_first_phase_sum,row,val);
                                   }
                                   field++;
                               }
                               
                           }
                          
                      }
                    
                            
                  }
                  
               gsl_vector* i_L=gsl_vector_alloc(edging_power_line_num);
               gsl_blas_dgemv (CblasNoTrans,1,y_L_inv_inv, second_part_first_phase_sum,0, i_L);

               
               gsl_vector* Ni_L=gsl_vector_alloc(switch_pi-1);
               gsl_blas_dgemv (CblasNoTrans,-1,&N1.matrix, i_L,0, Ni_L);
               gsl_vector_add(Ni_L,&I_1.vector);
               
               gsl_vector* U2=gsl_vector_alloc(switch_pi-1);
               for(int i=0;i<component_servers.size();i++){
                   map<string,Server>::iterator it;
                   it=ps->getServers().find(component_servers[i].substr(0,component_servers[i].length()));
                   string host=it->second.host;
                   int port=it->second.port;
                   char *data=new char[SOCKET_BUFFER_SIZE];
                   bzero(data,SOCKET_BUFFER_SIZE);
                   std::ostringstream oss1(data);   
                   int server_start_index=servers_start_index[i];
                   int server_pis_num=servers_pis_num[i];
                        
                   if((server_start_index+server_pis_num)==switch_pi){
                          server_pis_num-=1;
                   }
                   oss1<<"Update Second Phase:"<<endl;
                   for(int k=server_start_index;k<server_start_index+server_pis_num;k++){
                       double data1=gsl_vector_get(Ni_L,k);
                       if(data1!=0)                    
                    oss1<<"Ni_L:"<<k-server_start_index<<":"<<data1<<endl;     
                   }
                   string result_second_phase=util::exec_on_server(host,port,oss1.str().c_str());
                       std::istringstream iss(result_second_phase);
                       string line;
                       int line_num=0;
                     while(getline(iss,line)){
                        if(line!=""){
                           gsl_vector_set(U2,server_start_index+line_num,atof(line.c_str()));
                        }
                         line_num++;
                     }
               }
           
               gsl_matrix_view B2=gsl_matrix_submatrix(B,0,0,trans_power_line.size(),switch_pi-1);
               gsl_vector *I_b2=gsl_vector_alloc(trans_power_line.size());
               gsl_blas_dgemv(CblasNoTrans,1,&B2.matrix,U2,0,I_b2);
                //This I_b is actually branch voltage vector in website link tutorial,
                //since the branch resistor is 1, so the branch conductance matrix is identity matrix,
                // the multiplication result is the same.
                for(int i=0;i<trans_power_line.size();i++){
                    map<string,PowerLine>::iterator it;
                    it=ps->getPowerLines().find(trans_power_line[i]);
                    double resistor=it->second.resistor;
                    double current=gsl_vector_get(I_b2,i)/resistor;
                    if(current<0.0000001 && current>-0.0000001){
                        current=0;
                        
                    }
                    it->second.current=current;
                }
          gsl_vector_free(U2);
          gsl_vector_free(I_b2);
          gsl_vector_free(Ni_L);
          gsl_vector_free(second_part_first_phase_sum);
                      
          }
          
          
          
        
     }

    
    
    
    
    
}
void ConnectedComponent::DynamicCalculation(){
                int s;
                int boundary_node_num=boundary_set.size();
                gsl_matrix* A_temp=gsl_matrix_alloc(switch_pi,switch_pi);
                gsl_matrix_memcpy(A_temp,A);
                if(A1_inv != NULL){
                    gsl_matrix_free(A1_inv);
                }
                A1_inv=gsl_matrix_alloc(switch_pi-boundary_node_num,
                        switch_pi-boundary_node_num);
                
                gsl_matrix_view A1B_view=gsl_matrix_submatrix(A_temp,0,switch_pi-boundary_node_num,
                        switch_pi-boundary_node_num,boundary_node_num);
                if(A1B != NULL){
                    gsl_matrix_free(A1B);
                }
                A1B=gsl_matrix_alloc(switch_pi-boundary_node_num,boundary_node_num);
                gsl_matrix_memcpy(A1B,&A1B_view.matrix);
                
                
              gsl_matrix_view A1=gsl_matrix_submatrix(A_temp,0,0,switch_pi-boundary_node_num,
                   switch_pi-boundary_node_num);
                cout<<"matrix A1:"<<endl;
                for(int i=0;i<switch_pi-boundary_node_num;i++){
                    for(int j=0;j<switch_pi-boundary_node_num;j++){
                        cout<<gsl_matrix_get(&A1.matrix,i,j)<<",";
                    }
                cout<<endl;
             }
              
              
              gsl_vector_view I1=gsl_vector_subvector(I,0,switch_pi-boundary_node_num);
              cout<<"Vector I1"<<endl;
              for(int i=0;i<switch_pi-boundary_node_num;i++){
                  cout<<gsl_vector_get(&I1.vector,i)<<endl;
              }
              cout<<"matrix A1B:"<<endl;
                for(int i=0;i<switch_pi-boundary_node_num;i++){
                    for(int j=0;j<boundary_node_num;j++){
                        cout<<gsl_matrix_get(&A1B_view.matrix,i,j)<<",";
                    }
                cout<<endl;
             }
              
              
                gsl_permutation * p = gsl_permutation_alloc (switch_pi-boundary_node_num);
                 gsl_linalg_LU_decomp (&A1.matrix, p, &s);
                gsl_linalg_LU_invert (&A1.matrix, p, A1_inv);
                if(AB1_A1_inv != NULL){
                    gsl_matrix_free(AB1_A1_inv);
                }
                AB1_A1_inv=gsl_matrix_alloc(boundary_node_num,switch_pi-boundary_node_num);
                gsl_blas_dgemm(CblasTrans,CblasNoTrans,1, &A1B_view.matrix,A1_inv,0,AB1_A1_inv);
               
                
                gsl_matrix* deltaX=gsl_matrix_alloc(boundary_node_num,boundary_node_num);
                gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1,AB1_A1_inv,&A1B_view.matrix,0,deltaX);
                
                gsl_vector* deltaY=gsl_vector_alloc(boundary_node_num); 
                gsl_blas_dgemv(CblasNoTrans,1,AB1_A1_inv,&I1.vector,0,deltaY);
                
                
              //  gsl_matrix_free(AB1_A1_inv);
                
                   
              
                
                
                cout<<"matrix deltaX:"<<endl;
                for(int i=0;i<boundary_node_num;i++){
                    for(int j=0;j<boundary_node_num;j++){
                        cout<<gsl_matrix_get(deltaX,i,j)<<",";
                    }
                cout<<endl;
             }
                cout<<"vector deltaY:"<<endl;
                for(int i=0;i<boundary_node_num;i++){
                    double current=gsl_vector_get(deltaY,i);
                    if(current<0.0000001 && current >-0.000001)
                        gsl_vector_set(deltaY,i,0);
                    cout<<gsl_vector_get(deltaY,i)<<endl;
                }
                 string host=coordinator_ip_;
                 int port = MASTER_PORT;
                 char *data=new char[1024];
                 bzero(data,1024);
                 std::ostringstream oss(data);
                 oss<<"coordinating info:"<<endl;
                 //assumes one boundary node with one remote machine connected.
                 oss<<ps->getLocalIp()<<":"<<boundary_set.size()<<endl;
                 // the boundary node interface:its corresponding diagonal entry without boundary 
                 //connection , the host and 
                 // power interface it is going to connected with. 
                 for(int i=0;i<boundary_set.size();i++){
                         map<string,int>::iterator it;
                               it=pi_index_mapping.find(boundary_set[i]);
                               int ss=it->second;
                               double entry=gsl_matrix_get(A,ss,ss);
                               // The injection current is not sent. assume is 0.
                               // one connection corresponding with one boundary nodes.   
                     oss<<boundary_set[i]<<":"<<entry<<":"<<dynamic_connections[boundary_set.size()-1-i].remote_ip<<":"<<dynamic_connections[boundary_set.size()-1-i].remote_pi<<endl;
                 }
                 for(int i=0;i<boundary_set.size();i++){
                     for(int j=0;j<boundary_set.size();j++)
                         oss<<gsl_matrix_get(deltaX,i,j)<<":";
                     oss<<endl;
                 }
                   for(int i=0;i<boundary_set.size();i++){
                         oss<<gsl_vector_get(deltaY,i)<<endl;
                 }
                 
                 string result=util::exec_on_server(host,port,oss.str().c_str());
                 cout<<"coordinated VB:"<<result<<endl;
                 
                 gsl_vector* Usss=gsl_vector_alloc(switch_pi);
                 gsl_vector_set_zero(Usss);
                 gsl_vector* VB=gsl_vector_alloc(boundary_set.size());
                 string line;
                 std::istringstream iss(result);
                 
                 for(int i=0;i<boundary_node_num;i++){
                       string value;
                          getline(iss,line);
                     std::istringstream ssline(line);
                     int field=0;
                   while (getline ( ssline, value, ':' )){
                      if(field==0){
                       gsl_vector_set(VB,i,atof(value.c_str()));
                       gsl_vector_set(Usss,switch_pi-boundary_node_num+i,atof(value.c_str()));
                      }
                      if(field==1){
                          double current=atof(value.c_str());
                          map<int,string>::iterator it1;
                          it1=index_pi_mapping.find(switch_pi-boundary_node_num+i);
                          string pi=it1->second;
                          string power_line=ps->find_tunnel_power_line(pi);
                         map<string,PowerLine>::iterator it;
                         it=ps->getPowerLines().find(power_line);
                         it->second.current=current;
                         
                         
                         //This is the place to set the interface module current in testbed. 
                         if(isTestbed==1){
                              int send_sock;
                              struct hostent *host1;
                              struct sockaddr_in server_addr1;
                              char datastd[50];
                              bzero(datastd,50);
                              char ts[10];
                              string ss="Update session:";
                              sprintf(ts,"%d:",18);
                              strcat(datastd,ss.c_str());
                              strcat(datastd,ts);
        
        
                                            
                              char settings[10];                   
                              if(current >0){
                                   int ss=int(fabs(current))+32768;
                                   sprintf(settings,"%d",ss);
                              }
                              else{
                                 sprintf(settings,"%d",(int)fabs(current+1));
                              }
            
                              strcat(datastd,settings);
        
                              strcat(datastd,"\r\n");
                              printf("%s,$$,%s",interfaceIp.c_str(),datastd);
        
                              host1 = gethostbyname(interfaceIp.c_str());
                            if ((send_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                                 printf("Socket creation failed! ");
                                 continue;   
                            }

                            server_addr1.sin_family = AF_INET;
                              // the server port on java sgmonitor
                            server_addr1.sin_port = htons(9009);
                            server_addr1.sin_addr = *((struct in_addr *)host1->h_addr);
                            bzero(&(server_addr1.sin_zero),8);

                           if (connect(send_sock, (struct sockaddr *)&server_addr1,
                              sizeof(struct sockaddr)) == -1)
                           {
                              printf("Connect failed!!! Beagleboard may not be booted.\n");
                            
                           }
                          send(send_sock,datastd,50, 0); 
                          printf("sent! %d\n",i);
     
                           close(send_sock); 
                         
                         }
                         
                         
                         
                   
                      }
                   
                       field++;
                    } 
               }
                 
                 
                 
                 
                 
                 
                 gsl_vector* A1B_VB=gsl_vector_alloc(switch_pi-boundary_node_num);
                 gsl_blas_dgemv(CblasNoTrans,1,&A1B_view.matrix,VB,0,A1B_VB);
                 gsl_vector_sub (&I1.vector, A1B_VB);
                 gsl_vector* V_A=gsl_vector_alloc(switch_pi-boundary_node_num);
                 gsl_blas_dgemv(CblasNoTrans,1,A1_inv,&I1.vector,0,V_A);
                 for(int i=0;i<switch_pi-boundary_node_num;i++){
                     gsl_vector_set(Usss,i,gsl_vector_get(V_A,i));
                 }                
                 
                 cout<<"Final Usss:"<<endl;
                 for(int i=0;i<switch_pi;i++)
                     cout<<gsl_vector_get(Usss,i)<<endl;
                
               
                gsl_matrix_view B1=gsl_matrix_submatrix(B,0,0,trans_power_line.size(),switch_pi);
 
                gsl_vector *I_b=gsl_vector_alloc(trans_power_line.size());
                gsl_blas_dgemv(CblasNoTrans,1,&B1.matrix,Usss,0,I_b);
                
                
                
                
                
                //This I_b is actually branch voltage vector in website link tutorial,
                //since the branch resistor is 1, so the branch conductance matrix is identity matrix,
                // the multiplication result is the same.
                for(int i=0;i<trans_power_line.size();i++){
                    map<string,PowerLine>::iterator it;
                    it=ps->getPowerLines().find(trans_power_line[i]);
                    double resistor=it->second.resistor;
                    double current=gsl_vector_get(I_b,i)/resistor;
                    if(current<0.0000001 && current>-0.0000001){
                        current=0;
                        
                    }
                    it->second.current=current;
                }
                   gsl_vector_free(I_b); 
                  // gsl_matrix_free(A1_inv);
                   gsl_vector_free(V_A);
                   gsl_vector_free(A1B_VB);
                   gsl_vector_free(VB);
                   gsl_matrix_free(A_temp);
                   gsl_matrix_free(deltaX);
                   gsl_vector_free(deltaY);
                   gsl_vector_free(Usss);
                                       
}
void ConnectedComponent::LocalCalculation(){
    
    //This is the place to send testbed update to the coordinator.
    
     gsl_matrix* A_temp=gsl_matrix_alloc(switch_pi,switch_pi);
     gsl_matrix_memcpy(A_temp,A);
     gsl_matrix_view A1=gsl_matrix_submatrix(A_temp,0,0,switch_pi-1,switch_pi-1);
     gsl_permutation * p = gsl_permutation_alloc(switch_pi-1);
                int signum;
                
                //this decomposition process has changed matrix A.
                gsl_linalg_LU_decomp (&A1.matrix, p, &signum);
                gsl_linalg_LU_invert (&A1.matrix, p, A_inv);
                gsl_permutation_free(p);  
                gsl_vector_view I1=gsl_vector_subvector(I,0,switch_pi-1);
                gsl_blas_dgemv(CblasNoTrans,1,A_inv,&I1.vector,0,U);
                gsl_matrix_view B1=gsl_matrix_submatrix(B,0,0,trans_power_line.size(),switch_pi-1);
 
                gsl_vector *I_b=gsl_vector_alloc(trans_power_line.size());
                gsl_blas_dgemv(CblasNoTrans,1,&B1.matrix,U,0,I_b);
                
                cout<<"normal U:"<<endl;
                for(int i=0;i<switch_pi-1;i++)
                    cout<<gsl_vector_get(U,i)<<endl;
                //This I_b is actually branch voltage vector in website link tutorial,
                //since the branch resistor is 1, so the branch conductance matrix is identity matrix,
                // the multiplication result is the same.
                for(int i=0;i<trans_power_line.size();i++){
                    map<string,PowerLine>::iterator it;
                    it=ps->getPowerLines().find(trans_power_line[i]);
                    double resistor=it->second.resistor;
                    double current=gsl_vector_get(I_b,i)/resistor;
                    
                    if(current<0.0000001 && current>-0.0000001){
                        current=0;
                        
                    }
                    it->second.current=current;
                }
                gsl_vector_free(I_b);
                gsl_vector_free(U);                  
                gsl_matrix_free(A_temp);                            
}
void ConnectedComponent::DistributedCalculation(){
       int edging_power_line_num = edging_power_line.size();
       //change from A to Y_b.
       
       //Matrix A is changed.
       
       // choose a node as a reference node, in this case, the last one in matrix
       gsl_matrix_view Y_b=gsl_matrix_submatrix(A,0,0,switch_pi-1,switch_pi-1);
       gsl_vector_view I_1=gsl_vector_subvector(I,0,switch_pi-1);
       gsl_matrix_view N1=gsl_matrix_submatrix(N,0,0,switch_pi-1,edging_power_line_num);
       gsl_matrix* N1y_L=gsl_matrix_alloc(switch_pi-1,edging_power_line_num);
       gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1, &N1.matrix,y_L,0,N1y_L);
       gsl_matrix* N1y_LN1_T=gsl_matrix_alloc(switch_pi-1,switch_pi-1);
       gsl_blas_dgemm(CblasNoTrans,CblasTrans,1,N1y_L,&N1.matrix,0,N1y_LN1_T);
       gsl_matrix_add(&(Y_b.matrix),N1y_LN1_T);
       gsl_matrix_free(N1y_L);
       gsl_matrix_free(N1y_LN1_T);
       //calculate y_L inverse. Since y_L is a diagonal matrix,
       // the inverse is just put the inverse of diagonal element in the diagonal.
       gsl_matrix* y_L_inv=gsl_matrix_alloc(edging_power_line_num,edging_power_line_num);
       gsl_matrix_set_zero(y_L_inv);
       for(int i=0;i<edging_power_line_num;i++){
           double data=gsl_matrix_get(y_L,i,i);              
           double data_inv=1/data;
           gsl_matrix_set(y_L_inv,i,i,data_inv);
       }
       gsl_vector* second_part_first_phase_sum=gsl_vector_alloc(edging_power_line_num);
       gsl_vector_set_zero (second_part_first_phase_sum);
                  
       // should use multi threads in the future.
       for(int i=0;i<component_servers.size();i++){
                      
           map<string,Server>::iterator it;
           it=ps->getServers().find(component_servers[i].substr(0,component_servers[i].length()));
           if(it== ps->getServers().end()){
                cout<<"Server not found!"<<endl;
           }
           string host=it->second.host;
           int port=it->second.port;
           char *data=new char[SOCKET_BUFFER_SIZE];
           bzero(data,SOCKET_BUFFER_SIZE);
           std::ostringstream oss1(data);
                     
           int server_start_index=servers_start_index[i];
           int server_pis_num=servers_pis_num[i];
                        
           if((server_start_index+server_pis_num)==switch_pi){
                   server_pis_num-=1;
           }
           oss1<<"First Phase:"<<endl;
           oss1<<server_pis_num<<":"<<edging_power_line_num<<endl;                     
           for(int k=server_start_index;k<server_start_index+server_pis_num;k++){
               for (int j=server_start_index;j<server_start_index+server_pis_num;j++){
                  double data1=gsl_matrix_get(&Y_b.matrix,k,j);
                  if (data1 !=0)
                   oss1<<"Yb:"<<k-server_start_index<<":"<<j-server_start_index<<":"<<data1<<endl;
               }  
           }
           for(int k=server_start_index;k<server_start_index+server_pis_num;k++){
                for(int j=0;j<edging_power_line_num;j++){
                   double data1=gsl_matrix_get(&N1.matrix,k,j);
                   if(data1!=0 )
                     oss1<<"N:"<<k-server_start_index<<":"<<j<<":"<<data1<<endl;                            
                }
           }
           for(int k=server_start_index;k<server_start_index+server_pis_num;k++){
                  double data1=gsl_vector_get(&I_1.vector,k);
                  if(data1 !=0)
                      oss1<<"I:"<<k-server_start_index<<":"<<data1<<endl;
           }
           string result_first_phase=util::exec_on_server(host,port,oss1.str().c_str());
           delete[] data;
           std::istringstream iss(result_first_phase);
           string line;
          while(getline(iss,line)){
            string value;
            istringstream ssline(line);
            int field=0;
            int flag=-1;
            while(getline(ssline,value,':')){
                if(field==0){
                    if(value=="1")
                     flag=1;//first part result of first phase;
                    if(value=="2")
                     flag=2;//second part result of first phase;
                     field++;
                     continue;
                    }
                    if(flag==1){
                        int row,col;
                        double val;
                       if(field==1)
                        row=atoi(value.c_str());
                       if(field==2){
                        col=atoi(value.c_str());
                       }
                       if(field==3){
                        val=atof(value.c_str());
                        val*=-1;
                        double pre=gsl_matrix_get(y_L_inv,row,col);
                        val+=pre;
                        gsl_matrix_set(y_L_inv,row,col,val);
                        }
                        field++;
                    }
                    if(flag==2){
                        int row;
                        double val;
                      if(field==1){
                        row=atoi(value.c_str());
                      }
                      if(field==2){
                        val=atof(value.c_str());
                        val*=-1;
                        double pre=gsl_vector_get(second_part_first_phase_sum,row);
                        val+=pre;
                        gsl_vector_set(second_part_first_phase_sum,row,val);
                      }
                        field++;
                    }
                               
                }
                          
            }
                    
                            
        }
                  
               gsl_vector* i_L=gsl_vector_alloc(edging_power_line_num);
               y_L_inv_inv=gsl_matrix_alloc(edging_power_line_num,edging_power_line_num);
               gsl_permutation * p2 = gsl_permutation_alloc(edging_power_line_num);
               int signum1;
               gsl_linalg_LU_decomp (y_L_inv, p2, &signum1);
               gsl_linalg_LU_invert (y_L_inv, p2, y_L_inv_inv);
               gsl_permutation_free(p2); 
               gsl_blas_dgemv (CblasNoTrans,1,y_L_inv_inv, second_part_first_phase_sum,0, i_L);
               
               
               gsl_vector* Ni_L=gsl_vector_alloc(switch_pi-1);
               gsl_blas_dgemv (CblasNoTrans,-1,&N1.matrix, i_L,0, Ni_L);
               gsl_vector_add(Ni_L,&I_1.vector);
               
               gsl_vector* U2=gsl_vector_alloc(switch_pi-1);
               for(int i=0;i<component_servers.size();i++){
                   map<string,Server>::iterator it;
                      
                   it=ps->getServers().find(component_servers[i].substr(0,component_servers[i].length()));
                   if(it==ps->getServers().end()){
                       cout<<"Server not found!"<<endl;
                   }
                   string host=it->second.host;
                   int port=it->second.port;
                   char *data=new char[SOCKET_BUFFER_SIZE];
                   bzero(data,SOCKET_BUFFER_SIZE);
                   std::ostringstream oss1(data);   
                   int server_start_index=servers_start_index[i];
                   int server_pis_num=servers_pis_num[i];
                        
                   if((server_start_index+server_pis_num)==switch_pi){
                          server_pis_num-=1;
                   }
                   oss1<<"Second Phase:"<<endl;
                   for(int k=server_start_index;k<server_start_index+server_pis_num;k++){
                       double data1=gsl_vector_get(Ni_L,k);
                       if(data1!=0)                    
                    oss1<<"Ni_L:"<<k-server_start_index<<":"<<data1<<endl;     
                   }
                   string result_second_phase=util::exec_on_server(host,port,oss1.str().c_str());
                       std::istringstream iss(result_second_phase);
                       string line;
                       int line_num=0;
                     while(getline(iss,line)){
                        if(line!=""){
                           gsl_vector_set(U2,server_start_index+line_num,atof(line.c_str()));
                        }
                         line_num++;
                     }
               }
                 
               
               gsl_matrix_view B2=gsl_matrix_submatrix(B,0,0,trans_power_line.size(),switch_pi-1);
 
                gsl_vector *I_b2=gsl_vector_alloc(trans_power_line.size());
                gsl_blas_dgemv(CblasNoTrans,1,&B2.matrix,U2,0,I_b2);
                //This I_b is actually branch voltage vector in website link tutorial,
                //since the branch resistor is 1, so the branch conductance matrix is identity matrix,
                // the multiplication result is the same.
                for(int i=0;i<trans_power_line.size();i++){
                    map<string,PowerLine>::iterator it;
                    it=ps->getPowerLines().find(trans_power_line[i]);
                    double resistor=it->second.resistor;
                    double current=gsl_vector_get(I_b2,i)/resistor;
                    if(current<0.0000001 && current>-0.0000001){
                        current=0;
                        
                    }
                    it->second.current=current;
                }
    
          gsl_vector_free(U2);
          gsl_vector_free(I_b2);
          gsl_vector_free(Ni_L);
          gsl_matrix_free(y_L_inv);
          gsl_vector_free(second_part_first_phase_sum);
    //      gsl_matrix_free(A);   
               
}
string ConnectedComponent::coordinator_ip(){
    if(coordinator_ip_!="localhost"){
        return coordinator_ip_;
    }
    else{
        return "default";
    }
}
void ConnectedComponent::set_coordinator_ip(string ip){
    coordinator_ip_=ip;
}
void ConnectedComponent::AddDynamicConnection(DynamicConnection dc){
     std::vector<DynamicConnection>::iterator it;
     if(dynamic_connections.size()==0){
       dynamic_connections.push_back(dc);   
     }
     else{
      it = dynamic_connections.begin();
      dynamic_connections.insert(it,dc);
     }
}
bool ConnectedComponent::is_dynamic(){
    return is_dynamic_;
}
void ConnectedComponent::set_is_dynamic(bool is_dynamic){
    is_dynamic_=is_dynamic;
}
void ConnectedComponent::DynamicSimulate(){
    cout<<"Dynamic Simulation begins!"<<endl;
    
    is_balanced=DynamicLoadBalancing();
       for(int i=0;i<switch_pi;i++){
                    map<int,string>::iterator it;
                    double current=0;
                    it=index_pi_mapping.find(i);
                    string power_interface=it->second;
                    map<string,PowerInterface>::iterator it3;
                    it3=ps->getPowerInterfaces().find(power_interface);
                    string node_id=it3->second.getNodeId();
                 
                    multimap<string,string>::iterator it1;
                    pair<multimap<string,string>::iterator,multimap<string,string>::iterator> ret;
                    ret = ps->get_adj_list().equal_range(power_interface);
                    for (it1=ret.first; it1!=ret.second; ++it1){
                        string neighbor=it1->second;
                        
                        map<string,PowerInterface>::iterator it2;
                        it2=ps->getPowerInterfaces().find(neighbor);
                        string neighbor_node_id=it2->second.getNodeId();
                        map<string,Node>::iterator it3;
                        it3=ps->getNodes().find(neighbor_node_id);
                        string role=it3->second.role;
                        
                        if(role =="WindTurbine" || role=="SolarPanel" || role=="PowerPlant"){  
                              current+=it3->second.current; 
                              gsl_vector_set(I,i,current);
                            }
                        if(role=="Demander"){
                              current-=it3->second.current;
                              gsl_vector_set(I,i,current);
                        }
                        if(role=="Storage"){
                            string op_mode=it3->second.operation_mode;
                            if(op_mode=="charging"){
                                current-=it3->second.current;
                                gsl_vector_set(I,i,current);
                            }else if(op_mode=="discharging"){
                                current+=it3->second.current; 
                                gsl_vector_set(I,i,current);
                            }else{
                                gsl_vector_set(I,i,0);
                            }
                        }
          
                    }
            }
    if(is_balanced){
        DynamicReconstruct();
        DynamicCalculation();
    }
   
}
void ConnectedComponent::DynamicReconstruct(){
    //This function modifies the existing A, B, I to enable dynamic calculation.
    // Process all the boundary nodes all at once.
    for(int i=0;i<dynamic_connections.size();i++){
       string pi=dynamic_connections[i].local_pi;
       int swap_index=switch_pi-boundary_set.size()-1;
       bool swap_needed=true;
    //if the power interface is already in the boundary set, then no swap is needed.
       for(int i=0;i<boundary_set.size();i++){
          if(pi==boundary_set[i]){
             swap_needed=false;
             break;
           }
       }
      if(swap_needed){
        vector<string>::iterator issss;
        if(boundary_set.size()==0){
          boundary_set.push_back(pi);   
        }
        else{
         issss=boundary_set.begin();
         boundary_set.insert(issss,pi);
        }
        map<string,int>::iterator it;
        it=pi_index_mapping.find(pi);
        int original_index=it->second;
        map<int,string>::iterator it1;
        it1=index_pi_mapping.find(swap_index);
        string swap_pi=it1->second;
   //the reordered sequence will make the distributed computation matrix not block diagonal anymore.
        pi_index_mapping[pi]=swap_index;
        index_pi_mapping[swap_index]=pi;
        pi_index_mapping[swap_pi]=original_index;
        index_pi_mapping[original_index]=swap_pi;

        //swap I
        double temp1=gsl_vector_get(I,original_index);
        double temp2=gsl_vector_get(I,swap_index);
        double temp=temp1;
        temp1=temp2;
        temp2=temp;
        gsl_vector_set(I,original_index,temp1);
        gsl_vector_set(I,swap_index,temp2);
        
        //swap A        
        gsl_matrix_swap_rows(A,original_index,swap_index);
        gsl_matrix_swap_columns(A,original_index,swap_index);
    
        
        //swap B
        gsl_matrix_swap_columns(B,original_index,swap_index);
       }
       else{
        //
       }
    }
    
}
void ConnectedComponent::DynamicCoordinatedLoadUpdate(string data){
    double demands;
    double supplies;
    string line;
    std::istringstream iss(data);
    getline(iss,line);
    string value;
    std::istringstream ssline(line);
    int flag;
    int field=0;
    while (getline ( ssline, value, ':' )){
        if(field==0)
            flag=atoi(value.c_str());
        if(field==1)
            demands=atof(value.c_str());
        if(field==2)
            supplies=atof(value.c_str());
        field++;
    }
   
      if(flag==1){
           
            for(int i=0;i<connected_component.size();i++){
                string power_interface=connected_component[i];
                map<string,PowerInterface>::iterator it;
                it=ps->getPowerInterfaces().find(power_interface);
                string node_id=it->second.getNodeId();
                map<string,Node>::iterator it1;
                it1=ps->getNodes().find(node_id);
                 double ratio;
                 double current;
                 string role=it1->second.role;
                if(role=="WindTurbine" || role=="SolarPanel" || role=="PowerPlant" ){
                    ratio=(it1->second.desired_current)/desired_supplies;
                    current=supplies*ratio;
                    ps->update_appliance(it1->second.id,current);

                }else if(role=="Demander"){
                      ps->update_appliance(it1->second.id,it1->second.desired_current);
                }else if(role=="Storage"){
                    string op_mode=it1->second.operation_mode;
                    if(op_mode=="charging"){
                        ps->update_appliance(it1->second.id,it1->second.desired_current);
                    }else if(op_mode=="discharging"){
                        ratio=(it1->second.desired_current)/desired_supplies;
                        current=supplies*ratio;
                        ps->update_appliance(it1->second.id,current);
                    }else{
                        ps->update_appliance(it1->second.id,0);
                    }
                    
                }
            }
    }
    else{
            
            
            for(int i=0;i<connected_component.size();i++){
                string power_interface=connected_component[i];
                map<string,PowerInterface>::iterator it;
                it=ps->getPowerInterfaces().find(power_interface);
                string node_id=it->second.getNodeId();
                map<string,Node>::iterator it1;
                it1=ps->getNodes().find(node_id);
                double ratio;
                double current;
                string role=it1->second.role;
                if(role=="Demander"){
                    ratio=(it1->second.desired_current)/desired_demands;
                    current=demands*ratio;
                    ps->update_appliance(it1->second.id,current);

                }else if(role=="WindTurbine" || role=="SolarPanel" || role=="PowerPlant"){
                    ps->update_appliance(it1->second.id,it1->second.desired_current);
                }else if(role=="Storage"){
                    string op_mode=it1->second.operation_mode;
                    if(op_mode=="charging"){
                        ratio=(it1->second.desired_current)/desired_demands;
                        current=demands*ratio;
                        ps->update_appliance(it1->second.id,current);
                    }else if(op_mode=="discharging"){
                       ps->update_appliance(it1->second.id,it1->second.desired_current);  
                    }else{
                         ps->update_appliance(it1->second.id,0);
                    }
                }

            }
            

        }
      int index=0; 
      int boundary_node_num=boundary_set.size();
      
          for(int i=0;i<switch_pi;i++){
                    map<int,string>::iterator it;
                    double current=0;
                    it=index_pi_mapping.find(i);
                    string power_interface=it->second;
                    map<string,PowerInterface>::iterator it3;
                    it3=ps->getPowerInterfaces().find(power_interface);
                    string node_id=it3->second.getNodeId();
                 
                    multimap<string,string>::iterator it1;
                    pair<multimap<string,string>::iterator,multimap<string,string>::iterator> ret;
                    ret = ps->get_adj_list().equal_range(power_interface);
                    for (it1=ret.first; it1!=ret.second; ++it1){
                        string neighbor=it1->second;
                        
                        map<string,PowerInterface>::iterator it2;
                        it2=ps->getPowerInterfaces().find(neighbor);
                        string neighbor_node_id=it2->second.getNodeId();
                        map<string,Node>::iterator it3;
                        it3=ps->getNodes().find(neighbor_node_id);
                        string role=it3->second.role;
                        
                        if(role =="WindTurbine" || role=="SolarPanel" || role=="PowerPlant"){  
                              current+=it3->second.current; 
                              gsl_vector_set(I,i,current);
                            }
                        if(role=="Demander"){
                              current-=it3->second.current;
                              gsl_vector_set(I,i,current);
                        }
                        if(role=="Storage"){
                            string op_mode=it3->second.operation_mode;
                            if(op_mode=="charging"){
                                current-=it3->second.current;
                                gsl_vector_set(I,i,current);
                            }else if(op_mode=="discharging"){
                                current+=it3->second.current; 
                                gsl_vector_set(I,i,current);
                            }else{
                                gsl_vector_set(I,i,0);
                            }
                        }
          
                    }
            }
    
          gsl_vector_view I1=gsl_vector_subvector(I,0,switch_pi-boundary_node_num);
          cout<<"I1 vector value:"<<endl;
          for(int i=0;i<switch_pi-boundary_node_num;i++){
              cout<<gsl_vector_get(I,i)<<endl;
          }
                gsl_vector* deltaY=gsl_vector_alloc(boundary_node_num); 
                gsl_vector_set_zero(deltaY);
                gsl_blas_dgemv(CblasNoTrans,1,AB1_A1_inv,&I1.vector,0,deltaY);
                
                
             cout<<"updated vector deltaY:"<<endl;
                for(int i=0;i<boundary_node_num;i++){
                    double current=gsl_vector_get(deltaY,i);
                    if(current<0.0000001 && current >-0.000001)
                        gsl_vector_set(deltaY,i,0);
                    cout<<gsl_vector_get(deltaY,i)<<endl;
                }
                 string host=coordinator_ip_;
                 int port = MASTER_PORT;
                 char *data1=new char[1024];
                 bzero(data1,1024);
                 std::ostringstream oss(data1);
                 oss<<"coordinating info updated:"<<endl;
                 //assumes one boundary node with one remote machine connected.
                 oss<<ps->getLocalIp()<<":"<<boundary_node_num<<endl;
         
                 for(int i=0;i<boundary_node_num;i++){
                         oss<<gsl_vector_get(deltaY,i)<<endl;
                 }
                 
                 string result=util::exec_on_server(host,port,oss.str().c_str());
                 cout<<"coordinated VB not used:"<<result<<endl;
//                 if(1){
//                 gsl_vector* Usss=gsl_vector_alloc(switch_pi);
//                 gsl_vector_set_zero(Usss);
//                 gsl_vector* VB=gsl_vector_alloc(boundary_node_num);
//                 string line;
//                 std::istringstream iss(result);
//                 int field=0;
//                 
//                 
//                 while(getline(iss,line)){
//                     if(line!=""){
//                         gsl_vector_set(VB,field,atof(line.c_str()));
//                         gsl_vector_set(Usss,switch_pi-boundary_node_num+field,atof(line.c_str()));
//                     }
//                     field++;
//                 }
//                 gsl_vector* A1B_VB=gsl_vector_alloc(switch_pi-boundary_node_num);
//                 gsl_blas_dgemv(CblasNoTrans,1,A1B,VB,0,A1B_VB);
//                 
//                 
//                 gsl_vector* I_temp=gsl_vector_alloc(switch_pi-boundary_node_num);
//                 gsl_vector_memcpy (I_temp, &I1.vector);
//                 
//                 
//                 gsl_vector_sub (I_temp, A1B_VB);
//                 gsl_vector* V_A=gsl_vector_alloc(switch_pi-boundary_node_num);
//                 gsl_blas_dgemv(CblasNoTrans,1,A1_inv,I_temp,0,V_A);
//                 for(int i=0;i<switch_pi-boundary_node_num;i++){
//                     gsl_vector_set(Usss,i,gsl_vector_get(V_A,i));
//                 }                
//                 
//                 cout<<"Final Usss:"<<endl;
//                 for(int i=0;i<switch_pi;i++)
//                     cout<<gsl_vector_get(Usss,i)<<endl;
//                
//               
//                gsl_matrix_view B1=gsl_matrix_submatrix(B,0,0,trans_power_line.size(),switch_pi);
// 
//                gsl_vector *I_b=gsl_vector_alloc(trans_power_line.size());
//                gsl_blas_dgemv(CblasNoTrans,1,&B1.matrix,Usss,0,I_b);
//                
//                
//                
//                
//                
//                //This I_b is actually branch voltage vector in website link tutorial,
//                //since the branch resistor is 1, so the branch conductance matrix is identity matrix,
//                // the multiplication result is the same.
//                for(int i=0;i<trans_power_line.size();i++){
//                    map<string,PowerLine>::iterator it;
//                    it=ps->getPowerLines().find(trans_power_line[i]);
//                    double resistor=it->second.resistor;
//                    double current=gsl_vector_get(I_b,i)/resistor;
//                    if(current<0.0000001 && current>-0.0000001){
//                        current=0;
//                        
//                    }
//                    it->second.current=current;
//                }
//                gsl_vector_free(I_temp);
//                   gsl_vector_free(I_b); 
//                   gsl_vector_free(V_A);
//                   gsl_vector_free(A1B_VB);
//                   gsl_vector_free(VB);
                   gsl_vector_free(deltaY);
//                   gsl_vector_free(Usss);       
//                 }
}
void ConnectedComponent::DynamicCoordinatedBoundaryUpdate(string data){
    int boundary_node_num=boundary_set.size();
    gsl_vector* Usss=gsl_vector_alloc(switch_pi);
    gsl_vector_set_zero(Usss);
    if(boundary_node_num==0)
        return;
    gsl_vector* VB=gsl_vector_alloc(boundary_node_num);
    int index=0;
    
    string line;
    string value;
    std::istringstream iss(data);
    for(int i=0;i<boundary_node_num;i++){
           getline(iss,line);
           std::istringstream ssline(line);
   
           int field=0;
           while (getline ( ssline, value, ':' )){
               if(field==0){
                  gsl_vector_set(VB,i,atof(value.c_str()));
                  gsl_vector_set(Usss,switch_pi-boundary_node_num+i,atof(value.c_str()));
               }
               if(field==1){
                   double current=atof(value.c_str());
                   map<int,string>::iterator it1;
                   it1=index_pi_mapping.find(switch_pi-boundary_node_num+i);
                   string pi=it1->second;
                   string power_line=ps->find_tunnel_power_line(pi);
                   map<string,PowerLine>::iterator it;
                   it=ps->getPowerLines().find(power_line);
                   it->second.current=current;
                   
                   // This is the place to send interface module in testbed.
                    if(isTestbed==1){
                              int send_sock;
                              struct hostent *host1;
                              struct sockaddr_in server_addr1;
                              char datastd[50];
                              bzero(datastd,50);
                              char ts[10];
                              string ss="Update session:";
                              sprintf(ts,"%d:",18);
                              strcat(datastd,ss.c_str());
                              strcat(datastd,ts);
        
                              char settings[10];                   
                              if(current >0){
                                   int ss=int(fabs(current))+32768;
                                   sprintf(settings,"%d",ss);
                              }
                              else{
                                 sprintf(settings,"%d",(int)fabs(current+1));
                              }
            
                              strcat(datastd,settings);
        
                              strcat(datastd,"\r\n");
                              printf("%s,$$,%s",interfaceIp.c_str(),datastd);
        
                              host1 = gethostbyname(interfaceIp.c_str());
                            if ((send_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                                 printf("Socket creation failed! ");
                                 continue;   
                            }

                            server_addr1.sin_family = AF_INET;
                              // the server port on java sgmonitor
                            server_addr1.sin_port = htons(9009);
                            server_addr1.sin_addr = *((struct in_addr *)host1->h_addr);
                            bzero(&(server_addr1.sin_zero),8);

                           if (connect(send_sock, (struct sockaddr *)&server_addr1,
                              sizeof(struct sockaddr)) == -1)
                           {
                              printf("Connect failed!!! Beagleboard may not be booted.\n");
                            
                           }
                          send(send_sock,datastd,50, 0); 
                          printf("sent! %d\n",i);
     
                           close(send_sock); 
                         
                         }
                   
                   
                   
                   
                   
                   
               }
                   
               field++;
           }
        
        
        
    }
    
                  gsl_vector_view I1=gsl_vector_subvector(I,0,switch_pi-boundary_node_num);
                 gsl_vector* A1B_VB=gsl_vector_alloc(switch_pi-boundary_node_num);
                 gsl_blas_dgemv(CblasNoTrans,1,A1B,VB,0,A1B_VB);
                 
                 gsl_vector* I_temp=gsl_vector_alloc(switch_pi-boundary_node_num);
                 gsl_vector_memcpy (I_temp, &I1.vector);
                 
                 
                 gsl_vector_sub (I_temp, A1B_VB);
                 gsl_vector* V_A=gsl_vector_alloc(switch_pi-boundary_node_num);
                 gsl_blas_dgemv(CblasNoTrans,1,A1_inv,I_temp,0,V_A);
                 for(int i=0;i<switch_pi-boundary_node_num;i++){
                     gsl_vector_set(Usss,i,gsl_vector_get(V_A,i));
                 }                
                 
                 cout<<"Final Usss:"<<endl;
                 for(int i=0;i<switch_pi;i++)
                     cout<<gsl_vector_get(Usss,i)<<endl;
                
               
                gsl_matrix_view B1=gsl_matrix_submatrix(B,0,0,trans_power_line.size(),switch_pi);
 
                gsl_vector *I_b=gsl_vector_alloc(trans_power_line.size());
                gsl_blas_dgemv(CblasNoTrans,1,&B1.matrix,Usss,0,I_b);
                
                
                
                
                
                //This I_b is actually branch voltage vector in website link tutorial,
                //since the branch resistor is 1, so the branch conductance matrix is identity matrix,
                // the multiplication result is the same.
                for(int i=0;i<trans_power_line.size();i++){
                    map<string,PowerLine>::iterator it;
                    it=ps->getPowerLines().find(trans_power_line[i]);
                    double resistor=it->second.resistor;
                    double current=gsl_vector_get(I_b,i)/resistor;
                    if(current<0.0000001 && current>-0.0000001){
                        current=0;
                        
                    }
                    it->second.current=current;
                }
                   gsl_vector_free(I_b); 
                   gsl_vector_free(V_A);
                   gsl_vector_free(A1B_VB);
                   gsl_vector_free(VB);
                   gsl_vector_free(I_temp);
                   gsl_vector_free(Usss);       
                
    
    
    
}
void ConnectedComponent::DynamicUpdate(){
    cout<<"Dynamic Update Process!"<<endl;
    is_balanced=DynamicLoadBalancing();
    if(is_balanced){
       int index=0; 
      int boundary_node_num=boundary_set.size();
      
          for(int i=0;i<switch_pi;i++){
                    map<int,string>::iterator it;
                    double current=0;
                    it=index_pi_mapping.find(i);
                    string power_interface=it->second;
                    map<string,PowerInterface>::iterator it3;
                    it3=ps->getPowerInterfaces().find(power_interface);
                    string node_id=it3->second.getNodeId();
                 
                    multimap<string,string>::iterator it1;
                    pair<multimap<string,string>::iterator,multimap<string,string>::iterator> ret;
                    ret = ps->get_adj_list().equal_range(power_interface);
                    for (it1=ret.first; it1!=ret.second; ++it1){
                        string neighbor=it1->second;
                        
                        map<string,PowerInterface>::iterator it2;
                        it2=ps->getPowerInterfaces().find(neighbor);
                        string neighbor_node_id=it2->second.getNodeId();
                        map<string,Node>::iterator it3;
                        it3=ps->getNodes().find(neighbor_node_id);
                        string role=it3->second.role;
                        
                        if(role =="WindTurbine" || role=="SolarPanel" || role=="PowerPlant"){  
                              current+=it3->second.current; 
                              gsl_vector_set(I,i,current);
                            }
                        if(role=="Demander"){
                              current-=it3->second.current;
                              gsl_vector_set(I,i,current);
                        }
                        if(role=="Storage"){
                            string op_mode=it3->second.operation_mode;
                            if(op_mode=="charging"){
                                current-=it3->second.current;
                                gsl_vector_set(I,i,current);
                            }else if(op_mode=="discharging"){
                                current+=it3->second.current; 
                                gsl_vector_set(I,i,current);
                            }else{
                                gsl_vector_set(I,i,0);
                            }
                        }
          
                    }
            }
    
          gsl_vector_view I1=gsl_vector_subvector(I,0,switch_pi-boundary_node_num);
                gsl_vector* deltaY=gsl_vector_alloc(boundary_node_num); 
                gsl_blas_dgemv(CblasNoTrans,1,AB1_A1_inv,&I1.vector,0,deltaY);
                
                
             cout<<"updated vector deltaY:"<<endl;
                for(int i=0;i<boundary_node_num;i++){
                    double current=gsl_vector_get(deltaY,i);
                    if(current<0.0000001 && current >-0.000001)
                        gsl_vector_set(deltaY,i,0);
                    cout<<gsl_vector_get(deltaY,i)<<endl;
                }
                 string host=coordinator_ip_;
                 int port = MASTER_PORT;
                 char *data1=new char[1024];
                 bzero(data1,1024);
                 std::ostringstream oss(data1);
                 oss<<"coordinating info updated:"<<endl;
                 //assumes one boundary node with one remote machine connected.
                 oss<<ps->getLocalIp()<<":"<<boundary_node_num<<endl;
         
                 for(int i=0;i<boundary_node_num;i++){
                         oss<<gsl_vector_get(deltaY,i)<<endl;
                 }
                 
                 string result=util::exec_on_server(host,port,oss.str().c_str());
                 cout<<"coordinated VB not used:"<<result<<endl;
//                 if(1){
//                 gsl_vector* Usss=gsl_vector_alloc(switch_pi);
//                 gsl_vector_set_zero(Usss);
//                 gsl_vector* VB=gsl_vector_alloc(boundary_node_num);
//                 string line;
//                 std::istringstream iss(result);
//                 int field=0;
//                 
//                 
//                 while(getline(iss,line)){
//                     if(line!=""){
//                         gsl_vector_set(VB,field,atof(line.c_str()));
//                         gsl_vector_set(Usss,switch_pi-boundary_node_num+field,atof(line.c_str()));
//                     }
//                     field++;
//                 }
//                 gsl_vector* A1B_VB=gsl_vector_alloc(switch_pi-boundary_node_num);
//                 gsl_blas_dgemv(CblasNoTrans,1,A1B,VB,0,A1B_VB);
//                 
//                 gsl_vector* I_temp=gsl_vector_alloc(switch_pi-boundary_node_num);
//                 gsl_vector_memcpy (I_temp, &I1.vector);
//                 
//                 gsl_vector_sub (I_temp, A1B_VB);
//                 gsl_vector* V_A=gsl_vector_alloc(switch_pi-boundary_node_num);
//                 gsl_blas_dgemv(CblasNoTrans,1,A1_inv,I_temp,0,V_A);
//                 for(int i=0;i<switch_pi-boundary_node_num;i++){
//                     gsl_vector_set(Usss,i,gsl_vector_get(V_A,i));
//                 }                
//                 
//                 cout<<"Final Usss:"<<endl;
//                 for(int i=0;i<switch_pi;i++)
//                     cout<<gsl_vector_get(Usss,i)<<endl;
//                
//               
//                gsl_matrix_view B1=gsl_matrix_submatrix(B,0,0,trans_power_line.size(),switch_pi);
// 
//                gsl_vector *I_b=gsl_vector_alloc(trans_power_line.size());
//                gsl_blas_dgemv(CblasNoTrans,1,&B1.matrix,Usss,0,I_b);
//                
//                
//                
//                
//                
//                //This I_b is actually branch voltage vector in website link tutorial,
//                //since the branch resistor is 1, so the branch conductance matrix is identity matrix,
//                // the multiplication result is the same.
//                for(int i=0;i<trans_power_line.size();i++){
//                    map<string,PowerLine>::iterator it;
//                    it=ps->getPowerLines().find(trans_power_line[i]);
//                    double resistor=it->second.resistor;
//                    double current=gsl_vector_get(I_b,i)/resistor;
//                    if(current<0.0000001 && current>-0.0000001){
//                        current=0;
//                        
//                    }
//                    it->second.current=current;
//                }
//                   gsl_vector_free(I_b); 
//                   gsl_vector_free(V_A);
//                   gsl_vector_free(A1B_VB);
//                   gsl_vector_free(VB);
                   gsl_vector_free(deltaY);
//                   gsl_vector_free(Usss);   
//                   gsl_vector_free(I_temp);
//                 }
    }  
    
}
void ConnectedComponent::restore(){
    // assume one connection and remove all of them
    RemoveDynamicConnection(); // should add a parameter later.
    //clear boundary set and set power line 0
    for(int i=0;i<boundary_set.size();i++) {
          string power_line=ps->find_tunnel_power_line(boundary_set[i]);
          map<string,PowerLine>::iterator it;
          it=ps->getPowerLines().find(power_line);
          it->second.current=0;
      // This is the place to set interface module
           if(isTestbed==1){
                              int send_sock;
                              struct hostent *host1;
                              struct sockaddr_in server_addr1;
                              char datastd[50];
                              bzero(datastd,50);
                              char ts[10];
                              string ss="Update session:";
                              sprintf(ts,"%d:",18);
                              strcat(datastd,ss.c_str());
                              strcat(datastd,ts);
                              double current=0;
                              current=1;
                              char settings[10];                   
                            
                                 sprintf(settings,"%d",(int)current);
                              
            
                              strcat(datastd,settings);
        
                              strcat(datastd,"\r\n");
                              printf("%s,$$,%s",interfaceIp.c_str(),datastd);
        
                              host1 = gethostbyname(interfaceIp.c_str());
                            if ((send_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                                 printf("Socket creation failed! ");
                                 continue;   
                            }

                            server_addr1.sin_family = AF_INET;
                              // the server port on java sgmonitor
                            server_addr1.sin_port = htons(9009);
                            server_addr1.sin_addr = *((struct in_addr *)host1->h_addr);
                            bzero(&(server_addr1.sin_zero),8);

                           if (connect(send_sock, (struct sockaddr *)&server_addr1,
                              sizeof(struct sockaddr)) == -1)
                           {
                              printf("Connect failed!!! Beagleboard may not be booted.\n");
                            
                           }
                          send(send_sock,datastd,50, 0); 
                          printf("sent! %d\n",i);
     
                           close(send_sock); 
                         
                         }
    }
    boundary_set.clear();
    
    set_is_dynamic(false);
    set_coordinator_ip("localhost");
    gsl_matrix_free(A1_inv);
    gsl_matrix_free(A1B);
    gsl_matrix_free(AB1_A1_inv);
    ReturnSimulate();
   
}
void ConnectedComponent::RemoveDynamicConnection(){
    dynamic_connections.clear();
    
}
void ConnectedComponent::ReturnSimulate(){
     bool is_balanced=load_balancing(); 
    if(is_balanced){
      if(switch_pi<=1)
            return; 
      for(int i=0;i<switch_pi;i++){
            map<int,string>::iterator it;
            double current=0;
            it=index_pi_mapping.find(i);
            string power_interface=it->second;
            map<string,PowerInterface>::iterator it3;
            it3=ps->getPowerInterfaces().find(power_interface);
            string node_id=it3->second.getNodeId();
                 
            multimap<string,string>::iterator it1;
            pair<multimap<string,string>::iterator,multimap<string,string>::iterator> ret;
            ret = ps->get_adj_list().equal_range(power_interface);
            for (it1=ret.first; it1!=ret.second; ++it1){
            string neighbor=it1->second;
                        
            map<string,PowerInterface>::iterator it2;
            it2=ps->getPowerInterfaces().find(neighbor);
            string neighbor_node_id=it2->second.getNodeId();
            map<string,Node>::iterator it3;
            it3=ps->getNodes().find(neighbor_node_id);
            string role=it3->second.role;
                        
            if(role =="WindTurbine" || role=="SolarPanel" || role=="PowerPlant"){  
                    current+=it3->second.current; 
                    gsl_vector_set(I,i,current);
            }
            if(role=="Demander"){
                    current-=it3->second.current;
                    gsl_vector_set(I,i,current);
            }
            if(role=="Storage"){
                string op_mode=it3->second.operation_mode;
                if(op_mode=="charging"){
                    current-=it3->second.current;
                    gsl_vector_set(I,i,current);
                }else if(op_mode=="discharging"){
                                current+=it3->second.current; 
                                gsl_vector_set(I,i,current);
                            }else{
                                gsl_vector_set(I,i,0);
                            }
                        }
          
                    }
            } 
      
                gsl_vector* U=gsl_vector_alloc(switch_pi-1);
                gsl_vector_view I1=gsl_vector_subvector(I,0,switch_pi-1);
                
                // the A matrix could be reconstructed, so the A_inv is changed also.
                gsl_matrix_free(A_inv);
                gsl_matrix* A_temp=gsl_matrix_alloc(switch_pi,switch_pi);
                gsl_matrix_memcpy(A_temp,A);
                A_inv=gsl_matrix_alloc(switch_pi-1,switch_pi-1);
                gsl_matrix_set_zero(A_inv);
                gsl_matrix_view A1=gsl_matrix_submatrix(A_temp,0,0,switch_pi-1,switch_pi-1);
                gsl_permutation * p = gsl_permutation_alloc(switch_pi-1);
                int signum;
                
                //this decomposition process has changed matrix A.
                gsl_linalg_LU_decomp (&A1.matrix, p, &signum);
                gsl_linalg_LU_invert (&A1.matrix, p, A_inv);
                gsl_permutation_free(p);  
                
                gsl_blas_dgemv(CblasNoTrans,1,A_inv,&I1.vector,0,U);
                
                for(int i=0;i<switch_pi-1;i++){
                    if(gsl_vector_get(U,i)<0.00001 && gsl_vector_get(U,i)>-0.0001){
                       gsl_vector_set(U,i,0);
                    }
                }
                
                //if there is no matrix B and no trans_power_line for that connected component
                int trans_power_line_num=trans_power_line.size();
                if(trans_power_line_num<=0){
                    return;
                }
                gsl_matrix_view B1=gsl_matrix_submatrix(B,0,0,trans_power_line.size(),switch_pi-1);
 
                gsl_vector *I_b=gsl_vector_alloc(trans_power_line.size());
                gsl_blas_dgemv(CblasNoTrans,1,&B1.matrix,U,0,I_b);
                for(int i=0;i<trans_power_line.size();i++){
                    map<string,PowerLine>::iterator it;
                    it=ps->getPowerLines().find(trans_power_line[i]);
                    double resistor=it->second.resistor;
                    double current=gsl_vector_get(I_b,i)/resistor;
                    if(current<0.0000001 && current>-0.0000001){
                        current=0;
                        
                    }
                    it->second.current=current;
                }
                  gsl_vector_free(U);
                  gsl_vector_free(I_b);
                  gsl_matrix_free(A_temp);
      
    }
}
