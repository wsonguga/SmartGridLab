/* 
 * File:   Request.h
 * Author: stan
 *
 * Created on June 18, 2014, 9:44 AM
 */

#ifndef REQUEST_H
#define	REQUEST_H
#include "dynamic_connection.h"
//one request per host
class Request{
public:
    std::string server_ip;
    std::string coordinator_ip;
    vector<DynamicConnection> dcs;
};

#endif	/* REQUEST_H */

