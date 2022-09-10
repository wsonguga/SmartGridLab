/* 
 * File:   thread_functions.h
 * Author: stan
 *
 * Created on January 8, 2013, 8:54 AM
 */

#ifndef THREAD_FUNCTIONS_H
#define	THREAD_FUNCTIONS_H
#define SAMPLING_TIME_INTERVAL_EMULATOR 1 // seconds
#define SAMPLING_TIME_INTERVAL_TESTBED 2
#define STORAGE_MONITOR_INTERVAL 0.5 //seconds
using namespace std;
void* real_time_report(void* param);

void* storage_monitor(void* param);

void* TunnelSocketServer (void* param);

void* report_to_gui(void* param);

void* testbed_monitor(void* param);

#endif	/* THREAD_FUNCTIONS_H */

