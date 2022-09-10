/* 
 * File:   main.cpp
 * Author: stan
 *
 * Created on January 8, 2013, 8:32 AM
 */

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
#include "coordinator.h"
#include <fstream>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

using namespace std;

Coordinator* pre=NULL;
pthread_mutex_t mutexsum;
void* SocketHandler(void* lp);

int main(int argc, char** argv) {
    
       util::daemonize();
    /*
     Setting up the main socket for the program.
     */
     int sockfd, newsockfd, portno;

     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;   
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
       printf("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = 9017;
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);     
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0)
              printf("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     

     /*
      Thread related data structure. One thread takes care of one session.
      */
     char receive_buffer[SOCKET_BUFFER_SIZE];
     bzero(receive_buffer,SOCKET_BUFFER_SIZE);  
  //   long int n;
  
     pthread_t thread_id=0;
     pthread_mutex_init(&mutexsum, NULL);
     
     pre=new Coordinator();
     (*pre).ProcessRequest();
     while(true){
    
        cout<<"Coordinator stands by:"<<endl;
        
        newsockfd = accept(sockfd,
                 (struct sockaddr *) &cli_addr,
                 &clilen);
        if (newsockfd < 0)
            printf("ERROR on accept");
        else{
          pthread_create(&thread_id,0,&SocketHandler, (void*)&newsockfd );
          pthread_detach(thread_id);
        }
     
     }
     close(sockfd);
     delete pre;
    
    return 0;
}

 void* SocketHandler(void* lp){
     int newsockfd=*(int*)lp;
     long int n;
     char receive_buffer[SOCKET_BUFFER_SIZE];
      bzero(receive_buffer,SOCKET_BUFFER_SIZE);
     n=util::receive_data(newsockfd,receive_buffer,SOCKET_BUFFER_SIZE,MAXIMUM_PACKET_SIZE);   
     if (n < 0)
         printf("ERROR reading from socket");
     printf("Message received:\n");
     printf("*********************************\n");
     printf("%s",receive_buffer);
     printf("*********************************\n");
     string line;
     string result;
     std::istringstream iss(receive_buffer);
      /* 
      * The first received line decides which branches to go.
      */
     getline(iss,line);
     if(line !=""){
       if(line.compare("Desired demands and supplies:")==0){
         string data(receive_buffer+sizeof("Desired demands and supplies:"));
          pthread_mutex_lock (&mutexsum);
         result=(*pre).LoadUpdate(data);
         pthread_mutex_unlock (&mutexsum);
         bzero(receive_buffer,SOCKET_BUFFER_SIZE);
         cout<<"Load has been updated!"<<endl;
         util::send_data(newsockfd,result,MAXIMUM_PACKET_SIZE);
        
       }
       if(line.compare("coordinating info:")==0){
          string data(receive_buffer+sizeof("coordinating info:")); 
          pthread_mutex_lock (&mutexsum);
         result=(*pre).Coordinate(data);
         pthread_mutex_unlock (&mutexsum);
         bzero(receive_buffer,SOCKET_BUFFER_SIZE);
         cout<<"Coordination is done."<<endl;
          util::send_data(newsockfd,result,MAXIMUM_PACKET_SIZE);
       }
        if(line.compare("coordinating info updated:")==0){
          string data(receive_buffer+sizeof("coordinating info updated:")); 
          pthread_mutex_lock (&mutexsum);
         result=(*pre).CoordinateUpdate(data);
         pthread_mutex_unlock (&mutexsum);
         bzero(receive_buffer,SOCKET_BUFFER_SIZE);
         cout<<"Coordinated update is done."<<endl;
          util::send_data(newsockfd,result,MAXIMUM_PACKET_SIZE);
       }
        if(line.compare("CoordinatingRequest:\r")==0){
         string data(receive_buffer+sizeof("CoordinatingRequest:\r"));
         (*pre).RegisterRequest(data);
         bzero(receive_buffer,SOCKET_BUFFER_SIZE);
       }
        if(line.compare("CancelRequest:\r")==0){
         string data(receive_buffer+sizeof("CancelRequest:\r"));
          pthread_mutex_lock (&mutexsum);
         (*pre).UnRegisterRequest(data);
         pthread_mutex_unlock (&mutexsum);
         bzero(receive_buffer,SOCKET_BUFFER_SIZE);
       } 
       
       close(newsockfd);
       }
    
     return 0;
 }
