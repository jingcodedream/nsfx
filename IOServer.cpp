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

IMPL_LOGGER(IOServer, logger);
IMPL_LOGGER(IOServerEpoll, logger);
IMPL_LOGGER(IOHandleListen, logger);
IMPL_LOGGER(IOHandleConnect, logger);

std::pair<IOEvent, IOEvent> IOServer::AddEvent(int fd, IOEvent event,
        IOHandle *handle) {
    std::map<int, IOEventInfo>::iterator it = fdIOEventInfoMap.find(fd);
    std::pair<IOEvent, IOEvent> ret_pair;
    ret_pair.first = ret_pair.second = EVENT_EMPTY;
    if (it == fdIOEventInfoMap.end()) {
        LOG_DEBUG(logger, "insert fd="<<fd<<",handle="<<handle);
        IOEventInfo ioEventInfoTemp;
        ioEventInfoTemp.fd = fd;
        ioEventInfoTemp.handle = handle;
        std::pair<std::map<int, IOEventInfo>::iterator, bool> retInsert =
                fdIOEventInfoMap.insert(std::make_pair(fd, ioEventInfoTemp));
        if (retInsert.second == false) {
            LOG_ERROR(logger, "insert fdIOEventInfoMap error");
            return ret_pair;
        }
        it = retInsert.first;
    }
    IOEventInfo &ioEventInfoTemp = it->second;
    ret_pair.second = ioEventInfoTemp.event;
    ioEventInfoTemp.event |= event;
    ret_pair.first = ioEventInfoTemp.event;
    return ret_pair;
}

std::pair<IOEvent, IOEvent> IOServer::DelEvent(int fd, IOEvent event) {
    std::map<int, IOEventInfo>::iterator it = fdIOEventInfoMap.find(fd);
    std::pair<IOEvent, IOEvent> ret_pair;
    ret_pair.first = ret_pair.second = EVENT_EMPTY;
    if (it == fdIOEventInfoMap.end()) {
        LOG_ERROR(logger, "del event not find fd");
        return ret_pair;
    }

    IOEventInfo &ioEventInfoTemp = it->second;
    ret_pair.second = ioEventInfoTemp.event;
    ioEventInfoTemp.event &= (~event);
    ret_pair.first = ioEventInfoTemp.event;
    return ret_pair;
}

IOStatus IOHandleListen::OnRead(int fd) {
    struct sockaddr_in peer_addr;
    int acceptFd = TCPSocket::Accept(listenFd, peer_addr);

    if (acceptFd < 0) {
        LOG_ERROR(logger, "accept error, fd="<<acceptFd);
        return IO_ERROR;
    } LOG_DEBUG(logger, "listenFd="<<listenFd<<" OnRead, acceptFd="<<acceptFd);
    IOHandleConnect *ioHandConnect = new IOHandleConnect(ioServer);
    std::pair<IOEvent, IOEvent> ret_pair = ioServer->AddEvent(acceptFd, EVENT_READ, ioHandConnect);
    if (ret_pair.first == EVENT_EMPTY) {
        LOG_ERROR(logger, "accept error");
        close(acceptFd);    //如果没添加成功，那么IOServerEpoll不负责回收
        return IO_ERROR;
    } LOG_DEBUG(logger, "add acceptFd="<<acceptFd<<" event="<<EPOLLIN<<" success");
    return IO_CONTINUE;
}

IOStatus IOHandleConnect::OnRead(int fd) {
    char buff[1024];
    memset(buff, 0, 1024);
    LOG_DEBUG(logger, "connect="<<fd<<" OnRead");
    int ret = TCPSocket::Recv(fd, buff, sizeof(buff));
    if (ret < 0) {
        LOG_ERROR(logger, "recv date error, fd="<<fd<<",ret="<<ret);
        return IO_ERROR;
    } else if (ret == 0) {
        LOG_DEBUG(logger, "opposite side close,fd="<<fd<<",ret="<<ret);
        return IO_ERROR;
    }

    //处理接收到的数据

    //TODO 数据处理还没写
    std::pair<IOEvent, IOEvent> ret_pair = ioServer->AddEvent(fd, EVENT_WRITE, this);
    assert(ret_pair.first != EVENT_EMPTY);
    return IO_CONTINUE;
}

