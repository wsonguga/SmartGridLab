#include "coordinator.h"
//send the request to each slave servers.
void* process_request(void* param){
Coordinator * co = (Coordinator *) param;
        int send_sock;
        struct hostent *host1;
        struct sockaddr_in server_addr1;
        Request rr;
  while(true){
     if(co->is_ok_next==true){
         co->is_ok_next=false;
      (*co).cq.wait_and_pop(rr);
      cout<<rr.server_ip<<endl;
      host1 = gethostbyname(rr.server_ip.c_str());
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
           printf("Connect failed!!! PowerSimulator may shutdown.\n");
           exit(1);
        }
         char result_buffer1[1024];
         bzero(result_buffer1,1024);
         std::ostringstream oss1(result_buffer1);
         oss1<<"DynamicConnectionInfo:"<<endl;
         for(int i=0;i<rr.dcs.size();i++)
           oss1<<rr.coordinator_ip.c_str()<<":"<<rr.dcs[i].remote_ip.c_str()<<":"<<rr.dcs[i].remote_pi.c_str()<<":"<<rr.dcs[i].local_pi.c_str()<<endl;
         string data=oss1.str();
         co->sendData(send_sock,data,1024);       
         close(send_sock);  
      }
      sleep(1);
   }
}