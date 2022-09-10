/* 
 * File:   main.cpp
 * Author: stan
 *
 * Created on March 28, 2012, 9:39 AM
 */

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
#include "Power_slave.h"
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <cstdlib>
#include <arpa/inet.h>
using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
  void receive_data(int sockfd,char* receive_buffer,long int buffer_size,int max_packet_size);
  void send_data(int,string data,int max_packet_size);
  void daemonize();
  daemonize();
     char receive_buffer[BUFFER_SIZE];
     bzero(receive_buffer,BUFFER_SIZE);
     int sockfd, newsockfd;
        int port=10000;
        socklen_t clilen;
        bzero(receive_buffer,BUFFER_SIZE);
        struct sockaddr_in serv_addr, cli_addr;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
              printf("ERROR opening socket");
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(port);//local server port
        if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0)
              printf("ERROR on binding");
        listen(sockfd,5);
        clilen = sizeof(cli_addr);
        Power_slave* power_slave=NULL;
        Power_slave* pre=NULL;
        string masterIpPre;
        
     while(true){
           cout<<"Ready to receive: "<<endl;   
        newsockfd = accept(sockfd,
                   (struct sockaddr *) &cli_addr,
                   &clilen);
        
        socklen_t len;
        struct sockaddr_storage addr;
        char ipstr[INET6_ADDRSTRLEN];
        len = sizeof addr;
        getpeername(newsockfd, (struct sockaddr*)&addr, &len);

       // deal with both IPv4 and IPv6:
       if (addr.ss_family == AF_INET) {
         struct sockaddr_in *s = (struct sockaddr_in *)&addr;
         inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
       } else { // AF_INET6
         struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
         inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
       }
       printf("Master IP address: %s\n", ipstr);
       string masterIp(ipstr);
       if(masterIp!=masterIpPre && masterIp != "localhost" && masterIp!="127.0.0.1"){
           cout<<"Power master need to be set!"<<endl;
           masterIpPre=masterIp;
              int send_sock;
        struct hostent *host1;
        struct sockaddr_in server_addr1;
        host1 = gethostbyname("localhost");
        if ((send_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            printf("Socket creation failed! ");
            exit(1);
        }

        server_addr1.sin_family = AF_INET;
        server_addr1.sin_port = htons(9009);
        server_addr1.sin_addr = *((struct in_addr *)host1->h_addr);
        bzero(&(server_addr1.sin_zero),8);

        if (connect(send_sock, (struct sockaddr *)&server_addr1,
                    sizeof(struct sockaddr)) == -1)
        {
            printf("Connect failed!!! Local Power Simulator may shutdown.\n");
            exit(1);
        }
        string data="Power master:"+masterIpPre+"\r\n";
           send_data(send_sock,data,MAXIMUM_PACKET_SIZE);
           close(send_sock);
      }

        
        receive_data(newsockfd,receive_buffer,BUFFER_SIZE,MAXIMUM_PACKET_SIZE);
        cout<<"The received data is:"<<endl;
        printf("%s\n",receive_buffer);
     
     string line;
     string first_phase;
     string second_phase;
     std::istringstream iss(receive_buffer);
     getline(iss,line);
     if(line.compare("First Phase:")==0){
         string data(receive_buffer+sizeof("First Phase:"));
         delete pre;         
         power_slave=new Power_slave();
         pre=power_slave;
         (*power_slave).init_first_phase((char*)data.c_str());
         (*power_slave).parse_first_phase();
         first_phase=(*power_slave).execute_first_phase();
         cout<<"First phase result: "<<endl;
         cout<<first_phase;
         bzero(receive_buffer,BUFFER_SIZE);
         send_data(newsockfd,first_phase,MAXIMUM_PACKET_SIZE);
     }
     else if(line.compare("Second Phase:")==0){
         string data(receive_buffer+sizeof("Second Phase:"));
          (*pre).init_second_phase((char*)data.c_str());
         (*pre).parse_second_phase();
         second_phase=(*pre).execute_second_phase();
         cout<<"Second phase result: "<<endl;
         cout<<second_phase;
         send_data(newsockfd,second_phase,MAXIMUM_PACKET_SIZE);
         bzero(receive_buffer,BUFFER_SIZE);
     }
     else if(line.compare("Update First Phase:")==0){
         string data(receive_buffer+sizeof("Update First Phase:"));
         string first_phase_update=(*pre).update_first_phase(data);
         cout<<"First phase update result:"<<endl;
         cout<<first_phase_update;
         send_data(newsockfd,first_phase_update,MAXIMUM_PACKET_SIZE);
         bzero(receive_buffer,BUFFER_SIZE);
     }
     else if(line.compare("Update Second Phase:")==0){
         string data(receive_buffer+sizeof("Update Second Phase:"));
         string second_phase_update=(*pre).update_second_phase(data);
         cout<<"Second phase update result:"<<endl;
         cout<<second_phase_update;
         send_data(newsockfd,second_phase_update,MAXIMUM_PACKET_SIZE);
         bzero(receive_buffer,BUFFER_SIZE);
         
     }
        
         close(newsockfd);
        
     }
        close(sockfd);
        
    return 0;
}
//This function will send large data using socket. After that
//it invokes shutdown to disallow sending through socket,so the other
//side can know data has been received completely. Reading is still ok
//for receiving feedback. The maximum data length is a long int type.
void send_data(int sock,string data,int max_packet_size){
        long int bytes_left=data.length();
        long int packet_num=0;
        long int n=0;
        cout<<"Sending data..."<<endl;
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
       cout<<"Total "<<data.length()<<" bytes have been sent!"<<endl;
        // This closes the sending.
     shutdown(sock, SHUT_WR);       
        
} 
void receive_data(int sockfd,char* receive_buffer,long int receive_buffer_size,
        int max_packet_size){                      
    cout<<"Receiving start..."<<endl;
     long int bytes_received=0;
     int n;
     char packet[max_packet_size];
     while((n = read(sockfd,packet,max_packet_size))>0){
         bytes_received+=n;
         if(bytes_received >=receive_buffer_size){
             cout<<"Receive buffer overflow!"<<endl;
             break;
         }
         for(int i=0;i<n;i++){
            receive_buffer[bytes_received-n+i]=packet[i];
         }
         bzero(packet,max_packet_size);
     }
     cout<<"Total "<<bytes_received<<" bytes have been received!"<<endl;
}

void daemonize(void)
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
