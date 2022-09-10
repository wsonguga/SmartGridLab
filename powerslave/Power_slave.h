/* 
 * File:   Power_slave.h
 * Author: stan
 *
 * Created on March 28, 2012, 10:30 AM
 */

#ifndef POWER_SLAVE_H
#define	POWER_SLAVE_H
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>
#include <map>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define BUFFER_SIZE 20000
#define MAXIMUM_PACKET_SIZE 1024
#define DISTRIBUTED_BUFFER_SIZE 20000
using namespace std;
class Power_slave{
private:
    char *first_phase_data;
    char *second_phase_data;
    gsl_matrix* Y_b;
    gsl_matrix* N;
    gsl_vector* I;
    gsl_vector* I_x;
    gsl_matrix* Y_b_inv;
    gsl_matrix* N_TY_b_inv;
    int n;
    int l;
public:
    void init_first_phase(char* data){
          this->first_phase_data=data;
          Y_b=NULL;
          N=NULL;
          I=NULL;
          I_x=NULL;
          Y_b_inv=NULL;
          N_TY_b_inv=NULL;
    }
    void parse_first_phase(){
          std::istringstream iss(first_phase_data);
        string line;
          getline(iss,line);
         string value;
         // get the node number and link number;
         istringstream ssline(line);
         int field=0;
         while (getline ( ssline, value, ':' )){
             if(field==0){
                 n=atoi(value.c_str());
             }
             if(field==1){
                 l=atoi(value.c_str());
             }
             field++;
         }
         Y_b=gsl_matrix_alloc(n,n);
         gsl_matrix_set_zero(Y_b);
         N=gsl_matrix_alloc(n,l);
         gsl_matrix_set_zero(N);
         I=gsl_vector_alloc(n);
         gsl_vector_set_zero(I);
         Y_b_inv=gsl_matrix_alloc(n,n);
         gsl_matrix_set_zero(Y_b_inv);
         N_TY_b_inv=gsl_matrix_alloc(l,n);
         gsl_matrix_set_zero(N_TY_b_inv);
         while ( getline(iss,line)){
             istringstream ssline(line);
             int field=0;
             int flag=-1;
             int row=-1,col=-1;
             while (getline (ssline,value,':')){
                 if(field==0){
                     if(value=="Yb"){
                         flag=0;
                     }
                     if(value=="N"){
                         flag=1;
                     }
                     if(value=="I"){
                         flag=2;
                     }
                     field++;
                     continue;
                 }
                 if(flag==0){
                    
                     if(field==1)
                         row=atoi(value.c_str());
                     if(field==2)
                         col=atoi(value.c_str());
                     if(field==3){
                         gsl_matrix_set(Y_b,row,col,atof(value.c_str()));
                     }
                     field++;                         
                 }
                 if(flag==1){
                 
                     if(field==1)
                         row=atoi(value.c_str());
                     if(field==2)
                         col=atoi(value.c_str());
                     if(field==3){
                         gsl_matrix_set(N,row,col,atof(value.c_str()));
                     }
                     field++;    
                 }
                 if(flag==2){
              
                     if(field==1)
                         row=atoi(value.c_str());
                     if(field==2)
                         gsl_vector_set(I,row,atof(value.c_str()));
                     field++;
                 }
                            
             }
          
         }
         
         
            
       
    }
    void init_second_phase(char* data){
        this->second_phase_data=data;
    }
    void parse_second_phase(){
         I_x=gsl_vector_alloc(n);
         gsl_vector_set_zero(I_x);
         std::istringstream iss(second_phase_data);
         string line;
         string value;
         while(getline(iss,line)){
               istringstream ssline(line);
               int field=0;
               int row=-1;
               while (getline (ssline,value,':')){
                   
                   if(field==1){
                       row=atoi(value.c_str());
                   }
                   if(field==2){
                       gsl_vector_set(I_x,row,atof(value.c_str()));
                   }
                   
                   
                   field++;
               }
               
         }
         
       
        
    }
    string execute_first_phase(){
       cout<<"Execute first phase:"<<endl;
       cout<<"Matrix Y_b:"<<endl;
       for(int i=0;i<n;i++){
           for(int j=0;j<n;j++)
           cout<<gsl_matrix_get(Y_b,i,j)<<" ";
        cout<<endl;
       }
         cout<<"Matrix N:"<<endl;
       for(int i=0;i<n;i++){
           for(int j=0;j<l;j++){
               double value=gsl_matrix_get(N,i,j);
               if(value<0.0000001 && value>-0.0000001)
                   gsl_matrix_set(N,i,j,0);
               if(value>999999 || value<-999999)
                   gsl_matrix_set(N,i,j,0);
           cout<<gsl_matrix_get(N,i,j)<<" ";
           }
        cout<<endl;
       }        
         cout<<"Vector I:"<<endl;
       for(int i=0;i<n;i++){
           double value=gsl_vector_get(I,i);
           if(value<0.0000001 && value>-0.0000001)
               gsl_vector_set(I,i,0);
           if(value>999999.0 || value<-999999.0)
                gsl_vector_set(I,i,0);
        cout<<gsl_vector_get(I,i)<<endl;
       }
         
         //using LU to compute inversion
       gsl_permutation * p = gsl_permutation_alloc(n);
       int signum;
     //   gsl_matrix* Y_b_temp=gsl_matrix_alloc(n,n);
      //  gsl_matrix_memcpy(Y_b_temp,Y_b);
       
       gsl_linalg_LU_decomp (Y_b, p, &signum);
       gsl_linalg_LU_invert (Y_b, p, Y_b_inv);
       gsl_permutation_free(p);  
     //  gsl_matrix_free(Y_b_temp);
      
       gsl_blas_dgemm(CblasTrans,CblasNoTrans,1, N,Y_b_inv,0,N_TY_b_inv);
       gsl_matrix* N_TY_b_invN=gsl_matrix_alloc(l,l);
       gsl_vector* N_TY_b_invI=gsl_vector_alloc(l);
       gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1, N_TY_b_inv,N,0,N_TY_b_invN);
       gsl_blas_dgemv (CblasNoTrans,1,N_TY_b_inv,I,0, N_TY_b_invI);
     //  gsl_matrix_free(N_TY_b_inv);
       char first_phase_result[DISTRIBUTED_BUFFER_SIZE];
            //the buffer may overflow
       std::ostringstream oss1(first_phase_result);
       cout<<"First part of First Phase:"<<endl;
       for(int i=0;i<l;i++){
           for(int j=0;j<l;j++){
               double value=gsl_matrix_get(N_TY_b_invN,i,j);
               if(value<0.0000001 && value>-0.0000001){
                   gsl_matrix_set(N_TY_b_invN,i,j,0);
                   value=0;
               }
               if(value>999999 || value<-999999){
                   gsl_matrix_set(N_TY_b_invN,i,j,0);
                   value=0;
               }
               if(value!=0){
                   oss1<<"1:"<<i<<":"<<j<<":"<<value<<endl;
               }
               cout<<gsl_matrix_get(N_TY_b_invN,i,j)<<",";
           }
           cout<<endl;
       }
       cout<<"Second part of First Phase:"<<endl;
       for(int i=0;i<l;i++){
           double value=gsl_vector_get(N_TY_b_invI,i);
           if(value<0.0000001 && value > -0.0000001){
               gsl_vector_set(N_TY_b_invI,i,0);
               value=0;
           }
           if(value>999999 || value <-999999){
               gsl_vector_set(N_TY_b_invI,i,0);
               value=0;
           }
           if(value!=0){
                 oss1<<"2:"<<i<<":"<<value<<endl;
           }
           cout<<gsl_vector_get(N_TY_b_invI,i)<<endl;
          
       }        
       gsl_matrix_free(N_TY_b_invN);
       gsl_vector_free(N_TY_b_invI);
         
