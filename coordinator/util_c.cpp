#include <sys/stat.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <cstdlib>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <fstream>
#include "util_c.h"
using namespace std;
LogFile util::logFile;
void util::daemonize(void)
{
    pid_t pid, sid;

    /* already a daemon */
    if ( getppid() == 1 ) return;

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then we can exit the parent process. */
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    /* At this point we are executing as the child process */

    /* Change the file mode mask */
         umask(0);
    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
        exit(EXIT_FAILURE);
    }
   
    /* Change the current working directory.  This prevents the current
       directory from being locked; hence not being able to remove it. */
    if ((chdir("/")) < 0) {
        exit(EXIT_FAILURE);
    }

    /* Redirect standard files to /dev/null */
    freopen( "/dev/null", "r", stdin);
    freopen( "/dev/null", "w", stdout);
    freopen( "/dev/null", "w", stderr);
}
/*
 * each item should be in the same sequence as the required input of power simulator.
 */
string util::parse_entry(string entry){
    string line;
    std::istringstream iss(entry);
    char result_buffer[100];
    bzero(result_buffer,100);
    std::ostringstream oss(result_buffer);
    string role="";
    string node_model, node_desired_current,
     node_max_current,node_power_interfaces,node_power_interfaces_flag, node_location,
            e_stored,capacity,operation_mode;
    
    string power_line_resistor,power_line_lpeer,power_line_rpeer,power_line_power_interfaces;
    
    string power_simulator_host,power_simulator_port;
    while (getline (iss, line)){
       if(role==""){
           std::istringstream ssline(line);
           int field=0;
           string value;
           while (getline(ssline,value,' ')){
               if(field==0){
                   if(value=="node"){
                       oss<<"n:";
                       role="node";
                   }
                   else if(value=="power_line"){
                       oss<<"p:";
                       role="power_line";
                   }
                   else if(value=="power_simulator"){
                       oss<<"s:";
                       role="power_simulator";
                   }
                   else{ /*
                           Only process node and power_line entries.
                          */
                       return "";
                   }
               }
               if(field==1 && (role=="node" || role=="power_line" || role=="power_simulator"))
                   oss<<value<<":";
               
               field++;
           }
       }
       else{
          
           char* item;
           char* value1;
           item=strtok((char*)line.c_str()," ");
    
          if(item!=NULL){
           string t1(item);
        
          
           if(role=="node"){
               if(t1=="model"){
                   value1 = strtok (NULL, " ");
                   string t2(value1);
                   node_model=t2;
               }
               if(t1=="desired_current"){
                   value1 = strtok (NULL, " ");
                   string t2(value1);
                   node_desired_current=t2;
               }
               if(t1=="max_current"){
                   value1 = strtok (NULL, " ");
                   string t2(value1);
                   node_max_current=t2;
               }
               if(t1=="power_interfaces_flag"){
                   value1 = strtok (NULL, " ");
                   string t2(value1);
                   node_power_interfaces_flag=t2;
               }
               if(t1=="power_interfaces"){
                     int pos1=line.find("{");
                     int pos2=line.find("}");
                     node_power_interfaces=line.substr(pos1+1,pos2-pos1-1);
               }
               if(t1=="location"){
                   value1 = strtok (NULL, " ");
                   string t2(value1);
                   node_location=t2;
               }
               if(t1=="e_stored"){
                   value1 = strtok(NULL," ");
                   string t2(value1);
                   e_stored=t2;
               }
               if(t1=="capacity"){
                   value1 = strtok(NULL," ");
                   string t2(value1);
                   capacity=t2;
               }
                 if(t1=="operation_mode"){
                   value1 = strtok(NULL," ");
                   string t2(value1);
                   operation_mode=t2;
               }   
           }
           if(role=="power_line"){
               if(t1=="resistor"){
                   value1 = strtok (NULL, " ");
                   string t2(value1);
                   power_line_resistor=t2;
               }
               if(t1=="nodes"){
                     int pos1=line.find("{");
                     int pos2=line.find("}");
                     string peers=line.substr(pos1+1,pos2-pos1-1);
                     istringstream ssline(peers);
                     string value;
                     int field=0;
                     while(getline(ssline,value,' ')){
                         if(field==0)
                             power_line_lpeer=value;
                         if(field==1)
                             power_line_rpeer=value;
                         field++;
                     }
              }
               if(t1=="power_interfaces") {
                   int pos1=line.find("{");
                   int pos2=line.find("}");
                   string peers=line.substr(pos1+1,pos2-pos1-1);
                   power_line_power_interfaces=peers;
                   
               }
           }
           if(role=="power_simulator"){
               if(t1=="host"){
                   value1 = strtok (NULL, " ");
                   string t2(value1);
                   power_simulator_host=t2;
               }
               if(t1=="port"){
                   value1 = strtok (NULL, " ");
                   string t2(value1);
                   power_simulator_port=t2;
               }
               
           }
          }
       }
    }
    if(role=="node"){
        if(node_model!="" && node_desired_current!="" &&
                node_max_current!="" && node_power_interfaces != ""
                && node_power_interfaces_flag !=""){
            if(node_location=="")
                node_location="localhost";
            oss<<node_model<<":"<<node_desired_current<<":"<<node_max_current<<":"
                    <<node_power_interfaces<<":"<<node_power_interfaces_flag<<":"
                    <<node_location;
            if(node_model=="Storage")
                oss<<":"<<e_stored<<":"<<capacity<<":"<<operation_mode;
        }
        else {
            util::log(".imn batch input file wrong format!!");
            cout<<".imn batch input file wrong format!!"<<endl;
            exit(1);
        }
    }
    if(role=="power_line"){
        if(power_line_resistor!="" && power_line_lpeer!="" &&
                power_line_rpeer!="" && power_line_power_interfaces != ""
                ){
           
            oss<<power_line_resistor<<":"<<power_line_lpeer<<":"<<power_line_rpeer<<":"
                    <<power_line_power_interfaces;
        }
        else {
             util::log(".imn batch input file wrong format!!");
            cout<<".imn batch input file wrong format!!"<<endl;
            exit(1);
        }
    }
     if(role=="power_simulator"){
        if(power_simulator_host!="" && power_simulator_port!="" ){
            oss<<power_simulator_host<<":"<<power_simulator_port;
        }
        else {
             util::log(".imn batch input file wrong format!!");
            cout<<".imn batch input file wrong format!!"<<endl;
            exit(1);
        }
    }
    return oss.str();
}


