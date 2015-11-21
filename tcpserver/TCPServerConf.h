/*
 * TCPServerConf.h
 *
 *  Created on: 2015年9月9日
 *      Author: joe
 */

#ifndef TCPSERVER_TCPSERVERCONF_H_
#define TCPSERVER_TCPSERVERCONF_H_

#include <vector>

#include "../ReadConf.h"
#include "../Logger.h"

struct ListenConf {
    string ip;
    int port;
    int backlog;  //队列大小
};

struct ConnectConf {
    string ip;
    int port;
    int backlog;  //队列大小
};

class TCPServerConf {
public:
    TCPServerConf() {
        listen_num = 0;
        connect_num = 0;
        conf_max_event_num = 0;
        connect_time = 0;
    }
    ~TCPServerConf() {
    }

    int Init(ReadConf &readconf);
    //std::vector<ListenConf> getListenConfVector(){return listenConfVector;}
    //std::vector<ConnectConf> getConnectConfVector(){return connectConfVector;}

    std::vector<ListenConf> listenConfVector;
    std::vector<ConnectConf> connectConfVector;
    int conf_max_event_num;
    int connect_time;
    int listen_num;
    int connect_num;

private:
    DECL_LOGGER(logger)
    ;
};

#endif /* TCPSERVER_TCPSERVERCONF_H_ */
