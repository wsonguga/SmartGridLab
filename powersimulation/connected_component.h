/* 
 * File:   connected_component.h
 * Author: stan
 *
 * Created on January 8, 2013, 8:45 AM
 */

#ifndef CONNECTED_COMPONENT_H
#define	CONNECTED_COMPONENT_H
#include<vector>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>
#include<iostream>
#include "server.h"
#include "power_simulator.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "dynamic_connection.h"
#include "forwarder.h"
using namespace std;
class ConnectedComponent{
private:
    vector <string> connected_component;
    entity *ps;
    int isTestbed;
    gsl_matrix* A; // switch_pi*switch_pi
    gsl_matrix* A_inv;//(switch_pi-1)*(switch_pi-1)
    gsl_matrix* B;
    gsl_vector* U;
    gsl_vector* I;
    // dynamic
    gsl_matrix*A1_inv;
    gsl_matrix* A1B;
    gsl_matrix* AB1_A1_inv;
    //distributed
    gsl_matrix *y_L;
    gsl_matrix *N;
    gsl_matrix* y_L_inv_inv;
    
    
    bool is_distributed;
    bool is_balanced;
    bool is_dynamic_;
    vector<string>component_servers; 
    vector<string>* pis_on_servers;
    int* servers_pis_num;
    int* servers_start_index;
    
    int switch_pi; //number of pi of switches.
    map<string,int> pi_index_mapping;//key is the power interface
    map<int,string> index_pi_mapping;//key is the row index of matrix A
    vector<string> trans_power_line;// power line between switch node, used to initial matrix B.
    vector<string> edging_power_line;// the vector for all the edging power lines between servers.
    vector<string> boundary_set;
    vector<DynamicConnection> dynamic_connections;
    bool load_balancing();
    bool DynamicLoadBalancing();
    void LocalCalculation();
    void DistributedCalculation();
    void DynamicCalculation();
    void DynamicReconstruct();
    string coordinator_ip_;
    double desired_supplies;
    double desired_demands;
public:
    ConnectedComponent();
    ConnectedComponent(PowerSimulator *p, vector<string>& connected_component);
    ConnectedComponent(forwarder *p, vector<string>& connected_component);
    vector<string>& getConnectedComponent();
    void simulate();
    void Update();
    void set_coordinator_ip(string ip);
    string coordinator_ip();
    bool is_dynamic();
    void set_is_dynamic(bool is_dynamic);
    void AddDynamicConnection(DynamicConnection dc);
    void DynamicSimulate();
    void DynamicCoordinatedLoadUpdate(string data);
    void DynamicCoordinatedBoundaryUpdate(string data);
    void DynamicUpdate();
    void restore();
    void ReturnSimulate();
    void RemoveDynamicConnection();
    ~ConnectedComponent(){
        cout<<"deallocate connected component"<<endl;
        gsl_matrix_free(A);
        gsl_matrix_free(A_inv);
        gsl_matrix_free(B);
       // gsl_vector_free(U);
        gsl_vector_free(I);
        gsl_matrix_free(y_L);
        gsl_matrix_free(N);
        gsl_matrix_free(A1_inv);
        gsl_matrix_free(A1B);
        gsl_matrix_free(AB1_A1_inv);
        gsl_matrix_free(y_L_inv_inv);
        delete []servers_pis_num;
        delete []servers_start_index;
        delete []pis_on_servers;
    }
};



#endif	/* CONNECTED_COMPONENT_H */

