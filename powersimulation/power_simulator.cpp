#include "power_simulator.h"
#include "connected_component.h"
#include <stdio.h>      
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
PowerSimulator::PowerSimulator(){
        struct ifaddrs * ifAddrStruct=NULL;
          struct ifaddrs * ifa=NULL;
          void * tmpAddrPtr=NULL;

          getifaddrs(&ifAddrStruct);

         for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
             if (ifa ->ifa_addr->sa_family==AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
           //  printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer); 
             if(strcmp(ifa->ifa_name,"wlan0")==0 || strcmp(ifa->ifa_name,"eth0")==0
                     || strcmp(ifa->ifa_name,"wlan1")==0 || strcmp(ifa->ifa_name,"eth1")==0)
               localIp=string(addressBuffer);  
          } 
         }
        if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
	gui_client_ip="localhost";
        
}
void PowerSimulator::setExecMode(int mode){
    exec_mode=mode;
}
int PowerSimulator::getExecMode(){
    return exec_mode;
}
map<string,PowerLine>& PowerSimulator::getPowerLines(){
    return powerLines;   
}


map<string,Node>& PowerSimulator::getNodes(){
    return nodes;
}


void PowerSimulator::parse(){
    string data(input);
    string line;
    std::istringstream iss(data);
    getline(iss,line);
    string value;
    std::istringstream ssline(line);
    int field=0;
    while (getline ( ssline, value, ':' )){
         if(field==0){
                 order=atoi(value.c_str());
          }
         if(field==1){
                 size=atoi(value.c_str());
         }
         if(field==2){
             numberOfServer=atoi(value.c_str());  
         }
             field++;
    }
    for(int i=0;i<order+size+numberOfServer;i++){
        getline(iss,line);
        if(line != ""){
        std::istringstream ssline(line);
        field=0;
        if(i<order){
            Node temp;
            while(getline(ssline,value,':')){
                switch (field){
                    case 1: temp.id=value;break;
                    case 2: temp.role=value;break;
                    case 3: temp.desired_current=atof(value.c_str());temp.current=0;break;
                    case 4: temp.max_current=atof(value.c_str());break;
                    case 5: {
                               std::istringstream t(value); 
                                 string t2;
                               while(getline(t,t2,' ')){
                                 string pi_id;
                                 PowerInterface temp_pi(temp.id,atoi(t2.c_str()));
                                 pi_id=temp_pi.getId();
                                 powerInterfaces[pi_id]=temp_pi;
                                 temp.power_interfaces.push_back(pi_id);
                              
                               }
                               temp.power_interface_num=temp.power_interfaces.size();  
                            }break;
                    case 6: temp.power_interface_flag=atoi(value.c_str());break;
                    case 7: temp.location=value.erase(value.find_last_not_of(" \n\r\t")+1); break; 
                    case 8: temp.e_stored=atof(value.c_str());break;
                    case 9: temp.capacity=atof(value.c_str());break;
                    case 10: temp.operation_mode=value.erase(value.find_last_not_of(" \n\r\t")+1); break; 
                }
                
                
                field++;
            }
            nodes[temp.id]=temp;
            
        }
        else if (i>=order && i<order+size){
            PowerLine temp;
            while(getline(ssline,value,':')){
                switch (field){
                    case 1: temp.id=value;break;
                    case 2: temp.resistor=atof(value.c_str());break;
                    case 3: temp.lpeer=value;break;
                    case 4: temp.rpeer=value;break;
                    case 5: std::istringstream s(value); string s2;int s1=0;
                            while(getline(s,s2,' ')){
                                if(s1==0){
                                    temp.lpeerIf=atoi(s2.c_str());
                                    temp.lpeerPI=temp.lpeer+s2;
                                }
                                if(s1==1){
                                    temp.rpeerIf=atoi(s2.c_str());
                                     char indexStr[5];
                                     sprintf(indexStr,"%d",temp.rpeerIf);
                                     string indexS(indexStr);
                                     string ss=temp.rpeer+indexS;
                                    temp.rpeerPI=ss;
                                }
                                s1++;
                               
                            }
                            break;
                     
                }
                
                
                field++;
            }
            temp.current=0;
            powerLines[temp.id]=temp;
            
        }
        else{
            Server temp;
            while(getline(ssline,value,':')){
                switch (field){
                    case 1: temp.serverName=value;break;
                    case 2: temp.host=value;break;
                    case 3: temp.port=atoi(value.c_str());break;
                }
                
                
                field++;
            }
            servers[temp.serverName]=temp;
            
        }
      }
    }
    create_adj_list();
    if(!adj_list.empty())
    find_connected_components();
}