IOStatus IOHandleConnect::OnWrite(int fd) {
    LOG_DEBUG(logger, "connect="<<fd<<" OnWrite");

    std::string retStr = "我日你大爷,我日你大爷,我日你大爷,我日你大爷,我日你大爷,我日你大爷,我日你大爷,我日你大爷,"
            "我日你大爷,我日你大爷,我日你大爷,我日你大爷,我日你大爷,我日你大爷,我日你大爷,我日你大爷,我日你大爷,v,我日你大爷,"
            "我日你大爷,我日你大爷,我日你大爷,我日你大爷,我日你大爷,我日你大爷,我日你大爷";
    int ret = TCPSocket::Send(fd, retStr.c_str(), retStr.length());
    if (ret < 0) {
        LOG_DEBUG(logger, "write error,fd="<<fd<<",ret="<<ret);
        return IO_ERROR;
    }
    return IO_SUCC;
}

void IOServerEpoll::Init(int maxEventNum) {
    LOG_DEBUG(logger, "IOServerEpoll Init");
    this->maxEventNum = maxEventNum;
    epollFd = epoll_create1(EPOLL_CLOEXEC);
    epollEvent = (struct epoll_event *) malloc(sizeof(struct epoll_event) * maxEventNum);
    assert(epollEvent != NULL);
    LOG_DEBUG(logger, "IOServerEpoll Init Success");
}

std::pair<IOEvent, IOEvent> IOServerEpoll::AddEvent(int fd, IOEvent event,
        IOHandle *handle) {
    std::pair<IOEvent, IOEvent> ret_pair = IOServer::AddEvent(fd, event,
            handle);
    LOG_TRACE(logger, "EpollIOServer:add io event fd="<<fd<<",event="<<event);
    if (ret_pair.first == ret_pair.second || ret_pair.first == EVENT_EMPTY)
        return ret_pair;

    struct epoll_event ep_event;
    ep_event.events = 0;
    ep_event.data.fd = fd;
    if (EVENT_HAS_READ(ret_pair.first))
        ep_event.events |= EPOLLIN;
    if (EVENT_HAS_WRITE(ret_pair.first))
        ep_event.events |= EPOLLOUT;

    if (ret_pair.second == EVENT_EMPTY) {
        if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ep_event) != 0) {
            LOG_ERROR(logger, "EpollIOServer:add io event failed. fd="<<fd<<",new_event="<<ret_pair.first<<",errno="<<errno<<"("<<strerror(errno)<<")");
            return IOServer::DelEvent(fd, EVENT_RDWT);
        } LOG_TRACE(logger, "EpollIOServer:add io event succ.fd="<<fd<<",new_event="<<ret_pair.first);
    } else {
        if (epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ep_event) != 0) {
            LOG_ERROR(logger, "EpollIOServer:mod io event failed. fd="<<fd<<",cur_event="<<ret_pair.second<<",new_event="<<ret_pair.first<<",errno="<<errno<<"("<<strerror(errno)<<")");
            return IOServer::DelEvent(fd, EVENT_RDWT);
        } LOG_TRACE(logger, "EpollIOServer:mod io event succ.fd="<<fd<<",cur_event="<<ret_pair.second<<",new_event="<<ret_pair.first);
    }
    return ret_pair;
}

std::pair<IOEvent, IOEvent> IOServerEpoll::DelEvent(int fd, IOEvent event) {
    std::pair<IOEvent, IOEvent> ret_pair = IOServer::DelEvent(fd, event);
    if (ret_pair.first == ret_pair.second)
        return ret_pair;

    if (ret_pair.first == EVENT_EMPTY) {
        if (epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL) != -1) {
            LOG_ERROR(logger, "EpollIOServer:del all io event failed. fd="<<fd<<",errno="<<errno<<"("<<strerror(errno)<<").");
        } else {
            LOG_TRACE(logger, "EpollIOServer:del io event succ.fd="<<fd<<",cur_event="<<ret_pair.second);
        }
    } else {
        struct epoll_event ep_event;
        ep_event.events = 0;
        ep_event.data.fd = fd;
        if (EVENT_HAS_READ(ret_pair.first))
            ep_event.events |= EPOLLIN;
        if (EVENT_HAS_WRITE(ret_pair.first))
            ep_event.events |= EPOLLOUT;
        if (epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ep_event) != 0) {
            LOG_ERROR(logger, "EpollIOServer:mod event failed. fd="<<fd<<",cur_event="<<ret_pair.second<<",new_event="<<ret_pair.first<<",errno="<<errno<<"("<<strerror(errno)<<").");
        } else {
            LOG_TRACE(logger, "EpollIOServer:mod event succ.fd="<<fd<<",cur_event="<<ret_pair.second<<",new_event="<<ret_pair.first);
        }
    }

    return ret_pair;
}

