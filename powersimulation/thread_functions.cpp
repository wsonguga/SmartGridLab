#include "thread_functions.h"
#include "power_simulator.h"
#include "forwarder.h"
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include "node.h"
#include "util.h"
#include "ipaddress.h"
#include "connected_component.h"
void* real_time_report(void* param){
    
   PowerSimulator * p_sim = (PowerSimulator *) param;
   time_t rawtime;
   struct tm * timeinfo;
   string out_file_name=p_sim->get_out_file();
   FILE* out_file;
   if(p_sim!=NULL){
       out_file=fopen(out_file_name.c_str(),"w");
       if(out_file==NULL){
           util::log("output file wrong!");
           exit(1);
       }
           
       time ( &rawtime );
       timeinfo = localtime ( &rawtime );
       fprintf(out_file,"%s\n","************************************************************");
       fprintf(out_file,"%s\n","Real-time info for emulation session ");
       fprintf(out_file,"       started at %s",asctime(timeinfo));
       fprintf(out_file,"%s\n","************************************************************");
       fclose(out_file);
    
    while(true){
      
          char result_buffer[OUTPUT_BUFFER_SIZE];
          std::ostringstream oss(result_buffer);
          time ( &rawtime );
          timeinfo = localtime ( &rawtime );
          oss<<asctime(timeinfo);
          map<string,PowerLine>::iterator it;
          for(it=p_sim->getPowerLines().begin();it!=p_sim->getPowerLines().end();it++){
              oss<<it->first<<":"<<it->second.current<<"|";
          }
          oss<<endl;
          map<string,Node>::iterator it1;
          for(it1=p_sim->getNodes().begin();it1!=p_sim->getNodes().end();it1++){
              string role=it1->second.role;
              if(role=="Switch")
              oss<<it1->first<<":"<<it1->second.power_interface_flag<<"|";
              else if(role=="Storage")
              oss<<it1->first<<":"<<it1->second.current<<":"<<it1->second.operation_mode<<":"<<it1->second.e_stored<<"|";
              else
                  oss<<it1->first<<":"<<it1->second.current<<"|";
          }
          oss<<endl;
          string result=oss.str().c_str();
          if(p_sim->getExecMode()==BATCH_MODE){  
              out_file=fopen(out_file_name.c_str(),"a+");
              if(out_file==NULL){
                 util::log("output file wrong!");
                 exit(1);
              }
              fprintf(out_file,"%s\n","***************************");
              fprintf(out_file,"%s\n",result.c_str());
              fclose(out_file);
              
          }else {
              out_file=fopen(out_file_name.c_str(),"a+");
              if(out_file==NULL){
                  util::log("output file wrong!");
                  exit(1);
              }
              fprintf(out_file,"%s\n","***************************");
              fprintf(out_file,"%s\n",result.c_str());
              fclose(out_file);
              
              
               int send_sock=util::open_client_socket(p_sim->gui_client_ip,9010); 
               if(send_sock>0){
               util::send_gui_update(send_sock,result);// a shutdown function at the end.
               close(send_sock);
               }
          }
          if(SAMPLING_TIME_INTERVAL_EMULATOR >=1){
            sleep(SAMPLING_TIME_INTERVAL_EMULATOR);
          }
          else {
            long int time_interval=(long)1000000*SAMPLING_TIME_INTERVAL_EMULATOR;
            usleep(time_interval);
          } 
          
          
        
    }    
   }
}
void* storage_monitor(void* param){
     PowerSimulator * p_sim = (PowerSimulator *) param;
     map<string,Node>::iterator it;
     vector<string> storages;
     for(it=p_sim->getNodes().begin();it!=p_sim->getNodes().end();it++){
         string role=it->second.role;
         if(role=="Storage")
             storages.push_back(it->first);
     }
     while (true){
        if(STORAGE_MONITOR_INTERVAL >=1){
            sleep(STORAGE_MONITOR_INTERVAL);
        }
        else {
            long int time_interval=(long)1000000*STORAGE_MONITOR_INTERVAL;
            usleep(time_interval);
        }
        for(int i=0;i<storages.size();i++){
            map<string,Node>::iterator it1;
            it1=p_sim->getNodes().find(storages[i]);
            string op_mode=it1->second.operation_mode;
            double e_stored=it1->second.e_stored;
            double current=it1->second.current;
            double capacity=it1->second.capacity;
            if(op_mode=="charging"){
                e_stored+=current*STORAGE_MONITOR_INTERVAL;
                if(e_stored>capacity){
                    it1->second.operation_mode="standing";
                    it1->second.e_stored=capacity;
                    string power_interface=storages[i]+"0";
                    int index=p_sim->find_component(power_interface);
                  //  ((p_sim->getCs()).at(index))->Update(power_interface);
                   p_sim->getCs()[index]->Update();
                   //s[index]->Update(power_interface);
                   // p_sim->update_component(power_interface);
                }else{
                   it1->second.e_stored=e_stored;
                }
                    
            }else if(op_mode=="discharging"){
                e_stored-=current*STORAGE_MONITOR_INTERVAL;
                if(e_stored<0){
                    it1->second.operation_mode="standing";
                    it1->second.e_stored=0;
                    string power_interface=storages[i]+"0";
                    int index=p_sim->find_component(power_interface);
                    p_sim->getCs()[index]->Update();
                    //p_sim->update_component(power_interface);
                }else{
                   it1->second.e_stored=e_stored;
                }
            }else{
                
            }
        }
        
        
     }
}

