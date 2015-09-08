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
    int ret = conf.Init("TCP.conf");
    if(ret < 0)
    {
        cout << conf.GetErrMsg() << endl;
    }
    cout << "hello" << endl;
    //int fd = TCPSocket::CreateSocket(true, true);
    //int listen_fd = TCPSocket::Listen(35000);
    //cout << listen_fd << endl;
    //int client_fd = TCPSocket::Accept(listen_fd);
    //cout << client_fd << endl;
    //cout << strerror(errno) << endl;
    return 0;

}


