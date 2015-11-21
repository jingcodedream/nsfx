/*
 * Main.cpp
 *
 *  Created on: 2015年9月14日
 *      Author: joe
 */

#include "TCPServerTest.h"

#include <iostream>
using namespace std;

#define CONF_NAME "conf/TCPServer.conf"

int main() {
    INIT_LOGGER("conf/log4cplus.conf");

    ReadConf conf;
    if (conf.Init(CONF_NAME) < 0) {
        std::cout << "conf init error" << std::endl;
        return -1;
    }

    TCPServerTest tcpServerTest;
    if (tcpServerTest.Init(conf) < 0) {
        std::cout << "tcpServer init error" << std::endl;
        return -1;
    }

    tcpServerTest.RunForever();

    return 0;
}