bool IOServerEpoll::WaitEvent(std::vector<EventOccur> &eventOccurVector,
        int wait_ms) {
    eventOccurVector.clear();
    int32_t eventCount = 0;
    if ((eventCount = epoll_wait(epollFd, epollEvent, maxEventNum, wait_ms))
            == -1) {
        if (errno != EINTR) {
            LOG_ERROR(logger, "wait event error, errno:"<<strerror(errno));
            return false;
        } else {
            return true;
        }
    }
    for (int i = 0; i < eventCount; ++i) {
        struct epoll_event epoll_event_temp = epollEvent[i];
        EventOccur eventOccurTemp;
        LOG_TRACE(logger, "EpollIOServer:fd="<<epoll_event_temp.data.fd<<",event="<<epoll_event_temp.events);
        eventOccurTemp.fd = epoll_event_temp.data.fd;
        if (epollEvent[i].events & (EPOLLERR | EPOLLRDHUP | EPOLLHUP)) {
            LOG_ERROR(logger, "EpollIOServer:io error occur.fd="<<epoll_event_temp.data.fd<<",events="<<epoll_event_temp.events<<",errno="<<errno<<"("<<strerror(errno)<<").");
            eventOccurTemp.event = EVENT_ERROR;
            eventOccurVector.push_back(eventOccurTemp);
            continue;
        }
        if (epoll_event_temp.events & EPOLLIN) {
            eventOccurTemp.event |= EVENT_READ;
        }
        if (epoll_event_temp.events & EPOLLOUT) {
            eventOccurTemp.event |= EVENT_WRITE;
        }
        eventOccurVector.push_back(eventOccurTemp);
    }
    return true;
}

bool IOServer::RunForever() {
    while (true) {
        if (!RunOnce()) {
            return false;
        }
    }
    return true;
}

bool IOServer::RunOnce() {
    std::vector<EventOccur> evnetOccurVector;
    int wait_ms = 1000;

    if (WaitEvent(evnetOccurVector, wait_ms)) {
        LOG_TRACE(logger, "collect io event finished. count="<<evnetOccurVector.size());
    } else {
        LOG_ERROR(logger, "collect io event failed");
    }

    int eventCount = evnetOccurVector.size();
    LOG_DEBUG(logger, "RunOnce, eventCount="<<eventCount);
    for (int i = 0; i < eventCount; ++i) {
        EventOccur eventOccurTemp = evnetOccurVector[i];
        int currentFd = eventOccurTemp.fd;
        std::map<int, IOEventInfo>::iterator it = fdIOEventInfoMap.find(
                currentFd);

        if (it == fdIOEventInfoMap.end()) {
            LOG_ERROR(logger, "currentFd="<<currentFd<<" not add");
            DelEvent(eventOccurTemp.fd, EVENT_RDWT);
            return false;
        }
        IOEventInfo ioEventInfoTemp = it->second;
        IOEvent delEvents = EVENT_EMPTY;
        bool noError = true;
        if (EVENT_HAS_ERROR(eventOccurTemp.event)) {
            noError = false;
            delEvents |= EVENT_RDWT;
        }
        if (noError && (eventOccurTemp.event & EVENT_READ)) {
            IOStatus ret = (ioEventInfoTemp.handle)->OnRead(currentFd);
            if (ret == IO_ERROR) {
                noError = false;
                delEvents |= EVENT_RDWT;
            }
            if (ret == IO_SUCC) {
                delEvents |= EVENT_READ;
            }
        }
        if (noError && (eventOccurTemp.event & EVENT_WRITE)) {
            IOStatus ret = (ioEventInfoTemp.handle)->OnWrite(currentFd);
            if (ret == IO_ERROR) {
                noError = false;
                delEvents |= EVENT_RDWT;
            }
            if (ret == IO_SUCC) {
                delEvents |= EVENT_WRITE;
            }
        }

        if (!noError) {
            LOG_ERROR(logger, "io error occur.fd="<<ioEventInfoTemp.fd);
            (ioEventInfoTemp.handle)->OnError(eventOccurTemp.fd);
        }
        if (delEvents != EVENT_EMPTY) {
            std::pair<IOEvent, IOEvent> ret_pair = DelEvent(eventOccurTemp.fd,
                    delEvents);

            if (ret_pair.first == EVENT_EMPTY) {
                if (!noError) {
                    close(eventOccurTemp.fd);
                    LOG_ERROR(logger, "io error occur and close it.fd="<<eventOccurTemp.fd);
                }

                LOG_TRACE(logger, "all io event is delete. fd="<<eventOccurTemp.fd);
                continue;
            }
        }

        if (!noError) {
            close(eventOccurTemp.fd);
            LOG_ERROR(logger, "io error occur and close it.fd="<<eventOccurTemp.fd);
        }
    }
    return true;
}

