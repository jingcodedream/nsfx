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

class TCPServer
{
public:
    TCPServer(ReadConf &readConf):readConf(readConf), tcpServerConf(readConf){}
    ~TCPServer(){}

    int Init();

private:
    ReadConf readConf;
    TCPServerConf tcpServerConf;

private:
    int Dolisten();
    int DoConnect();
};

#endif /* TCPSERVER_TCPSERVER_H_ */
