/* 
 * File:   ipaddress.h
 * Author: stan
 *
 * Created on January 15, 2014, 2:09 PM
 */

#ifndef IPADDRESS_H
#define	IPADDRESS_H
#include "forwarder.h"
#include <fstream>
#include <sstream>
#include <string>
using namespace std;
//static int numberOfNodes;
//static string* kIpaddress;
//static string interfaceIp;
//void GetKIpaddress();
    static const string kIpaddress [NUMBER_OF_NODES]={"131.96.131.47","131.96.131.245",
        "131.96.131.61","131.96.131.198",
        "131.96.131.195","131.96.131.44",
        "131.96.131.224","131.96.131.123",
        "131.96.131.52","131.96.131.237",
        "131.96.131.233","131.96.131.251",
        "131.96.131.246","131.96.131.252",
        "131.96.131.205","131.96.131.222",
        "131.96.131.210",
    };
static const string interfaceIp="131.96.131.131";


#endif	/* IPADDRESS_H */