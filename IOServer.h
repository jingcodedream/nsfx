/*
 * IOHandle.h
 *
 *  Created on: 2015年9月16日
 *      Author: joe
 */

#ifndef IOSERVER_H_
#define IOSERVER_H_

#include <assert.h>
#include <stddef.h>
#include <map>
#include <sys/epoll.h>
#include <stdlib.h>

#include "Logger.h"

class IOHandle
{
public:
    IOHandle(){}
    virtual ~IOHandle(){}
    virtual int OnRead(int fd)=0;
    virtual int OnWrite(int fd)=0;

private:
};

class IOServer
{
    IOServer(){}
    virtual ~IOServer(){}

    bool RunOnce();
    bool RunForever();

    virtual int AddEvent(int fd, int event,IOHandle *handle)=0;
    virtual int ModifyEvent(int fd, int event)=0;
    virtual int DelEvent(int fd, int event)=0;

    virtual int WaitEvent();
};

class IOServerEpoll:public IOServer
{
public:
    IOServerEpoll():epollFd(0),maxEventNum(0),epollEvent(NULL){};
    ~IOServerEpoll(){};

    void Init(int maxEventNum);

    int AddEvent(int fd, int event,IOHandle *handle);
    int ModifyEvent(int fd, int event);
    int DelEvent(int fd, int event);

private:
    int epollFd;
    int maxEventNum;
    struct epoll_event *epollEvent;
    std::map<int, IOHandle*> fdIOHandleMap;

private:
    DECL_LOGGER(logger);
};


class IOHandleListen:public IOHandle
{
public:
    IOHandleListen(int listenFd, IOServerEpoll *ioServerEpoll):listenFd(listenFd), ioServerEpoll(ioServerEpoll){}
    int OnRead(int fd);

private:
    //IOServer *ioServer;
    int listenFd;
    IOServerEpoll *ioServerEpoll;

private:
    int OnWrite(int fd){return 0;}//实现IOHandle的纯虚函数

private:
    DECL_LOGGER(logger);
};

class IOHandleConnect:public IOHandle
{
public:
    IOHandleConnect(){}
    int OnRead(int fd);
    int OnWrite(int fd);
private:
    DECL_LOGGER(logger);
};
#endif /* IOSERVER_H_ */
