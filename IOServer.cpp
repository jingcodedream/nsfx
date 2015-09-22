/*
 * IOServerEpoll.cpp
 *
 *  Created on: 2015年9月17日
 *      Author: joe
 */

#include "TCPSocket.h"

#include <string>
#include <netinet/in.h>
#include <string.h>

#include <iostream>

#include "IOServer.h"

IMPL_LOGGER(IOServerEpoll, logger);
IMPL_LOGGER(IOHandleListen, logger);
IMPL_LOGGER(IOHandleConnect, logger);

int IOHandleListen::OnRead(int fd)
{
    struct sockaddr_in peer_addr;
    int acceptFd = TCPSocket::Accept(listenFd, peer_addr);

    if(acceptFd < 0)
    {
        LOG_ERROR(logger, "accept error, fd="<<acceptFd);
        return -1;
    }
    LOG_DEBUG(logger, "listenFd="<<listenFd<<" OnRead, acceptFd="<<acceptFd);
    IOHandleConnect *ioHandConnect = new IOHandleConnect();
    if(ioServerEpoll->AddEvent(acceptFd, EPOLLIN, ioHandConnect) != 0)
    {
        return -1;
    }
    LOG_DEBUG(logger, "add acceptFd="<<acceptFd<<" event="<<EPOLLIN<<" success");
    return 0;
}

int IOHandleConnect::OnRead(int fd)
{
    char buff[1024];
    memset(buff, 0, 1024);
    int ret = TCPSocket::Recv(fd, buff, sizeof(buff));
    if(ret < 0)
    {
        return -1;
    }
    else if(ret == 0)
    {
        return -2;
    }

    //处理接收到的数据
    std::cout << buff << std::endl;
    return 0;
}

int IOHandleConnect::OnWrite(int fd)
{
    std::string retStr = "我日你大爷";

    int ret = TCPSocket::Send(fd, retStr.c_str(), retStr.length());
    if(ret < 0)
    {
        return -1;
    }
    return 0;
}

void IOServerEpoll::Init(int maxEventNum)
{
    LOG_DEBUG(logger, "IOServerEpoll Init");
    fdIOHandleMap.clear();
    this->maxEventNum = maxEventNum;
    epollFd = epoll_create1(EPOLL_CLOEXEC);
    epollEvent = (struct epoll_event *)malloc(sizeof(struct epoll_event) * maxEventNum);
    assert(epollEvent != NULL);
    LOG_DEBUG(logger, "IOServerEpoll Init Success");
}

int IOServerEpoll::AddEvent(int fd, int event, IOHandle *handle)
{
    std::map<int, IOHandle*>::iterator it = fdIOHandleMap.find(fd);
    if(it == fdIOHandleMap.end())
    {
        LOG_DEBUG(logger, "insert fd="<<fd<<",handle="<<handle);
        fdIOHandleMap.insert(std::make_pair(fd, handle));
    }

    struct epoll_event ep_event;
    ep_event.events = event;
    ep_event.data.fd = fd;
    if(epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ep_event) != 0)
    {
        LOG_ERROR(logger, "add fd="<<fd<<",event="<<event<<"failed,cause:"<<strerror(errno));
        return -1;
    }
    return 0;
}

int IOServerEpoll::ModifyEvent(int fd, int event)
{
    std::map<int, IOHandle*>::iterator it = fdIOHandleMap.find(fd);
    if(it == fdIOHandleMap.end())
    {
        LOG_ERROR(logger, "fd="<<fd<<",not be include");
        return -1;
    }
    struct epoll_event ep_event;
    ep_event.events = event;
    ep_event.data.fd = fd;
    if(epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ep_event) != 0)
    {
        LOG_ERROR(logger, "modify fd="<<fd<<",event="<<event<<"failed,cause:"<<strerror(errno));
        return -1;
    }
    return 0;
}

int IOServerEpoll::DelEvent(int fd, int event)
{
    std::map<int, IOHandle*>::iterator it = fdIOHandleMap.find(fd);
    if(it == fdIOHandleMap.end())
    {
        LOG_ERROR(logger, "fd="<<fd<<",not be include");
        return -1;
    }
    struct epoll_event ep_event;
    ep_event.events = event;
    ep_event.data.fd = fd;
    if(epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, &ep_event) != 0)
    {
        LOG_ERROR(logger, "delete fd="<<fd<<",event="<<event<<"failed,cause:"<<strerror(errno));
        return -1;
    }
    return 0;
}

bool IOServer::RunForever()
{
    while(true)
    {
        if(!RunOnce())
            return -1;
        {
            return false;
        }
    }
    return true;
}

bool IOServer::RunOnce()
{
    int eventCount = epoll_wait(epollFd, epollEvent, maxEventNum, 1000);
    LOG_DEBUG(logger, "RunOnce, eventCount="<<eventCount);
    for(int i = 0;i < eventCount;++i)
    {
        int currentFd = epollEvent[i].data.fd;
        std::map<int, IOHandle*>::iterator it = fdIOHandleMap.find(currentFd);

        if(it == fdIOHandleMap.end())
        {
            LOG_ERROR(logger, "currentFd="<<currentFd<<" not add");
            return false;
        }
        if(epollEvent[i].events & EPOLLIN)
        {
            LOG_DEBUG(logger, "读取一次" << currentFd);
            if((it->second)->OnRead(currentFd) != 0)
            {
                LOG_ERROR(logger, "OnRead error");
                return false;
            }
            LOG_DEBUG(logger, "删除监听读");
            if(DelEvent(currentFd, EPOLLIN) != 0)
            {
                LOG_ERROR(logger, "currentFd="<<currentFd<<",delete event="<<EPOLLIN<<" error");
                return false;
            }
        }
        if(epollEvent[i].events & EPOLLOUT)
        {
            LOG_DEBUG(logger, "写一次");
            if((it->second)->OnWrite(currentFd) != 0)
            {
                LOG_ERROR(logger, "OnWrite error");
                return false;
            }
            LOG_DEBUG(logger, "删除监听写");
            if(DelEvent(currentFd, EPOLLOUT) != 0)
            {
                LOG_ERROR(logger, "currentFd="<<currentFd<<",delete event="<<EPOLLIN<<" error");
                return false;
            }
        }
    }
    return true;
}



