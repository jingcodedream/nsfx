/*
 * TCPServer.cpp
 *
 *  Created on: 2015年9月12日
 *      Author: joe
 */

#include "../TCPSocket.h"

#include "TCPServer.h"

IMPL_LOGGER(TCPServer, logger);

int TCPServer::DoListen() {
    std::vector<ListenConf> listenConfVector = tcpServerConf.listenConfVector;

    for (size_t i = 0; i < listenConfVector.size(); ++i) {
        ListenConf listenConfTemp = listenConfVector[i];
        int listenFd = TCPSocket::Listen(listenConfTemp.ip.c_str(),
                listenConfTemp.port, false, listenConfTemp.backlog);
        LOG_DEBUG(logger, "Listen, ip="<<listenConfTemp.ip<<",port="<<listenConfTemp.port<<",backlog="<<listenConfTemp.backlog<<",fd="<<listenFd);
        if (listenFd < 0) {
            LOG_ERROR(logger, "listen erro, cause: " << strerror(errno));
            return -1;
        }

        IOHandle *ioHandleListen = new IOHandleListen(listenFd, &ioServerEpoll);
        std::pair<IOEvent, IOEvent> ret_pair = ioServerEpoll.AddEvent(listenFd,EVENT_READ, ioHandleListen);
        assert(ret_pair.first != EVENT_EMPTY);
        LOG_DEBUG(logger, "Listen success");
    }
    return 0;
}

int TCPServer::DoConnect() {
    std::vector<ConnectConf> connectConfVector = tcpServerConf.connectConfVector;

    for (size_t i = 0; i < connectConfVector.size(); ++i) {
        ConnectConf connectConfTemp = connectConfVector[i];
        int connFd = TCPSocket::Connect(connectConfTemp.ip.c_str(),
                connectConfTemp.port, false, 1000 * 6);

        LOG_DEBUG(logger, "Connect, ip="<<connectConfTemp.ip<<",port="<<connectConfTemp.port<<",wait_time="<<1000*6);
        if (connFd < 0) {
            LOG_ERROR(logger, "connect error, cause " << strerror(errno));
            return -1;
        }

        IOHandleConnect *ioHandleConnect = new IOHandleConnect(&ioServerEpoll);
        std::pair<IOEvent, IOEvent> ret_pair = ioServerEpoll.AddEvent(connFd,EVENT_READ, ioHandleConnect);
        if (ret_pair.first == EVENT_EMPTY) {
            LOG_ERROR(logger, "connect error");
            return -1;
        } LOG_DEBUG(logger, "Connect success");
    }
    return 0;
}

int TCPServer::Init(ReadConf &readConf) {
    this->readConf = readConf;

    LOG_DEBUG(logger, "Init TCPServerConf");
    if (tcpServerConf.Init(readConf) != 0) {
        LOG_ERROR(logger, "Iint TCPServerConf Failed!");
        return -1;
    }

    ioServerEpoll.Init(tcpServerConf.conf_max_event_num);

    if (tcpServerConf.listenConfVector.size() > 0) {
        LOG_DEBUG(logger, "Do Litsen！");
        if (DoListen() != 0) {
            LOG_ERROR(logger, "Do Listen Failed !");
            return -1;
        }
    }

    if (tcpServerConf.connectConfVector.size() > 0) {
        LOG_DEBUG(logger, "Do Connect!");
        if (DoConnect() != 0) {
            LOG_ERROR(logger, "Do Connect Failed !");
            return -1;
        }
    }

    return 0;
}

void TCPServer::RunForever() {
    while (true) {
        if (!ioServerEpoll.RunForever())
            break;
    }
}
