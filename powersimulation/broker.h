/* 
 * File:   broker.h
 * Author: stan
 *
 * Created on January 8, 2013, 8:44 AM
 */

#ifndef BROKER_H
#define	BROKER_H
#include <iostream>
#include "util.h"
#include <unistd.h> 
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h>
#include <iostream>
#include <sstream>
using namespace std;
class Broker{

    string masterIp;
public:
    Broker (string);
    void forward(string data);
    
};


#endif	/* BROKER_H */