//This function will send large data using socket. After that
//it invokes shutdown to disallow sending through socket,so the other
//side can know data has been received completely. Reading is still ok
//for receiving feedback. The maximum data length is a long int type.
void util::send_data(int sock,string data,int max_packet_size){
        long int bytes_left=data.length();
        long int packet_num=0;
        long int n=0;
 //       cout<<"Sending data..."<<endl;
     while(bytes_left>0){         
         if(bytes_left<=max_packet_size){
           n=send(sock,data.c_str()+packet_num*max_packet_size,bytes_left, 0);
         }
         else {
             n=send(sock,data.c_str()+packet_num*max_packet_size,max_packet_size, 0);
         }
        packet_num++;
        bytes_left=data.length()-packet_num*max_packet_size;
    }
    if(bytes_left<=0)
  //     cout<<"Total "<<data.length()<<" bytes have been sent!"<<endl;
        // This closes the sending.
     shutdown(sock, SHUT_WR);       
        
} 
long int util::receive_data(int sockfd,char* receive_buffer,long int receive_buffer_size,
        int max_packet_size){                      
  //  cout<<"Receiving start..."<<endl;
     long int bytes_received=0;
     int n;
     char packet[max_packet_size];
     while((n = read(sockfd,packet,max_packet_size))>0){
         bytes_received+=n;
         if(bytes_received >=receive_buffer_size){
             cout<<"Receive buffer overflow!"<<endl;
             break;
         }
         if(packet[n-6]=='$' && packet[n-5]=='$' && packet[n-4]=='$' && packet[n-3]=='$'){
           for(int i=0;i<n-6;i++){
             //$$$$ is length 4,plus \r\n,  6 bytes in all from tcl side.
            receive_buffer[bytes_received-n+i]=packet[i];
           }  
              return bytes_received-6;
         }
         for(int i=0;i<n;i++){
             //$$$$ is length 4,plus \r\n,  6 bytes in all from tcl side.
            receive_buffer[bytes_received-n+i]=packet[i];
         }
        
         bzero(packet,max_packet_size);
     }
  //   cout<<"Total "<<bytes_received<<" bytes have been received!"<<endl;
     return bytes_received;
}

