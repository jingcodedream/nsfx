/*
 * TCPServer.h
 *
 *  Created on: 2015年9月9日
 *      Author: joe
 */

#ifndef TCPSERVER_TCPSERVER_H_
#define TCPSERVER_TCPSERVER_H_

#include "../ReadConf.h"
#include "TCPServerConf.h"
#include "../Logger.h"

#include <vector>
#include <sys/epoll.h>

#define EPOLLEVENTS 100

class TCPServer
{
public:
    TCPServer():epollfd(-1){}
    ~TCPServer(){}

    int Init(ReadConf &readConf, int maxEpollSize);

    void RunFover();

private:
    ReadConf readConf;
    TCPServerConf tcpServerConf;

    std::vector<int> listenFdVector;
    int epollfd;
    //struct epoll_event events[EPOLLEVENTS];

private:
    int DoListen();
    int DoConnect();

    //epoll处理
    int AddEvent(int fd, int state);

    int DeleteEvent(int fd, int state);

    int ModifyEvent(int fd, int state);

    int EpollWait();

    int OnRead(int fd);

    int OnWrite(int fd);

private:
    DECL_LOGGER(logger);
};

#endif /* TCPSERVER_TCPSERVER_H_ */
