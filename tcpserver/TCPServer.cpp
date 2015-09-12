/*
 * TCPServer.cpp
 *
 *  Created on: 2015年9月12日
 *      Author: joe
 */

#include <vector>
#include "../TCPSocket.h"

#include "TCPServer.h"


int TCPServer::Dolisten()
{
    std::vector<ListenConf> listenConfVector = tcpServerConf.getListenConfVector();

    for(int i = 0; i < listenConfVector.size(); ++i)
    {
        ListenConf listenConfTemp = listenConfVector[i];
        int listenFd = TCPSocket::Listen(listenConfTemp.ip.c_str(), listenConfTemp.port, false, listenConfTemp.backlog);
        if(fd < 0)
        {

        }
    }
    return 0;
}