void PowerSimulator::create_adj_list(){
        map<string,PowerInterface>::iterator it;
    for(it=powerInterfaces.begin();it!=powerInterfaces.end();it++){
        //from node
        string power_interface=it->first;
        string nodeId=it->second.getNodeId();
        map<string,Node>::iterator it1;
        it1=nodes.find(nodeId);
        vector<string>neighbors;
        it1->second.pi_neighbors(power_interface,neighbors);
        for(int i=0;i<neighbors.size();i++){
            adj_list.insert(pair<string,string>(power_interface,neighbors[i]));
        }
        
        
        //from power line
        map<string,PowerLine>::iterator it2;
        for(it2=powerLines.begin();it2!=powerLines.end();it2++){
            string s=it2->second.otherPeer(power_interface);
            if(!s.empty())
                adj_list.insert(pair<string,string>(power_interface,s));
        }
             
    }
    
}


void PowerSimulator::dfs_visit(string power_interface){
    map<string,PowerInterface>::iterator it;
    it=powerInterfaces.find(power_interface);
    it->second.setColor("grey");
    multimap<string,string>::iterator it1;
    pair<multimap<string,string>::iterator,multimap<string,string>::iterator> ret;
    ret=adj_list.equal_range(power_interface);
     for (it1=ret.first; it1!=ret.second; ++it1){
         string neighbor=it1->second;
         map<string,PowerInterface>::iterator it2;
         it2=powerInterfaces.find(neighbor);
         string color=it2->second.getColor();
         if(color.compare("white")==0)
             PowerSimulator::dfs_visit(neighbor);
     
     }
    it->second.setColor("black");
}


