/* 
 * File:   util_c.h
 * Author: stan
 *
 * Created on January 8, 2013, 8:36 AM
 */

#ifndef UTIL_C_H
#define	UTIL_C_H
#define MAXIMUM_PACKET_SIZE 1024
#define TO_CONSOLE 0
#define TO_LOGFILE 1
#define LOGFILE "/tmp/PowerSimulator.log"
#define OUT_FILE "/home/stan/programs/android/SCORE/PowerSimulator/result.log"
#define LOGFLAG TO_LOGFILE
#define SOCKET_BUFFER_SIZE 20000
#include<fstream>
using namespace std;
class LogFile{
public:
    ofstream log_file;
    LogFile(){
        log_file.open(LOGFILE);
    }
    ~LogFile(){
        log_file.close();
    }
};
class util{
private:
    static LogFile logFile;
public:
/*
 * daemonize the whole program
 */
static void daemonize(void);
/*
 * parse a single entry in .imn file
 */
static string parse_entry(string entry);
/*
 * send large amount of data through sock.
 */
static void send_data(int sock,string data,int max_packet_size);
/*
 * receive large amount of data.
 */
static long int receive_data(int sock, char* receive_buffer,long int buffer_size, int max_packet_size);
/*
 * send data to power slave.
 */
static string exec_on_server(string host, int port, string data);
/*
 * send data to GUI during updates.
 */
static void send_gui_update(int sock,string update);
/*
 * set up a client socket to a server socket.
 */
static int open_client_socket(string host, int port);
/*
 * parse the batch input file.
 */
static string parse_batch_file(string file_name);

/*
 * Output log, either through console or log file.
 */
static void log(string log);
};


#endif	/* UTIL_C_H */

