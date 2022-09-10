#include "coordinator.h"
using namespace std;
string Coordinator::LoadUpdate(string data){
     string line;
    std::istringstream iss(data);
    getline(iss,line);
    string value;
    std::istringstream ssline(line);
    int field=0;
    string hostIp;
    double desired_demands;
    double desired_supplies;
    double demands;
    double supplies;
    while (getline ( ssline, value, ':' )){
        if(field==0)
            hostIp=value;
        if(field==1)
            desired_demands=atof(value.c_str());
        if(field==2)
            desired_supplies=atof(value.c_str());
        field++;
    }
     map<string,Host>::iterator it1;
     it1=hosts.find(hostIp);
     if (it1!= hosts.end()){
         total_desired_demands=total_desired_demands-it1->second.desired_demands+desired_demands;
         total_desired_supplies=total_desired_supplies-it1->second.desired_supplies+desired_supplies;
         it1->second.desired_demands=desired_demands;
         it1->second.desired_supplies=desired_supplies;
     }
     else{
         total_desired_demands=total_desired_demands+desired_demands;
         total_desired_supplies=total_desired_supplies+desired_supplies;
         Host h;
         h.desired_demands=desired_demands;
         h.desired_supplies=desired_supplies;
         h.ip=hostIp;
         hosts[hostIp]=h;
     }
     
     int flag;
     if(total_desired_demands>total_desired_supplies){
         total_supplies=total_desired_supplies;
         total_demands=total_desired_supplies;
         flag=0;
     }
     else{
         total_demands=total_desired_demands;
         total_supplies=total_desired_demands;
         flag=1;
     }
     
     map<string, Host>::iterator it;
     for(it=hosts.begin();it!=hosts.end();it++){
         if(flag==0){
             it->second.supplies=it->second.desired_supplies;
             it->second.demands=it->second.desired_demands/total_desired_demands*total_demands;
             if(hostIp==it->second.ip){
                 demands=it->second.demands;
                 supplies=it->second.supplies;
             }
             else{
                 // send updates to the other hosts;
                 double temp1=it->second.supplies;
                 double temp2=it->second.demands;
                 ostringstream strs;
                 strs <<flag<<":"<<temp2<<":"<<temp1;
                 string values = strs.str();
                 int send_sock;
                 struct hostent *host1;
                 struct sockaddr_in server_addr1;
                 host1 = gethostbyname(it->second.ip.c_str());
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
                 string data="Coordinating update:"+values+"\r\n";
                 sendData(send_sock,data,1024);       
                 close(send_sock);    
                 
                 
                 
             }
         }
         if(flag==1){
             it->second.demands=it->second.desired_demands;
             it->second.supplies=it->second.desired_supplies/total_desired_supplies*total_supplies;
             if(hostIp==it->second.ip){
                 demands=it->second.demands;
                 supplies=it->second.supplies;
             }
             else{
                 // send updates to the other hosts;
                      double temp1=it->second.supplies;
                 double temp2=it->second.demands;
                 ostringstream strs;
                 strs <<flag<<":"<<temp2<<":"<<temp1;
                 string values = strs.str();
                 int send_sock;
                 struct hostent *host1;
                 struct sockaddr_in server_addr1;
                 host1 = gethostbyname(it->second.ip.c_str());
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
                 string data="Coordinating update:"+values+"\r\n";
                 sendData(send_sock,data,1024);       
                 close(send_sock);    
                 
                 
             }
         }
         
     }
    cout<<"total desired supplies:"<<total_desired_supplies<<endl;
    cout<<"total desired demands: "<<total_desired_demands<<endl;
    cout<<"total demands:"<<total_demands<<endl;
    //The coordinator needs to wait the existing ones' update results before sending 
    //to the new ones. 
    sleep(2);
    char result_buffer[100];
    bzero(result_buffer,100);
    std::ostringstream oss(result_buffer);
    oss<<flag<<":"<<demands<<":"<<supplies<<endl; // the demands and the supplies are equal.
    return oss.str();
}
string Coordinator::Coordinate(string data){
    string line;
    std::istringstream iss(data);
    getline(iss,line);
    string value;
    string host_ip;
    int boundary_node;
    std::istringstream ssline(line);
    int field=0;
    while (getline ( ssline, value, ':' )){
        if(field==0)
            host_ip=value;
        if(field==1)
            boundary_node=atoi(value.c_str());
        field++;
    }
  
    map<string,Host>::iterator it;
    it=hosts.find(host_ip);
    if(it!=hosts.end()){
        it->second.boundary_node_num_=boundary_node;
        it->second.start_index_=boundary_nodes_number_;
    }
    int original_dimension=boundary_nodes_number_;
    boundary_nodes_number_+=boundary_node;
    
    
    if(original_dimension==0){
        YBB=gsl_matrix_alloc(boundary_node,boundary_node);
        VB=gsl_vector_alloc(boundary_node);
        IB=gsl_vector_alloc(boundary_node);
         gsl_matrix_set_zero(YBB);
          gsl_vector_set_zero(VB);
           gsl_vector_set_zero(IB);
        YBB_pre=YBB;
        VB_pre=VB;
        IB_pre=IB;
    }
    else{
       
        YBB=gsl_matrix_alloc(boundary_nodes_number_,boundary_nodes_number_);
        VB=gsl_vector_alloc(boundary_nodes_number_);
        IB=gsl_vector_alloc(boundary_nodes_number_);
          gsl_matrix_set_zero(YBB);
          gsl_vector_set_zero(VB);
           gsl_vector_set_zero(IB);
        for(int j=0;j<original_dimension;j++){
            for(int k=0;k<original_dimension;k++)
               gsl_matrix_set(YBB,j,k,gsl_matrix_get(YBB_pre,j,k));
            gsl_vector_set(VB,j,gsl_vector_get(VB_pre,j));
            gsl_vector_set(IB,j,gsl_vector_get(IB_pre,j));
        }
        
        gsl_matrix_free(YBB_pre);
        gsl_vector_free(VB_pre);
        gsl_vector_free(IB_pre);
        YBB_pre=YBB;
        VB_pre=VB;
        IB_pre=IB;
    }
    for(int i=0;i<boundary_node;i++){
        getline(iss,line);
         std::istringstream ssline(line);
         int field=0;
         string value;
         
         //assume there is only one of it. one to one connection. one target only
         string target_host1;
         string target_pi1;
         string target;
         PowerInterface pi;
     while (getline ( ssline, value, ':' )){
        if(field==0){
            pi.id=host_ip+value;
        }
        if(field==1){
            pi.original_diagonal_entry_=atof(value.c_str());
            
        }
        if(field==2){
            target_host1=value;
        }
        if(field==3){
            target_pi1=value;
        }
        
        
        field++;
     }
         
       target=target_host1+target_pi1;
       pi_index_mapping_[pi.id]=original_dimension+i;
       index_pi_mapping_[original_dimension+i]=pi.id;
       pi.neighbor_list_.push_back(target);
       pis[pi.id]=pi;
       
     
       
       double target_entry=0;
       map<string,int>::iterator it;
       it=pi_index_mapping_.find(target);
       map<string,PowerInterface>::iterator it1;
       it1=pis.find(target);
       if(it1!=pis.end()){
           target_entry=it1->second.original_diagonal_entry_;
       }
       if(it!=pi_index_mapping_.end()){
           int s=it->second;
           //assume resistor all -1, YBB keeps updating after filling up with deltaX. It is not like
           // the original one any more
           gsl_matrix_set(YBB,s,original_dimension+i,-1);
           gsl_matrix_set(YBB,original_dimension+i,s,-1);
           gsl_matrix_set(YBB,s,s,gsl_matrix_get(YBB,s,s)+1);
           gsl_matrix_set(YBB,original_dimension+i,original_dimension+i,pi.original_diagonal_entry_+1);
           
       }
       else{
           gsl_matrix_set(YBB,original_dimension+i,original_dimension+i,pi.original_diagonal_entry_);
       } 
    }
    
    cout<<"boundary node number: "<<boundary_nodes_number_<<endl;
       cout<<"YBB"<<endl;
       for(int i=0;i<boundary_nodes_number_;i++){
           for(int j=0;j<boundary_nodes_number_;j++)
              cout<<gsl_matrix_get(YBB,i,j)<<":";
           cout<<endl;
       }
       cout<<"pi index mapping:"<<endl;
       map<string,int>::iterator it2;
       cout<<pi_index_mapping_.size()<<endl;
       for(it2=pi_index_mapping_.begin();it2!=pi_index_mapping_.end();it2++)
           cout<<it2->first<<":"<<it2->second<<endl;
       cout<<"pis:"<<endl;
       map<string,PowerInterface>::iterator it3;
       for(it3=pis.begin();it3!=pis.end();it3++)
           cout<<it3->first<<":"<<it3->second.neighbor_list_.back()<<endl; 
       
       
     //fill up the matrix YBB using deltaX;  
     for(int i=0;i<boundary_node;i++){
            getline(iss,line);
            std::istringstream ssline(line);
            int field=0;
            string value;
        while (getline ( ssline, value, ':' )){
            if(value!=""){
            gsl_matrix_set(YBB,original_dimension+i,original_dimension+field,
                    gsl_matrix_get(YBB,original_dimension+i,original_dimension+field)-atof(value.c_str()));
           
                    
            field++;  
            }
         
        }
    }
           cout<<"New YBB"<<endl;
       for(int i=0;i<boundary_nodes_number_;i++){
           for(int j=0;j<boundary_nodes_number_;j++)
              cout<<gsl_matrix_get(YBB,i,j)<<":";
           cout<<endl;
       }
       
           //fill up IB;      assume the injection value of boundary node is 0;
     for(int i=0;i<boundary_node;i++){
            getline(iss,line);
            gsl_vector_set(IB,original_dimension+i,-1*atof(line.c_str()));                 
     }
           cout<<"New IB:"<<endl;
           for(int i=0;i<boundary_nodes_number_;i++){
               cout<<gsl_vector_get(IB,i)<<endl;
           }
           
           
     //calculate VB;    
     string result;
       if(boundary_nodes_number_==1){
               result="0";
               is_ok_next=true;
               return result;
       }
      gsl_matrix* YBB_temp=gsl_matrix_alloc(boundary_nodes_number_,boundary_nodes_number_);
      gsl_matrix_memcpy(YBB_temp,YBB);
      
      gsl_matrix_view YBB_temp1=gsl_matrix_submatrix(YBB_temp,0,0,boundary_nodes_number_-1
              ,boundary_nodes_number_-1);
      gsl_permutation * p = gsl_permutation_alloc(boundary_nodes_number_-1);
      int signum;
      gsl_linalg_LU_decomp (&YBB_temp1.matrix, p, &signum);
      double det=gsl_linalg_LU_det(&YBB_temp1.matrix,signum);
      cout<<"determinant of YBB1: "<<det<<endl;
      gsl_vector_view VB1=gsl_vector_subvector(VB,0,boundary_nodes_number_-1);
      gsl_vector_view IB1=gsl_vector_subvector(IB,0,boundary_nodes_number_-1);
      if(det!=0){
           gsl_linalg_LU_solve (&YBB_temp1.matrix, p, &IB1.vector, &VB1.vector);
      }
      gsl_matrix_free(YBB_temp);
     for(int i=0;i<boundary_nodes_number_;i++){
         cout<<"after coordination, VB:"<<endl;
         cout<<gsl_vector_get(VB,i)<<endl;
     }       
    
   map<string, Host>::iterator it5;
 
   for(it5=hosts.begin();it5!=hosts.end();it5++){
            char result_buffer[100];
           bzero(result_buffer,100);
            std::ostringstream oss(result_buffer);
     for(int i=it5->second.start_index_;i<it5->second.start_index_+it5->second.boundary_node_num_;i++){
         //boundary node voltage and corresponding power line current.
         map<int,string>::iterator itss;
         itss=index_pi_mapping_.find(i);
         string pi_temp=itss->second;
         map<string,PowerInterface>::iterator its1;
         its1=pis.find(pi_temp);
         string neighbor_pi=its1->second.neighbor_list_[0]; //assume one neighbor only
         map<string,int>::iterator its2;
         its2=pi_index_mapping_.find(neighbor_pi);
         
         double temp1,temp2,current;
         if(its2!=pi_index_mapping_.end()){
         int neighbor_index=its2->second;
           temp2=gsl_vector_get(VB,neighbor_index);
           temp1=gsl_vector_get(VB,i);
           current=temp2-temp1/1.0;
         }else{
             current=0;
         }
        
         
         oss<<gsl_vector_get(VB,i)<<":"<<current<<endl;
     }
         if(it5->first==host_ip){
             result=oss.str();
             cout<<"Result for initiated host:"<<result<<endl;
         }
         else{
         //send to all the other hosts; oss.str();
        
                 int send_sock;
                 struct hostent *host1;
                 struct sockaddr_in server_addr1;
                 host1 = gethostbyname(it5->second.ip.c_str());
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
                     char result_buffer1[100];
                    bzero(result_buffer1,100);
                 std::ostringstream oss1(result_buffer1);
                 oss1<<"boundary voltage and power line current:"<<endl;
                 oss1<<oss.str();
                 string data=oss1.str();
                 sendData(send_sock,data,1024);       
                 close(send_sock);    
                 
                 
             
         }
     
                
   }
    
  
               
   is_ok_next=true;
    return result;
}
void Coordinator::sendData(int sock,string data,int max_packet_size){
        long int bytes_left=data.length();
        long int packet_num=0;
        long int n=0;
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
     shutdown(sock, SHUT_WR);       
        
} 