void PowerSimulator::find_connected_components(){
    map<string,PowerInterface>::iterator it;
        for(it=powerInterfaces.begin();it!=powerInterfaces.end();it++){
        it->second.setColor("white");
    }
    for(it=powerInterfaces.begin();it!=powerInterfaces.end();it++){
        string color=it->second.getColor();
        vector<string> connected_component;
        if(color.compare("white")==0){
            dfs_visit(it->first);
        }
        map<string,PowerInterface>::iterator it1;
        for(it1=powerInterfaces.begin();it1!=powerInterfaces.end();it1++){
            if(it1->second.getColor().compare("black")==0){
                connected_component.push_back(it1->first);
                it1->second.setColor("green");
            }
        }
        // if not null
        if(!connected_component.empty()){    
           connected_components.push_back(connected_component);
        }
    }
    
}
// All the resistors of the power lines are hard coded as 1.
string PowerSimulator::simulate(){
 
    for(int i=0;i<connected_components.size();i++){
    //    simulate_component(connected_components[i]);   
        ConnectedComponent* c=new ConnectedComponent(this,connected_components[i]);
        c->simulate();
        Cs.push_back(c);
    }
    string result="s";
    return result;    
}
//update energy of appliance and the connected power line.
bool PowerSimulator::update_appliance(string node_id,double new_current){
    
        bool is_success=false;  
        map<string,Node>::iterator it;
        it=nodes.find(node_id);
        string role=it->second.role;
        if(role=="Demander" || role =="WindTurbine" || 
                role=="SolarPanel" || role=="PowerPlant" || role=="Storage"){
            it->second.current=new_current;
    // update corresponding app power line
           string power_line=find_app_power_line(node_id);
           if(!power_line.empty()){
             map<string,PowerLine>::iterator it1;
             it1=powerLines.find(power_line);
             it1->second.current=new_current;
             is_success=true;
           }
        }
        return is_success;
}
string PowerSimulator::find_app_power_line(string app_node_id){
    map<string,Node>::iterator it;
    string power_line;
    string power_interface;
    it=nodes.find(app_node_id);
    string role=it->second.role;
    if(role=="Demander" || role =="WindTurbine" || role=="SolarPanel" || role=="PowerPlant" || role=="Storage"){
         power_interface=it->second.power_interfaces[0];
    }
    map<string,PowerLine>::iterator it1;
    for(it1=powerLines.begin();it1!=powerLines.end();it1++){
        bool is_peer=it1->second.isPeer(power_interface);
        if(is_peer)
            power_line=it1->first;
    }
    return power_line;
        
    
}
//find power line id based on two power interfaces id;
string PowerSimulator::find_power_line(string peer1, string peer2){
    map<string,PowerLine>::iterator it;
    string power_line;
    for(it=powerLines.begin();it!=powerLines.end();it++){
        if(it->second.isPeer(peer1)&& it->second.isPeer(peer2))
            power_line=it->first;
    }
    
    return power_line;
}
string PowerSimulator::update(string data){ 
    string result; 
    int field=0;
    string fields[10];//may exceed 10 fields later.
    string value;
    istringstream ssline(data);
    while (getline ( ssline, value, ':' )){
         fields[field++]=value;
    }
    string node_id=fields[0];
    string node_update=fields[1];
    map<string,Node>::iterator it;
    it=nodes.find(node_id);
    string role=it->second.role;
    if(role=="Demander"){
        double request_current=atof(node_update.c_str());
        if(request_current>it->second.max_current){
            //cannot exceed pre-configured maximum current
            request_current=it->second.max_current;
        }
        it->second.desired_current=request_current;
        string power_interface=node_id+"0";
        update_appliance_desired_current(node_id,request_current);
        int index=find_component(power_interface);
        if(Cs[index]->is_dynamic()){
            Cs[index]->DynamicUpdate();
        }else{
           Cs[index]->Update();
        }
        result="s";
        
    }
        //Supplier will update its maximum output
    if(role =="WindTurbine" || role=="SolarPanel" || role=="PowerPlant"){
        double request_current=atof(node_update.c_str());
        it->second.max_current=request_current;
        string power_interface=node_id+"0";
        update_appliance_desired_current(node_id,request_current);
        int index=find_component(power_interface);
        if(Cs[index]->is_dynamic()){
            Cs[index]->DynamicUpdate();
        }else{
        Cs[index]->Update();
        }
        result="s";
    }
    if(role=="Storage"){
        node_update=node_update.erase(value.find_last_not_of(" \n\r\t")+1);
        if(node_update=="charging" || node_update=="discharging" || node_update=="standing"){
            it->second.operation_mode=node_update;
            string power_interface=node_id+"0";
            int index=find_component(power_interface);
          if(Cs[index]->is_dynamic()){
            Cs[index]->DynamicUpdate();
          }else{
            Cs[index]->Update();
          }
        }
        else{
            double request_current=atof(node_update.c_str());
            if(it->second.operation_mode=="charging"){
                  if(request_current>it->second.max_current){
            //cannot exceed pre-configured maximum current
                    request_current=it->second.max_current;
                   }
                  it->second.desired_current=request_current;
                  string power_interface=node_id+"0";
                  update_appliance_desired_current(node_id,request_current);
                  int index=find_component(power_interface);
                if(Cs[index]->is_dynamic()){
                    Cs[index]->DynamicUpdate();
                }else{
                  Cs[index]->Update();    
                }
            }else if(it->second.operation_mode=="discharging"){
                it->second.max_current=request_current;
                string power_interface=node_id+"0";
                update_appliance_desired_current(node_id,request_current);
                int index=find_component(power_interface);
                if(Cs[index]->is_dynamic()){
                 Cs[index]->DynamicUpdate();
                }else{
                   Cs[index]->Update();       
                 }
            }else{
                 string power_interface=node_id+"0";
                 update_appliance_desired_current(node_id,request_current);
                 int index=find_component(power_interface);
                 if(Cs[index]->is_dynamic()){
                   Cs[index]->DynamicUpdate();
                }else{
                   Cs[index]->Update();    
                 }
            }
            
        }
        result="s";
    }
    if(role=="Switch"){
        
      int power_interface_flag=atoi(node_update.c_str());
      if(power_interface_flag!=it->second.power_interface_flag){
         connected_components.clear();
         for(int i=0;i<Cs.size();i++)
             delete Cs[i];
         Cs.clear();
         adj_list.clear();
       
        it->second.power_interface_flag=power_interface_flag;
        create_adj_list();
        if(!adj_list.empty())
        find_connected_components();
      }
        string result1=simulate();
        result=result1+data;
        
    }
    if(!result.empty())
   return result;
   
    
}
void PowerSimulator::update_appliance_desired_current(string node_id, double request_current){
    map<string,Node>::iterator it;
    it=nodes.find(node_id);
    string role=it->second.role;
    if(role=="Demander" || role =="WindTurbine" || role=="SolarPanel" || 
             role=="PowerPlant" || role=="Storage"){
            it->second.desired_current=request_current;
    }
}