void* testbed_monitor(void* param){
    forwarder* fd = (forwarder*) param;    
    char send_buffer[50];
    char receive_buffer[50];
    int i;
    struct tm * timeinfo;
    time_t rawtime;
    int counter=0;
  while(true){
      
         pthread_mutex_lock (fd->pMtx);
    for(i=0;i<NUMBER_OF_NODES;i++){    
         if(i==0 || i==3 || i==9 || i==15)
            continue;
        int send_sock;
        struct hostent *host1;
        struct sockaddr_in server_addr1;
        bzero(send_buffer,50);
        bzero(receive_buffer,50);
        char ts[10];
        string ss="Retrieve session:";
        sprintf(ts,"%d:",i+1);
        strcat(send_buffer,ss.c_str());
        strcat(send_buffer,ts);
        
        // the flag to match energy daemon
        const char* flag="0";
        strcat(send_buffer,flag);
        
        strcat(send_buffer,"\r\n");
        host1 = gethostbyname(kIpaddress[i].c_str());
        if ((send_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            printf("Socket creation failed! ");
            exit(1);
        }

        server_addr1.sin_family = AF_INET;
        // the server port on java sgmonitor
        server_addr1.sin_port = htons(9009);
        server_addr1.sin_addr = *((struct in_addr *)host1->h_addr);
        bzero(&(server_addr1.sin_zero),8);

        if (connect(send_sock, (struct sockaddr *)&server_addr1,
                    sizeof(struct sockaddr)) == -1)
        {
            printf("Connect failed!!!  Beagleboard  %d may not be booted.\n",i+1);
            exit(1);
        }
        send(send_sock,send_buffer,50, 0); 
        shutdown(send_sock, SHUT_WR);
        read(send_sock,receive_buffer,50);
        close(send_sock); 
        string result(receive_buffer);
        //hard coding for the switch status
        if((i+1)==4){
            result="15";
        }
        if((i+1)==1){
            result="30";
        }
        if((i+1)==10){
            result="30";
        }
        if((i+1)==16){
            result="28";
        }
    //    cout<<"receive from node"<<i+1<<":"<<result;
        
        
        char ids[10];
        bzero(ids,10);
        char ts2[5];
        bzero(ts2,5);
        ids[0]='n';
        sprintf(ts2,"%d",i+1);
        strcat(ids,ts2);
        string node_id(ids);
        map<string,Node>::iterator it;
        it=fd->getNodes().find(node_id);
        string role=it->second.role;
        if(role=="Demander" || role =="WindTurbine" || 
                role=="SolarPanel"){
            double x=atoi(result.c_str())/100.0;
            it->second.current=atoi(result.c_str())/100.0;
            if(x<it->second.max_current){
                //gradually converge to final stable stage.
                it->second.desired_current=(it->second.max_current-x)/2;
            }
            
            
            
        }
        else if(role=="PowerPlant"){
            double x=atoi(result.c_str())/100.0;
            it->second.current=atoi(result.c_str())/10.0;
             if(x<it->second.max_current){
                it->second.desired_current=(it->second.max_current-x)/2;
            }
        }
        else if(role=="Switch"){
           it->second.power_interface_flag=atoi(result.c_str());
        }else{
            if(it->second.operation_mode=="discharging"){
                it->second.current=(atoi(result.c_str())-32768)/100.0;
                double x=atoi(result.c_str())/100.0;
                if(x<it->second.max_current){
                it->second.desired_current=(it->second.max_current-x)/2;
               }
            }
            else{
                
                it->second.current=(atoi(result.c_str()))/100.0;
                double x=atoi(result.c_str())/100.0;
                if(x<it->second.max_current){
                it->second.desired_current=(it->second.max_current-x)/2;
               }
            }
            
                
        }
        
        
        
        
        
    }
       //add switch update later.
   /*    fd->connected_components.clear();
        for(int i=0;i<fd->Cs.size();i++)
             delete fd->Cs[i];
      
         fd->Cs.clear();
         fd->adj_list.clear();
      
        fd->create_adj_list();
        if(!fd->adj_list.empty())
        fd->find_connected_components(); */
     
    // here should be simply updated the current, not topology data structures.
    //   cout<<"Simulate begins!"<<endl;
    //       fd->simulate();
    //   cout<<"Simulate ends!"<<endl;
         
         if(counter==0){
             fd->create_adj_list();
             if(!fd->adj_list.empty())
             fd->find_connected_components();
             cout<<"Simulate begins!"<<endl;
             fd->simulate();
             cout<<"Simulate ends!"<<endl;
         }
         else {
         string data="data";
         cout<<"Update begins!"<<endl;
           fd->update(data);
         cout<<"Update ends!"<<endl;
         
         }
       
   //  while (true){  
       char result_buffer[SOCKET_BUFFER_SIZE];
       std::ostringstream oss(result_buffer);
          time ( &rawtime );
          timeinfo = localtime ( &rawtime );
          oss<<asctime(timeinfo);
          map<string,PowerLine>::iterator it;
          for(it=fd->getPowerLines().begin();it!=fd->getPowerLines().end();it++){
              oss<<it->first<<":"<<it->second.current<<"|";
          }
          oss<<endl;
          map<string,Node>::iterator it1;
          for(it1=fd->getNodes().begin();it1!=fd->getNodes().end();it1++){
              string role=it1->second.role;
              if(role=="Switch")
              oss<<it1->first<<":"<<it1->second.power_interface_flag<<"|";
              else if(role=="Storage")
              oss<<it1->first<<":"<<it1->second.current<<":"<<it1->second.operation_mode<<":"<<it1->second.e_stored<<"|";
              else
                  oss<<it1->first<<":"<<it1->second.current<<"|";
          }
          oss<<endl;
          string result=oss.str().c_str();
          int send_sock=util::open_client_socket("localhost",9010); 
          if(send_sock>0){
               util::send_gui_update(send_sock,result);// a shutdown function at the end.
               close(send_sock);
          }
      
         pthread_mutex_unlock (fd->pMtx);
       if(SAMPLING_TIME_INTERVAL_TESTBED >=1){
            sleep(SAMPLING_TIME_INTERVAL_TESTBED);
       }
       else {
            long int time_interval=(long)1000000*SAMPLING_TIME_INTERVAL_TESTBED;
            usleep(time_interval);
       }   
      
      
         counter++;
  }
}
