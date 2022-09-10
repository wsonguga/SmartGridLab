/* 
 * File:   power_interface.h
 * Author: stan
 *
 * Created on January 8, 2013, 8:35 AM
 */

#ifndef POWER_INTERFACE_H
#define	POWER_INTERFACE_H
#include <vector>
using namespace std;
struct PowerInterface{
    string id;
    double original_diagonal_entry_;
    vector<string>neighbor_list_;
    ~PowerInterface(){ neighbor_list_.clear();}
};


#endif	/* POWER_INTERFACE_H */

