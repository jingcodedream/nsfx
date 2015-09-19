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
#include "../IOServer.h"

#include <vector>
#include <sys/epoll.h>

#define EPOLLEVENTS 100

class TCPServer
{
public:
    TCPServer(){}
    virtual ~TCPServer(){} //基类要使用虚析构函数，否则基类指针指向子类对象时，不会调用子类析构函数，导致释放不完全

    int Init(ReadConf &readConf);

    IOServer* GetIOServer(){return &ioServer;}

    void RunFover();

private:
    ReadConf readConf;
    TCPServerConf tcpServerConf;

    IOServerEpoll ioServer;

private:
    int DoListen();
    int DoConnect();

private:
    DECL_LOGGER(logger);
};

#endif /* TCPSERVER_TCPSERVER_H_ */
