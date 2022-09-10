#include "forwarder.h"
#include "ipaddress.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include "connected_component.h"
#include "thread_functions.h"
forwarder::forwarder(){
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
        pMtx=NULL;
}

void forwarder::initialize(char* input){
     this->input=input;
//     GetKIpaddress();
}

void forwarder::parse(){
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
             field++;
    }
    for(int i=0;i<order+size;i++){
        getline(iss,line);
        if(line != ""){
        std::istringstream ssline(line);
        field=0;
        if(i<order){
            Node temp;
            while(getline(ssline,value,':')){
                switch (field){
                    case 1: temp.id=value;break;
                    case 2: temp.role=value.substr(1);break;
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
    //                case 7: temp.location=value.erase(value.find_last_not_of(" \n\r\t")+1); break; 
                    case 7: temp.e_stored=atof(value.c_str());break;
                    case 8: temp.capacity=atof(value.c_str());break;
                    case 9: temp.operation_mode=value.erase(value.find_last_not_of(" \n\r\t")+1); break; 
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
      }
    }
    create_adj_list();
   if(!adj_list.empty());
    find_connected_components();
    
}
void forwarder::create_adj_list(){
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


void forwarder::dfs_visit(string power_interface){
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
             forwarder::dfs_visit(neighbor);
     
     }
    it->second.setColor("black");
}


void forwarder::find_connected_components(){
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
string forwarder::simulate(){
    for(int i=0;i<connected_components.size();i++){  
        ConnectedComponent* c=new ConnectedComponent(this,connected_components[i]);
        c->simulate();
        Cs.push_back(c);
    }
     string result="s";
    return result;  
}

bool forwarder::update_appliance(string node_id,double new_current){
    
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
string forwarder::find_app_power_line(string app_node_id){
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
string forwarder::find_power_line(string peer1, string peer2){
    map<string,PowerLine>::iterator it;
    string power_line;
    for(it=powerLines.begin();it!=powerLines.end();it++){
        if(it->second.isPeer(peer1)&& it->second.isPeer(peer2))
            power_line=it->first;
    }
    
    return power_line;
}
string forwarder::update(string data){
    
    // only deal with current updates, no switch updates.
     if(Cs[0]->is_dynamic()){
            Cs[0]->DynamicUpdate();
     }else{
           Cs[0]->Update();
     } 
    return data;
    
}
void forwarder::update_appliance_desired_current(string node_id, double request_current){
    map<string,Node>::iterator it;
    it=nodes.find(node_id);
    string role=it->second.role;
    if(role=="Demander" || role =="WindTurbine" || role=="SolarPanel" || 
             role=="PowerPlant" || role=="Storage"){
            it->second.desired_current=request_current;
    }
}



void forwarder:: print_everything(){
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
//                cout<<"Ib:"<<endl;
//                for(int i=0;i<trans_power_line.size();i++){
//                    cout<<gsl_vector_get(I_b2,i)<<endl;
//                }
                    
                   
         
//       cout<<"adj_list:"<<endl;
//        pair<multimap<string,string>::iterator,multimap<string,string>::iterator> ret;
//        multimap<string,string>::iterator its;
//        map<string,PowerInterface>::iterator itt;
//        
//        for (itt=powerInterfaces.begin();itt!=powerInterfaces.end();itt++)
//        {
//              cout << itt->first << " =>";
//              ret = adj_list.equal_range(itt->first);
//            for (its=ret.first; its!=ret.second; ++its)
//                   cout << " " << (*its).second;
//        cout << endl;
//          }
}
int forwarder::find_component(string power_interface){
    for(int i=0;i<connected_components.size();i++)
        for(int j=0;j<connected_components[i].size();j++){
            if(power_interface.compare(connected_components[i][j])==0)
                return i;
        }
    return -1;
}
void forwarder::forward(){
      char data[50];
      int i;
    for(i=0;i<NUMBER_OF_NODES;i++){
        if(i==0 || i==3 || i==9 || i==15)
            continue;
        int send_sock;
        struct hostent *host1;
        struct sockaddr_in server_addr1;
        bzero(data,50);
        char ts[10];
        string ss="Update session:";
        sprintf(ts,"%d:",i+1);
        strcat(data,ss.c_str());
        strcat(data,ts);
        
        
        char ids[10];
        bzero(ids,10);
        char ts2[5];
        bzero(ts2,5);
        ids[0]='n';
        sprintf(ts2,"%d",i+1);
        strcat(ids,ts2);
        string node_id(ids);
      
        map<string,Node>::iterator it;
        it=nodes.find(node_id);
        string role=it->second.role;
        char settings[10];
        if(role=="Demander" || role =="WindTurbine" || 
                role=="SolarPanel" || role=="PowerPlant" ){
            sprintf(settings,"%d",(int)it->second.desired_current);
        }
        else if(role=="Switch"){
            sprintf(settings,"%d",(int)it->second.power_interface_flag);
        }else{
            if(it->second.operation_mode=="discharging"){
                int ss=it->second.desired_current+32768;
                sprintf(settings,"%d",ss);
            }
            else{
                sprintf(settings,"%d",(int)it->second.desired_current);
            }
            
                
        }
        strcat(data,settings);
        
        strcat(data,"\r\n");
        printf("%s,$$,%s",kIpaddress[i].c_str(),data);
        
        host1 = gethostbyname(kIpaddress[i].c_str());
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
            continue;
        }
        send(send_sock,data,50, 0); 
        printf("sent! %d\n",i);
     
        close(send_sock); 
     
    }
}
void forwarder::getTestbedStatus(void *p){
   int testbed_monitor_thread_create;
   this->pMtx=(pthread_mutex_t*)p;
   testbed_monitor_thread_create=pthread_create(&testbed_monitor_thread,NULL,&testbed_monitor,this);  
}
string forwarder::CoordinatorIpOfConnectedComponent(string power_interface){
    int index=find_component(power_interface);
          return Cs[index]->coordinator_ip();
}
void forwarder::DynamicConnectedSimulation(string data){
        string value;
    string coordinator_ip;
    string remote_ip;
    string remote_pi;
    string local_pi;
    std::istringstream ssline(data);
    
    //this part is different from the power simulator since only one dynamic connection could be 
    // established to the testbed using one interface module.
    int field=0;
    while (getline ( ssline, value, ':' )){
         if(field==0){
               coordinator_ip=value;
               if(coordinator_ip=="default"){
                   //in this case, it is the coordinator server, let it wait the others to go first.
                   coordinator_ip="localhost";
               }
         }
         if(field==1){
                remote_ip=value; 
         }
         if(field==2){
             remote_pi=value;
         }
         if(field==3){
             local_pi=value.erase(value.find_last_not_of(" \n\r\t")+1);
         }
             field++;
    }
    int index=find_component(local_pi);
    Cs[index]->set_coordinator_ip(coordinator_ip);
    DynamicConnection dc;
    dc.remote_ip=remote_ip;
    dc.remote_pi=remote_pi;
    dc.local_pi=local_pi;
    Cs[index]->AddDynamicConnection(dc);
    Cs[index]->set_is_dynamic(true);
    Cs[index]->DynamicSimulate();
        
        
}

void forwarder::DynamicCoordinatedLoadUpdate(string data){  
    //potential error: one connected component only
    Cs[0]->DynamicCoordinatedLoadUpdate(data);
}

void forwarder::DynamicCoordinatedBoundaryUpdate(string data){
    //potential error: one connected component only
    Cs[0]->DynamicCoordinatedBoundaryUpdate(data);
  
}
string forwarder::find_tunnel_power_line(string peer1){
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
void forwarder::restore(){
    
    Cs[0]->restore();
    
}