void PowerSimulator:: print_everything(){
    //               cout<<"y_L_inv_inv:"<<endl;
//               for(int i=0;i<edging_power_line_num;i++){
//                  for(int j=0;j<edging_power_line_num;j++)
//                      cout<<gsl_matrix_get(y_L_inv_inv,i,j)<<endl;
//               }
//               cout<<"second_part_first_phase_sum:"<<endl;
//               for(int i=0;i<edging_power_line_num;i++){
//                      cout<<gsl_vector_get(second_part_first_phase_sum,i)<<endl;
//               }
//               
//               cout<<"i_L:"<<endl;
//               for(int i=0;i<edging_power_line_num;i++)
//               cout<<gsl_vector_get(i_L,i)<<endl;
    //                 cout<<"Matrix A:"<<endl;
//                 for(int i=0;i<switch_pi;i++){
//                     for(int j=0;j<switch_pi;j++){
//                         cout<<gsl_matrix_get(A,i,j)<<",";
//                     }
//                 cout<<endl;
//                 }
//                 cout<<"Vector I:"<<endl;
//                 for(int i=0;i<switch_pi;i++){
//                     cout<<gsl_vector_get(I,i)<<endl;
//                 }
//                 
//                 cout<<"Power interface index:"<<endl;
//                 map<int,string>::iterator it;
//                 for(it=index_pi_mapping.begin();it!=index_pi_mapping.end();it++)
//                     cout<<it->first<<":"<<it->second<<endl;
           //     cout<<"Ib:"<<endl;
           //     for(int i=0;i<trans_power_line.size();i++){
           //         cout<<gsl_vector_get(I_b2,i)<<endl;
           //     }
                    
                   
         
       cout<<"adj_list:"<<endl;
        pair<multimap<string,string>::iterator,multimap<string,string>::iterator> ret;
        multimap<string,string>::iterator its;
        map<string,PowerInterface>::iterator itt;
        
        for (itt=powerInterfaces.begin();itt!=powerInterfaces.end();itt++)
        {
              cout << itt->first << " =>";
              ret = adj_list.equal_range(itt->first);
            for (its=ret.first; its!=ret.second; ++its)
                   cout << " " << (*its).second;
        cout << endl;
          }
}
int PowerSimulator::find_component(string power_interface){
    for(int i=0;i<connected_components.size();i++)
        for(int j=0;j<connected_components[i].size();j++){
            if(power_interface.compare(connected_components[i][j])==0)
                return i;
        }
    return -1;
}

//update vector I,since after load balance, the vector I for all power interfaces should be updated.


string PowerSimulator::sessionInfo(){
    
         map<string,PowerLine> powerLines=this->getPowerLines();
         map<string,Node> nodes=this->getNodes();
         char result_buffer[OUTPUT_BUFFER_SIZE];
         std::ostringstream oss(result_buffer);
         map<string,PowerLine>::iterator it;
         map<string,Node>::iterator it1;
         for(it=powerLines.begin();it!=powerLines.end();it++){
           oss<<it->first<<":"<<it->second.current<<":"<<it->second.lpeer
                   <<":"<<it->second.rpeer<<":"<<it->second.lpeerIf<<":"
                   <<it->second.rpeerIf<<endl;
         }
         for (it1=nodes.begin();it1!=nodes.end();it1++){
             oss<<it1->first<<":"<<it1->second.current<<":"<<it1->second.desired_current<<":"
                     <<it1->second.max_current<<":"<<it1->second.power_interface_flag<<":";
             for(int j=0;j<it1->second.power_interfaces.size();j++){
                 if(j==it1->second.power_interfaces.size())
                 oss<<it1->second.power_interfaces[j][it1->second.power_interfaces[j].size()-1];
                 else
                   oss<<it1->second.power_interfaces[j][it1->second.power_interfaces[j].size()-1]<<" ";  
             }
             string role=it1->second.role;
             if(role=="Storage"){
                 oss<<":"<<it1->second.e_stored<<":"<<it1->second.capacity<<":"<<it1->second.operation_mode;
             }
             oss<<endl;
           
         }         
         return oss.str().c_str();
    
    
}
string PowerSimulator::reply(string data){
    string result; 
    int field=0;
    string fields[10];//may exceed 10 fields later.
    string value;
    istringstream ssline(data);
    while (getline ( ssline, value, ':' )){
         fields[field++]=value;
    }
    string node_id=fields[0];
    fields[1]=fields[1].erase(fields[1].find_last_not_of(" \n\r\t")+1);
    if(fields[1]=="neighbors"){
        result=find_neighbors(node_id);        
    }else if(fields[1]=="power_line"){
        fields[2]=fields[2].erase(fields[2].find_last_not_of(" \n\r\t")+1);
        result=find_power_line_resistor(node_id,fields[2]);
    }else if(fields[1]=="type"){
        map<string,Node>::iterator it;
        it=nodes.find(node_id);
        result=it->second.role;
    }
    else{
     int flag=atoi(fields[1].c_str());
     map<string,Node>::iterator it;
     it=nodes.find(node_id);
     std::ostringstream strs;
     switch (flag){
        case 0: strs << it->second.current;
                result=strs.str();break;
        case 1: strs << it->second.max_current;
                result=strs.str();break;
        case 2: strs << it->second.e_stored;
                result=strs.str();break;
        case 3: strs << it->second.operation_mode;
                result=strs.str();break;
     }
    }
    return result;
    
}
string PowerSimulator::find_neighbors(string node){
    map<string,PowerLine>::iterator it;
    char result_buffer[100];
    bzero(result_buffer,100);
    std::ostringstream oss(result_buffer);
    for(it=powerLines.begin();it!=powerLines.end();it++){
        string lpeer=it->second.lpeer;
        string rpeer=it->second.rpeer;
        if(lpeer==node)
            oss<<rpeer<<":";
        if(rpeer==node)
            oss<<lpeer<<":";
    }
    return oss.str();
}
string PowerSimulator::find_power_line_resistor(string node1, string node2){
    map<string,PowerLine>::iterator it;
    char result_buffer[100];
    bzero(result_buffer,100);
    std::ostringstream oss(result_buffer);
    for(it=powerLines.begin();it!=powerLines.end();it++){
        string lpeer=it->second.lpeer;
        string rpeer=it->second.rpeer;
        if((lpeer==node1 && rpeer == node2) || (lpeer==node2 && rpeer == node1)){
            double resistor=it->second.resistor;
            oss<<resistor;
        }
          
    }
    return oss.str();
    
}
bool PowerSimulator::find_tunnel(vector<string>& connected_component){
     for(int i=0;i<connected_component.size();i++){
          
             map<string,PowerInterface>::iterator it;
             it=powerInterfaces.find(connected_component[i]);
             string nodeId=it->second.getNodeId();
             map<string,Node>::iterator it1;
             it1=nodes.find(nodeId);
             string role=it1->second.role;
             double de=it1->second.desired_current;
             if(role == "Switch" && de==1)
                 return true;
     }
     return false;
    
}
void PowerSimulator::DynamicCoordinatedLoadUpdate(string data){  
    //potential error: one connected component only
    Cs[0]->DynamicCoordinatedLoadUpdate(data);
}
void PowerSimulator::DynamicCoordinatedBoundaryUpdate(string data){
    //potential error: one connected component only
    Cs[0]->DynamicCoordinatedBoundaryUpdate(data);
  
}

