/* 
 * File:   coordinator.h
 * Author: stan
 *
 * Created on January 8, 2013, 8:35 AM
 */

#ifndef COORDINATOR_H
#define	COORDINATOR_H
#include "util_c.h"
#include "host.h"
#include <iostream>
#include <map>
#include <string>
#include <stdio.h>
#include <ctype.h>
#include <sstream>
#include <algorithm>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>
#include "power_interface.h"
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <boost/lockfree/queue.hpp>
#include <boost/thread/thread.hpp>
#include "Request.h"
#include "threads_functions.h"
#include "concurrent_queue.h"
class Coordinator{
private:
    double total_desired_supplies;
    double total_desired_demands;
    double total_supplies;
    double total_demands;
    map<string,Host>hosts;
    int boundary_nodes_number_;
    gsl_matrix* YBB_pre;
    gsl_vector* IB_pre;
    gsl_vector* VB_pre;
    gsl_matrix* YBB;
    gsl_vector* IB;
    gsl_vector* VB;
    map<string,int>pi_index_mapping_;
    map<int,string>index_pi_mapping_;
    map<string,PowerInterface> pis;
    pthread_t process_request_thread;
    
public:
    concurrent_queue<Request> cq;
    bool is_ok_next;
    Coordinator():cq(){
        total_supplies=0;
        total_demands=0;
        total_desired_supplies=0;
        total_desired_demands=0;
        boundary_nodes_number_=0;
        YBB=NULL;
        IB=NULL;
        VB=NULL;
        YBB_pre=NULL;
        IB_pre=NULL;
        VB_pre=NULL;
        is_ok_next=true;
    }
    string LoadUpdate(string data);
    string Coordinate(string data);
    string CoordinateUpdate(string data);
    void sendData(int sock,string data,int max_packet_size);
    void RegisterRequest(string data);
    void UnRegisterRequest(string data);
    void ProcessRequest();
    
};


#endif	/* COORDINATOR_H */