string util::exec_on_server(string host, int port, string data){
        int send_sock;
        struct hostent *host1;
        struct sockaddr_in server_addr1;
   //     cout<<"Execute on Server "<<host<<endl;
        host1 = gethostbyname(host.c_str());

        if ((send_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            printf("Socket creation failed!");
            exit(1);
        }

        server_addr1.sin_family = AF_INET;
        server_addr1.sin_port = htons(port);
        server_addr1.sin_addr = *((struct in_addr *)host1->h_addr);
        bzero(&(server_addr1.sin_zero),8);

        if (connect(send_sock, (struct sockaddr *)&server_addr1,
                    sizeof(struct sockaddr)) == -1)
        {
            printf("Connect failed!");
            exit(1);
        }
        send_data(send_sock,data,MAXIMUM_PACKET_SIZE);
        char receive_buffer[SOCKET_BUFFER_SIZE];
        bzero(receive_buffer,SOCKET_BUFFER_SIZE);
        receive_data(send_sock,receive_buffer,SOCKET_BUFFER_SIZE,MAXIMUM_PACKET_SIZE);
        string result(receive_buffer);
        return result;
 }
void util::send_gui_update(int sock,string update){
  //  cout<<"Send to GUI in real-time"<<endl;
   // cout<<update<<endl;
    send_data(sock,update,MAXIMUM_PACKET_SIZE);
}
int util::open_client_socket(string host,int port){
         int send_sock;
         struct hostent *host1;
         struct sockaddr_in server_addr1;
         host1 = gethostbyname(host.c_str());

         if ((send_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            printf("Socket creation failed!");
            //exit(1);
         }

        server_addr1.sin_family = AF_INET;
        server_addr1.sin_port = htons(port);
        server_addr1.sin_addr = *((struct in_addr *)host1->h_addr);
        bzero(&(server_addr1.sin_zero),8);

        if (connect(send_sock, (struct sockaddr *)&server_addr1,
                    sizeof(struct sockaddr)) == -1)
        {
            util::log("Connect to GUI failed!");
            cout<<"Connect to GUI failed!"<<endl;
          //  exit(1);
            send_sock=-1;
        }
        return send_sock;
    
}
string util::parse_batch_file(string file_name){
         int node_num=0;
         int power_line_num=0;
         int server_num=0;
         ifstream in_file;
         string data;
         in_file.open ( file_name.c_str());
         if(in_file.fail()==true){
             util::log("batch input file wrong!");
             return data;
         }
         char entry_buffer[500];
         char data_buffer[SOCKET_BUFFER_SIZE];
         bzero(data_buffer,SOCKET_BUFFER_SIZE);
         std::ostringstream oss(data_buffer);
         std::ostringstream entry(entry_buffer);
         string line1;
         while (getline(in_file,line1)){
            
             if(line1 != ""){
                 entry<<line1<<endl;                 
             }
             else{
                 //cout<<entry.str();
                 string s=parse_entry(entry.str());
                 if(s!=""){
                   oss<<s<<endl;
                   if(s[0]=='n'){
                       node_num++;
                   }
                   if(s[0]=='p'){
                       power_line_num++;
                   }
                   if(s[0]=='s'){
                       server_num++;
                   }
                 }
                 entry.clear();
                 entry.str("");
             }
           
         }
         in_file.close();
         //append server information
       //  oss<<"s:localhost:localhost:10000"<<endl;
         char temp[SOCKET_BUFFER_SIZE];
         bzero(temp,SOCKET_BUFFER_SIZE);
         std::ostringstream oss1(temp);
         oss1<<node_num<<":"<<power_line_num<<":"<<server_num<<endl;
         oss1<<oss.str();
         data=oss1.str();
         return data;
}
void util::log(string log){
    if(LOGFLAG==TO_CONSOLE){
        cout<<log<<endl;
    }
    if(LOGFLAG==TO_LOGFILE){   
     util::logFile.log_file<<log<<endl;
    }
}