string PowerSimulator::find_tunnel_power_line(string peer1){
    map<string,PowerLine>::iterator it;
    string power_line;
     map<string,PowerInterface>::iterator it5;

    for(it=powerLines.begin();it!=powerLines.end();it++){
        if(it->second.isPeer(peer1)){
            string otherPeer=it->second.otherPeer(peer1);
            it5=powerInterfaces.find(otherPeer);
            string neighbor_node_id=it5->second.getNodeId();
            map<string,Node>::iterator it6;
            it6=nodes.find(neighbor_node_id);
            string role=it6->second.role;
            if(role=="Tunnel"){
               power_line=it->first;
            }
        }
    }
    
    return power_line;
}
string PowerSimulator::CoordinatorIpOfConnectedComponent(string power_interface){
    //the index could be -1 if the power interface doesn't exist. the error happens
    int index=find_component(power_interface);
     return Cs[index]->coordinator_ip();
}

void PowerSimulator::DynamicConnectedSimulation(string data){

    string value;
    string coordinator_ip;
    string line;
    std::istringstream iss(data);
    int index=0;
    while(getline(iss,line)){
        int field=0;
        std::istringstream ssline(line);
         DynamicConnection dc;
        while (getline ( ssline, value, ':' )){
         if(field==0){
               coordinator_ip=value;
               if(coordinator_ip=="default"){
                   //in this case, it is the coordinator server, let it wait the others to go first.
                   coordinator_ip="localhost";
               }
         }
         if(field==1){
                dc.remote_ip=value; 
         }
         if(field==2){
                dc.remote_pi=value;
         }
         if(field==3){
             dc.local_pi=value.erase(value.find_last_not_of(" \n\r\t")+1);
         }
             field++;
       }
        
       index=find_component(dc.local_pi);
 
       Cs[index]->AddDynamicConnection(dc);
       
    } 
    // assume one connected_component.
    Cs[index]->set_coordinator_ip(coordinator_ip);
    Cs[index]->set_is_dynamic(true);
    Cs[index]->DynamicSimulate();
    
}
// assume one connected component
void PowerSimulator::restore(){
    
    Cs[0]->restore();
    
}