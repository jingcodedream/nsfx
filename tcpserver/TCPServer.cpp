/*
 * TCPServer.cpp
 *
 *  Created on: 2015年9月12日
 *      Author: joe
 */

#include "../TCPSocket.h"

#include "TCPServer.h"

#include <iostream> //test

IMPL_LOGGER(TCPServer, logger);

int TCPServer::DoListen()
{
    std::vector<ListenConf> listenConfVector = tcpServerConf.listenConfVector;

    for(int i = 0; i < listenConfVector.size(); ++i)
    {
        ListenConf listenConfTemp = listenConfVector[i];
        int listenFd = TCPSocket::Listen(listenConfTemp.ip.c_str(), listenConfTemp.port, false, listenConfTemp.backlog);
        LOG_DEBUG(logger, "Listen, ip="<<listenConfTemp.ip<<",port="<<listenConfTemp.port<<",backlog="<<listenConfTemp.backlog);
        if(listenFd < 0)
        {
            LOG_ERROR(logger, "listen erro, cause: " << strerror(errno));
            return -1;
        }

        if(AddEvent(listenFd, EPOLLIN) != 0)
        {
            close(listenFd);
            LOG_ERROR(logger, "add listen error, cause: " << strerror(errno));
            return -1;
        }
        listenFdVector.push_back(listenFd);
        LOG_DEBUG(logger, "Listen success");

    }
    return 0;
}

int TCPServer::DoConnect()
{
    std::vector<ConnectConf> connectConfVector = tcpServerConf.connectConfVector;

    for(int i = 0; i < connectConfVector.size(); ++i)
    {
        ConnectConf connectConfTemp = connectConfVector[i];
        int connfd = TCPSocket::Connect(connectConfTemp.ip.c_str(), connectConfTemp.port, false, 1000*6);
        LOG_DEBUG(logger, "Listen, ip="<<connectConfTemp.ip<<",port="<<connectConfTemp.port<<",wait_time="<<1000*6);
        if(connfd < 0)
        {
            LOG_ERROR(logger, "connect error, cause " << strerror(errno));
            return -1;
        }
    }
    return 0;
}

int TCPServer::Init(ReadConf &readConf, int maxEpollSize)
{
    epollfd = epoll_create(maxEpollSize);
    listenFdVector.clear();
    this->readConf = readConf;

    LOG_DEBUG(logger, "Init TCPServerConf");
    if(tcpServerConf.Init(readConf) !=  0)
    {
        LOG_ERROR(logger, "Iint TCPServerConf Failed!");
        return -1;
    }

    if(tcpServerConf.listenConfVector.size() > 0)
    {
        LOG_DEBUG(logger, "Do Liten！");
        if(DoListen() != 0)
        {
            LOG_ERROR(logger, "Do Listen Failed !");
            return -1;
        }
    }

    if(tcpServerConf.connectConfVector.size() > 0)
    {
        LOG_DEBUG(logger, "Do Connect!");
        if(DoConnect() != 0)
        {
            LOG_ERROR(logger, "Do Connect Failed !");
            return -1;
        }
    }

    return 0;
}

//epoll处理
int TCPServer::AddEvent(int fd, int state) {
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) != 0)
    {
        return -1;
    }
    return 0;
}

int TCPServer::DeleteEvent(int fd, int state) {
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    if(epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev) != 0)
    {
        return -1;
    }
    return 0;
}

int TCPServer::ModifyEvent(int fd, int state) {
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    if(epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev) != 0)
    {
        return -1;
    }
    return 0;
}

int TCPServer::EpollWait()
{
    struct epoll_event events[EPOLLEVENTS];
    int ret = 0;
    LOG_DEBUG(logger, "多久执行一次");
    if((ret = epoll_wait(epollfd, events, EPOLLEVENTS, 1000)) < 0)
    {
        LOG_ERROR(logger, "epoll_wait error, cause:"<<strerror(errno));
        return -1;
    }

    for(int i = 0;i < ret;++i)
    {
        bool find = false;
        for(int j = 0;j < listenFdVector.size(); ++j)
        {
            if(events[i].data.fd == listenFdVector[j] && (events[i].events & EPOLLIN))
            {
                LOG_DEBUG(logger, "accept!!");
                int acceptFd = TCPSocket::Accept(listenFdVector[j]);
                if(acceptFd < 0)
                {
                    return -1;
                }
                LOG_DEBUG(logger, "add acceptFd!!");
                if(AddEvent(acceptFd, EPOLLIN) < 0)
                {
                    close(acceptFd);
                    LOG_ERROR(logger, "add acceptFd error，cause: " << strerror(errno));
                    return -1;
                }
                find = true;
                break;
            }
        }
        if(find)
        {
            continue;
        }
        if(events[i].events & EPOLLIN)
        {
            int ret = OnRead(events[i].data.fd);
            if(ret < 0)
            {
                LOG_ERROR(logger, "OnRead error!!");
                return -1;
            }
            else if(ret == 0)
            {
                LOG_DEBUG(logger, "fd close, fd="<< events[i].data.fd);
            }
        }
        else if(events[i].events & EPOLLOUT)
        {
            int ret = OnWrite(events[i].data.fd);
            if(ret != 0)
            {
                LOG_ERROR(logger, "OnWrite error!!");
                return -1;
            }
        }
    }
    return 0;
}

int TCPServer::OnRead(int fd)
{
    char buff[1024] = {0};
    int ret = TCPSocket::Recv(fd, buff, sizeof(buff));
    if(ret < 0)
    {
        return -1;
    }
    LOG_DEBUG(logger, "ret="<<ret);
    if(ret == 0)
    {
        close(fd);
        return 0;
    }
    std::cout << buff << std::endl;
    if(ModifyEvent(fd, EPOLLOUT) != 0)
    {
        return -1;
    }
    return 1;
}

int TCPServer::OnWrite(int fd)
{
    char buff[1024] = "灰化肥发挥会发黑";
    int ret = TCPSocket::Send(fd, buff, sizeof(buff));
    if(ret < 0)
    {
        return -1;
    }
    //std::cout << buff << std::endl;
    if(ModifyEvent(fd, EPOLLIN) != 0)
    {
        return -1;
    }
    return 0;
}

void TCPServer::RunFover()
{
    while(true)
    {
        if(EpollWait() != 0)
        {
            break;
        }
    }
}