       return oss1.str();
    }
    string execute_second_phase(){
        cout<<"Execute second phase:"<<endl;
        cout<<"I_x:"<<endl;
        for(int i=0;i<n;i++){
            cout<<gsl_vector_get(I_x,i)<<endl;
        }
        gsl_vector* U=gsl_vector_alloc(n);
        gsl_blas_dgemv (CblasNoTrans,1,Y_b_inv,I_x,0, U);
         char second_phase_result[DISTRIBUTED_BUFFER_SIZE];
            //the buffer may overflow
       std::ostringstream oss2(second_phase_result);
        cout<<"Vector U:"<<endl;
        for(int i=0;i<n;i++){
            double value=gsl_vector_get(U,i);
            if(value<0.0000001 && value>-0.0000001)
                gsl_vector_set(U,i,0);
            if(value>999999 || value <-999999)
                gsl_vector_set(U,i,0);
            cout<<gsl_vector_get(U,i)<<endl;
            oss2<<gsl_vector_get(U,i)<<endl;
        }
        gsl_vector_free(U);
        
        gsl_matrix_free(Y_b);
        gsl_matrix_free(N);
      //  gsl_vector_free(I);
      //  gsl_vector_free(I_x);
        return oss2.str();
    }
    string update_first_phase(string data){
          std::istringstream iss(data);
          string line;
          string value;
            while ( getline(iss,line)){
             istringstream ssline(line);
             int field=0;
             int row=-1;
             while (getline (ssline,value,':')){
                 if(field==0){
                     if (value!="I")
                         break;
                     
                 }
                 if(field==1)
                     row=atoi(value.c_str());
                 if(field==2)
                     gsl_vector_set(I,row,atof(value.c_str()));
                 field++;            
             }
          
           }
          gsl_vector* N_TY_b_invI=gsl_vector_alloc(l);
          gsl_blas_dgemv (CblasNoTrans,1,N_TY_b_inv,I,0, N_TY_b_invI);
          char first_phase_update_result[DISTRIBUTED_BUFFER_SIZE];
            //the buffer may overflow
          cout<<"Update first phase result:"<<endl;
          std::ostringstream oss1(first_phase_update_result);
          for(int i=0;i<l;i++){
           double value=gsl_vector_get(N_TY_b_invI,i);
           if(value<0.0000001 && value > -0.0000001){
               gsl_vector_set(N_TY_b_invI,i,0);
               value=0;
           }
           if(value>999999 || value <-999999){
               gsl_vector_set(N_TY_b_invI,i,0);
               value=0;
           }
           if(value!=0){
                 oss1<<"2:"<<i<<":"<<value<<endl;
           }
           cout<<gsl_vector_get(N_TY_b_invI,i)<<endl;
          
          }              
           gsl_vector_free(N_TY_b_invI);
           return oss1.str();
      
    }
    string update_second_phase(string data){
           std::istringstream iss(data);
         string line;
         string value;
         while(getline(iss,line)){
               istringstream ssline(line);
               int field=0;
               int row=-1;
               while (getline (ssline,value,':')){
                   
                   if(field==1){
                       row=atoi(value.c_str());
                   }
                   if(field==2){
                       gsl_vector_set(I_x,row,atof(value.c_str()));
                   }
                   
                   
                   field++;
               }
               
         }
           gsl_vector* U=gsl_vector_alloc(n);
        gsl_blas_dgemv (CblasNoTrans,1,Y_b_inv,I_x,0, U);
         char second_phase_result[DISTRIBUTED_BUFFER_SIZE];
            //the buffer may overflow
       std::ostringstream oss2(second_phase_result);
        cout<<"Updated Vector U:"<<endl;
        for(int i=0;i<n;i++){
            double value=gsl_vector_get(U,i);
            if(value<0.0000001 && value>-0.0000001)
                gsl_vector_set(U,i,0);
            if(value>999999 || value <-999999)
                gsl_vector_set(U,i,0);
            cout<<gsl_vector_get(U,i)<<endl;
            oss2<<gsl_vector_get(U,i)<<endl;
        }
        gsl_vector_free(U);
         
        return oss2.str();
        
        
        
    }
    ~Power_slave(){
     //   gsl_matrix_free(Y_b);
     //   gsl_matrix_free(N);
        gsl_vector_free(I);
        gsl_vector_free(I_x);
        gsl_matrix_free(Y_b_inv);
        gsl_matrix_free(N_TY_b_inv);
    }
};




#endif	/* POWER_SLAVE_H */