string Coordinator::CoordinateUpdate(string data){
    cout<<"CoordinateUpdate:"<<endl;
     string line;
    std::istringstream iss(data);
    getline(iss,line);
    string value;
    string host_ip;
    int boundary_node=0;
    std::istringstream ssline(line);
    int field=0;
    while (getline ( ssline, value, ':' )){
        if(field==0)
            host_ip=value;
        if(field==1)
            boundary_node=atoi(value.c_str());
        field++;
    }
  
    map<string,Host>::iterator it;
    it=hosts.find(host_ip);
    string result;
    if(it!=hosts.end())
    {
       int start_index=it->second.start_index_;
       int sstemp= (*IB).size;
      for(int i=start_index;i<start_index+boundary_node;i++){
          if(i<sstemp){
             getline(iss,line);
             gsl_vector_set(IB,i,-1*atof(line.c_str()));
          }
       
      }
    
    
       if(boundary_nodes_number_<=1){
         if(hosts.size()==1){
           map<string, Host>::iterator it7;
           
         for(it7=hosts.begin();it7!=hosts.end();it7++){
            char result_buffer[100];
           bzero(result_buffer,100);
            std::ostringstream oss2(result_buffer);
            oss2<<"boundary voltage and power line current:"<<endl;
             oss2<<"0"<<":"<<"0"<<endl;
              int send_sock;
                 struct hostent *host1;
                 struct sockaddr_in server_addr1;
                 host1 = gethostbyname(it7->second.ip.c_str());
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
                 string data=oss2.str();
                 sendData(send_sock,data,1024);       
                 close(send_sock);     
           }   
          }
               result="0";
               return result;
       }
      gsl_matrix* YBB_temp=gsl_matrix_alloc(boundary_nodes_number_,boundary_nodes_number_);
      gsl_matrix_memcpy(YBB_temp,YBB);
      
      gsl_matrix_view YBB_temp1=gsl_matrix_submatrix(YBB_temp,0,0,boundary_nodes_number_-1
              ,boundary_nodes_number_-1);
      gsl_permutation * p = gsl_permutation_alloc(boundary_nodes_number_-1);
      int signum;
      gsl_linalg_LU_decomp (&YBB_temp1.matrix, p, &signum);
      double det=gsl_linalg_LU_det(&YBB_temp1.matrix,signum);
      cout<<"determinant of YBB1: "<<det<<endl;
      gsl_vector_set_zero(VB);
      gsl_vector_view VB1=gsl_vector_subvector(VB,0,boundary_nodes_number_-1);
      gsl_vector_view IB1=gsl_vector_subvector(IB,0,boundary_nodes_number_-1);
      if(det!=0){
           gsl_linalg_LU_solve (&YBB_temp1.matrix, p, &IB1.vector, &VB1.vector);
      }
      gsl_matrix_free(YBB_temp);
   
      char result_buffer[100];
           bzero(result_buffer,100);
            std::ostringstream oss(result_buffer);
     for(int i=0;i<boundary_nodes_number_;i++){
         
         cout<<gsl_vector_get(VB,i)<<endl;
     }       
    //for(int i=start_index;i<start_index+boundary_node;i++)
    //  oss<<gsl_vector_get(VB,i)<<endl;
            
            
            
            
            
      map<string, Host>::iterator it5;
 
   for(it5=hosts.begin();it5!=hosts.end();it5++){
            char result_buffer[100];
           bzero(result_buffer,100);
            std::ostringstream oss(result_buffer);
     for(int i=it5->second.start_index_;i<it5->second.start_index_+it5->second.boundary_node_num_;i++){
         //boundary node voltage and corresponding power line current.
         map<int,string>::iterator itss;
         itss=index_pi_mapping_.find(i);
         string pi_temp=itss->second;
         map<string,PowerInterface>::iterator its1;
         its1=pis.find(pi_temp);
         string neighbor_pi=its1->second.neighbor_list_[0]; //assume one neighbor only
         map<string,int>::iterator its2;
         its2=pi_index_mapping_.find(neighbor_pi);
         
         double temp1,temp2,current;
         if(its2!=pi_index_mapping_.end()){
         int neighbor_index=its2->second;
           temp2=gsl_vector_get(VB,neighbor_index);
           temp1=gsl_vector_get(VB,i);
           current=temp2-temp1/1.0;
         }else{
             current=0;
         }
         
         oss<<gsl_vector_get(VB,i)<<":"<<current<<endl;
     
        
                 int send_sock;
                 struct hostent *host1;
                 struct sockaddr_in server_addr1;
                 host1 = gethostbyname(it5->second.ip.c_str());
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
                     char result_buffer1[100];
                    bzero(result_buffer1,100);
                 std::ostringstream oss1(result_buffer1);
                 oss1<<"boundary voltage and power line current:"<<endl;
                 oss1<<oss.str();
                 string data=oss1.str();
                 sendData(send_sock,data,1024);       
                 close(send_sock);    
                 
                 
             
         
     }        
   }
  }
            
    return result;
}
void Coordinator::RegisterRequest(string data){
     std::istringstream iss(data);
     string line;
      Request re;
      string value;
     while(getline(iss,line)){
         std::istringstream ssline(line);
         int field=0;
         DynamicConnection dc;
         while (getline ( ssline, value, ':' )){
             if(field==0)
                re.server_ip=value;
             if(field==1)
                re.coordinator_ip=value;
             if(field==2)
                dc.remote_ip=value;
             if(field==3)
                dc.remote_pi=value;
             if(field==4)
                dc.local_pi=value;
             field++;
         }
         re.dcs.push_back(dc);
     }
    cq.push(re);
}
void Coordinator::ProcessRequest(){
     int process_request_thread_create;
     process_request_thread_create = pthread_create(&process_request_thread, NULL, &process_request, this);
     pthread_detach(process_request_thread);
}

