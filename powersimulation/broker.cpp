#include "broker.h"
Broker::Broker(string masterIp){
    this->masterIp=masterIp;
}
void Broker::forward(string data){
        int send_sock;
        struct hostent *host1;
        struct sockaddr_in server_addr1;
        host1 = gethostbyname(masterIp.c_str());
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
            printf("Connect failed!!! Power Master may shutdown.\n");
            exit(1);
        }
        util::send_data(send_sock,data,1024);       
        close(send_sock);    
}


