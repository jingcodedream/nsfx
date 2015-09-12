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

struct ListenConf
{
    string ip;
    int port;
    int backlog;  //队列大小
};

struct ConnectConf
{
    string ip;
    int port;
    int backlog;  //队列大小
};

class TCPServerConf
{
public:
    TCPServerConf():listen_num(0), connect_num(0){}
    ~TCPServerConf(){}

    int Init(ReadConf &readconf);
    std::vector<ListenConf> getListenConfVector(){return listenConfVector;}
    std::vector<ConnectConf> getConnectConfVector(){return connectConfVector;}

private:
    int listen_num;
    int connect_num;
    std::vector<ListenConf> listenConfVector;
    std::vector<ConnectConf> connectConfVector;
};


#endif /* TCPSERVER_TCPSERVERCONF_H_ */