// Remove the corresponding part for the disconnected instance.
// The whole instance is gone!! not just a single boundary node.
// Assume one boundary node per instance.
void Coordinator::UnRegisterRequest(string data){
    string line;
    string hostIp, power_interface;
    std::istringstream iss(data);
    vector<string>host_pis;
    while(getline(iss,line)){
       line.erase(line.find_last_not_of(" \n\r\t")+1);
       string value;
       std::istringstream ssline(line);
       int field=0;
       while (getline ( ssline, value, ':' )){
         if(field==0)
            hostIp=value;
         if(field==1)
            power_interface=value;
         field++;
       }
       string target=hostIp+power_interface;
       host_pis.push_back(target);
    }
    
     map<string,Host>::iterator it1;
     it1=hosts.find(hostIp);
     int flag;
     if (it1!= hosts.end()){
         total_desired_demands=total_desired_demands-it1->second.desired_demands;
         total_desired_supplies=total_desired_supplies-it1->second.desired_supplies;   
         if(total_desired_demands>total_desired_supplies){
            total_supplies=total_desired_supplies;
            total_demands=total_desired_supplies;
            flag=0;
         }
         else{
            total_demands=total_desired_demands;
            total_supplies=total_desired_demands;
            flag=1;
         }
         // update the coordinate data structure;
         // 1. update previous matrix entry due to disconnection
         for(int i=0;i<host_pis.size();i++){
          map<string,PowerInterface>::iterator itt;
          itt=pis.find(host_pis[i]);
          if(itt!=pis.end()){
           for(int i=0;i<itt->second.neighbor_list_.size();i++){
               string neighbor=itt->second.neighbor_list_[i];
                map<string,int>::iterator it;
                it=pi_index_mapping_.find(neighbor);
                int s=it->second;
                gsl_matrix_set(YBB,s,s,gsl_matrix_get(YBB,s,s)-1);
           }
          }
         }
         // 2. instantiate a new matrix, copy the value to the new one.
         int start=it1->second.start_index_;
         int length=it1->second.boundary_node_num_;
         int new_boundary_nodes_number_= boundary_nodes_number_-length;
         if(new_boundary_nodes_number_<=0){
             total_supplies=0;
             total_demands=0;
             total_desired_supplies=0;
             total_desired_demands=0;
             boundary_nodes_number_=0;
             if(YBB != NULL){
             gsl_matrix_free(YBB);
             }
             if(IB != NULL){
             gsl_vector_free(IB);
             }
             if(VB != NULL){
             gsl_vector_free(VB);
             }
             // the YBB_pre, IB_pre,VB_pre are freed at the
             // same time automatically, if freed, then double free crash.
             hosts.clear();
             pi_index_mapping_.clear();
             index_pi_mapping_.clear();
             pis.clear();
             is_ok_next=true;
             
         }
         else {   
          // all the following assumes one host one power interface.
         YBB=gsl_matrix_alloc(new_boundary_nodes_number_,new_boundary_nodes_number_);
         VB=gsl_vector_alloc(new_boundary_nodes_number_);
         IB=gsl_vector_alloc(new_boundary_nodes_number_);
         gsl_matrix_set_zero(YBB);
         gsl_vector_set_zero(VB);
         gsl_vector_set_zero(IB);
         for(int i=0;i<start;i++){
             for(int j=0;j<start;j++)
                 gsl_matrix_set(YBB,i,j,gsl_matrix_get(YBB_pre,i,j));
         } 
         for(int i=start+length;i<boundary_nodes_number_;i++){
             for(int j=0;j<start;j++)
                  gsl_matrix_set(YBB,i-length,j,gsl_matrix_get(YBB_pre,i,j));
         }
         for(int i=0;i<start;i++){
             for(int j=start+length;j<boundary_nodes_number_;j++)
                gsl_matrix_set(YBB,i,j-length,gsl_matrix_get(YBB_pre,i,j)); 
         }
         for(int i=start+length;i<boundary_nodes_number_;i++){
             for(int j=start+length;j<boundary_nodes_number_;j++)
                  gsl_matrix_set(YBB,i-length,j-length,gsl_matrix_get(YBB_pre,i,j));
         }    
         boundary_nodes_number_=new_boundary_nodes_number_;
         gsl_matrix_free(YBB_pre);
         gsl_vector_free(VB_pre);
         gsl_vector_free(IB_pre);
         YBB_pre=YBB;
         VB_pre=VB;
         IB_pre=IB;
         
         cout<<"YBB after cancel request:"<<endl;
         for(int i=0;i<boundary_nodes_number_;i++){
             for(int j=0;j<boundary_nodes_number_;j++)
                 cout<<gsl_matrix_get(YBB,i,j)<<" ";
             cout<<endl;
         }
             
         
         //3. all the left data structures
         hosts.erase (it1);
         for(int i=0;i<host_pis.size();i++){
            map<string,PowerInterface>::iterator itt;
            itt=pis.find(host_pis[i]);
            pis.erase(itt);
         }
         map<string,int>::iterator it2;
         map<string,int>::iterator it3;
         // here it not assumes one interface one host
         
          it2=pi_index_mapping_.find(host_pis[0]); 
          int index=it2->second;
         
         //update the indexes for all the power interfaces.
          
          for(it3=pi_index_mapping_.begin();it3!=pi_index_mapping_.end();it3++){
             if(it3->second>index){
                 it3->second=it3->second-length; //should replace 1 with length if multiple
             }  
          }     
         for(int i=0;i<host_pis.size();i++){
           it2=pi_index_mapping_.find(host_pis[i]); 
           pi_index_mapping_.erase(it2);
         
         }   
         index_pi_mapping_.clear();
         for(it3=pi_index_mapping_.begin();it3!=pi_index_mapping_.end();it3++){
             index_pi_mapping_[it3->second]=it3->first;
          
         }
               
         
         
         
         
        map<string,Host>::iterator itsss;
        for(itsss=hosts.begin();itsss!=hosts.end();itsss++){
            if(itsss->second.start_index_>start){
                itsss->second.start_index_=itsss->second.start_index_-length;
            }
        }
         
          
         // send the updated loads to all the rest hosts.
         // since only two hosts at the moment, check whether there is
         // just one host left. if so, send run alone signal.
   
           // this else part is actually cannot be reached since there are only two hosts.
           // so it has not been tested whether working or not. 
          map<string, Host>::iterator it;
          for(it=hosts.begin();it!=hosts.end();it++){
             if(flag==0){
                 it->second.supplies=it->second.desired_supplies;
                 it->second.demands=it->second.desired_demands/total_desired_demands*total_demands;
                 // send updates to the other hosts;
                 double temp1=it->second.supplies;
                 double temp2=it->second.demands;
                 ostringstream strs;
                 strs <<flag<<":"<<temp2<<":"<<temp1;
                 string values = strs.str();
                 int send_sock;
                 struct hostent *host1;
                 struct sockaddr_in server_addr1;
                 host1 = gethostbyname(it->second.ip.c_str());
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
                 string data="Coordinating update:"+values+"\r\n";
                 sendData(send_sock,data,1024);       
                 close(send_sock);    
             }
            if(flag==1){
               it->second.demands=it->second.desired_demands;
               it->second.supplies=it->second.desired_supplies/total_desired_supplies*total_supplies;
                 // send updates to the other hosts;
                 double temp1=it->second.supplies;
                 double temp2=it->second.demands;
                 ostringstream strs;
                 strs <<flag<<":"<<temp2<<":"<<temp1;
                 string values = strs.str();
                 int send_sock;
                 struct hostent *host1;
                 struct sockaddr_in server_addr1;
                 host1 = gethostbyname(it->second.ip.c_str());
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
                 string data="Coordinating update:"+values+"\r\n";
                 sendData(send_sock,data,1024);       
                 close(send_sock);       
            }  
           }
         }
        
        
        //notify the canceled host
                 int send_sock;
                 struct hostent *host1;
                 struct sockaddr_in server_addr1;
                 host1 = gethostbyname(hostIp.c_str());
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
                 string data="Unregistered from the coordinator.\r\n";
                 sendData(send_sock,data,1024);       
                 close(send_sock);  
          
     }
     else{
         cout<<"The host is not registered"<<endl;
     }    
}