/*
 * Main.cpp
 *
 *  Created on: 2015年9月8日
 *      Author: joe
 */

#include<iostream>
using namespace std;

#include "../TCPSocket.h"
#include "../ReadConf.h"

#include <iostream> //test
using namespace std; //test

int main(int argc, char **args){

    ReadConf conf;
    int ret = conf.Init("TCPServer.conf");
    if(ret < 0)
    {
        cout << conf.GetErrMsg() << endl;
    }

    int svr_id = 0;
    conf.GetValue("TCP_SERVER", "svr_id", svr_id, 0);

    string session_param;
    conf.GetValue("TCP_CONNECT_1", "session_param", session_param, "");
    cout << session_param << endl;
    //int fd = TCPSocket::CreateSocket(true, true);
    //int listen_fd = TCPSocket::Listen(35000);
    //cout << listen_fd << endl;
    //int client_fd = TCPSocket::Accept(listen_fd);
    //cout << client_fd << endl;
    //cout << strerror(errno) << endl;
    return 0;

}


