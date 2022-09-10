/* 
 * File:   host.h
 * Author: stan
 *
 * Created on January 8, 2013, 8:33 AM
 */

#ifndef HOST_H
#define	HOST_H
#include <string>
#include <string.h>
using namespace std;
class Host{
public:
    string ip;
    double desired_demands;
    double desired_supplies;
    double demands;
    double supplies;
    int boundary_node_num_;
    int start_index_;

    Host(){
        boundary_node_num_=0;
        start_index_=0;
        desired_demands=0;
        desired_supplies=0;
        demands=0;
        supplies=0;
    }
   
    
};



#endif	/* HOST_H */

