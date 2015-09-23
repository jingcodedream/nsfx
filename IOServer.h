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
#include <vector>

#include "Logger.h"

typedef enum _io_status_
{
    IO_ERROR,
    IO_CONTINUE,
    IO_SUCC,
}IOStatus;

//io事件
typedef uint16_t IOEvent;
#define EVENT_EMPTY    0x0000                    //空
#define EVENT_READ     0x0001                    //读
#define EVENT_WRITE    0x0002                    //写
#define EVENT_RDWT     0x0003                    //读写
#define EVENT_ERROR    0x0004                    //错误

#define EVENT_HAS_READ(x)  (((x)&EVENT_READ)  != 0) //是否设置读
#define EVENT_HAS_WRITE(x) (((x)&EVENT_WRITE) != 0) //是否设置写
#define EVENT_HAS_ERROR(x) (((x)&EVENT_ERROR) != 0) //是否发生错误

class IOHandle
{
public:
    IOHandle(){}
    virtual ~IOHandle(){}
    virtual IOStatus OnRead(int fd)=0;
    virtual IOStatus OnWrite(int fd)=0;
    virtual IOStatus OnError(int fd)=0;

private:
};

class IOServer
{
private:
    class IOEventInfo
    {
    public:
        IOEventInfo()
            :fd(-1)
            ,event(EVENT_EMPTY)
            ,handle(NULL)
        {}

        int fd;
        IOEvent event;
        IOHandle *handle;
    };
public:
    class EventOccur
    {
    public:
        int fd;
        IOEvent event;
        EventOccur():fd(-1), event(0){}
    };
public:
    IOServer(){}
    virtual ~IOServer(){}

    bool RunOnce();
    bool RunForever();

    virtual std::pair<IOEvent, IOEvent> AddEvent(int fd, IOEvent event,IOHandle *handle);
    virtual std::pair<IOEvent, IOEvent> DelEvent(int fd, IOEvent event);
    virtual bool WaitEvent(std::vector<EventOccur> &eventOccurVector, int wait_ms)=0;

private:
    std::map<int, IOEventInfo> fdIOEventInfoMap;

private:
    DECL_LOGGER(logger);
};

class IOServerEpoll:public IOServer
{
public:
    IOServerEpoll():epollFd(0),maxEventNum(0),epollEvent(NULL){};
    ~IOServerEpoll(){};

    void Init(int maxEventNum);

    std::pair<IOEvent, IOEvent> AddEvent(int fd, IOEvent event,IOHandle *handle);
    //int ModifyEvent(int fd, int event);
    std::pair<IOEvent, IOEvent> DelEvent(int fd, IOEvent event);
    bool WaitEvent(std::vector<EventOccur> &eventOccurVector, int wait_ms);

private:
    int epollFd;
    int maxEventNum;
    struct epoll_event *epollEvent;

private:
    DECL_LOGGER(logger);
};


class IOHandleListen:public IOHandle
{
public:
    IOHandleListen(int listenFd, IOServer *ioServer):listenFd(listenFd), ioServer(ioServer){}
    IOStatus OnRead(int fd);

private:
    //IOServer *ioServer;
    int listenFd;
    IOServer *ioServer;

private:
    IOStatus OnWrite(int fd) { return IO_ERROR; }//实现IOHandle的纯虚函数
    IOStatus OnError(int fd) { return IO_ERROR; }

private:
    DECL_LOGGER(logger);
};

class IOHandleConnect:public IOHandle
{
public:
    IOHandleConnect(IOServer *ioServer) : ioServer(ioServer) {}
    IOStatus OnRead(int fd);
    IOStatus OnWrite(int fd);
private:
    IOStatus OnError(int fd) { return IO_ERROR; }

private:
    IOServer *ioServer;

private:
    DECL_LOGGER(logger);
};
#endif /* IOSERVER_H_ */
