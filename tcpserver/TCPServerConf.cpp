/*
 * TCPServerConf.cpp
 *
 *  Created on: 2015年9月9日
 *      Author: joe
 */

#include "TCPServerConf.h"

#include <string>
#include <sstream>

#include <iostream>   //test
using namespace std;
//test

IMPL_LOGGER(TCPServerConf, logger);

int TCPServerConf::Init(ReadConf &conf) {
    string TCP_SERVER = "TCP_SERVER";

    conf.GetValue(TCP_SERVER, "max_event_num", conf_max_event_num, 65535);
    conf.GetValue(TCP_SERVER, "connect_time", connect_time, 3);
    conf.GetValue(TCP_SERVER, "listen_num", listen_num, 0);
    conf.GetValue(TCP_SERVER, "connect_num", connect_num, 0);
    LOG_DEBUG(logger, "ListenConf,TCP_SERVER="<<TCP_SERVER<<",max_event_num="<<conf_max_event_num\
 <<",connect_time="<<connect_time<<",listen_num="<<listen_num<<",connect_num="<<connect_num);

    for (int i = 0; i < listen_num; ++i) {
        ostringstream os;
        os << "LISTEN_CONF_" << i;
        string LISTEN_CONF = os.str();
        ListenConf listenConfTemp;
        conf.GetValue(LISTEN_CONF, "ip", listenConfTemp.ip);
        conf.GetValue(LISTEN_CONF, "port", listenConfTemp.port, -1);
        conf.GetValue(LISTEN_CONF, "backlog", listenConfTemp.backlog, -1);

        LOG_DEBUG(logger, "ListenConf,LISTEN_CONF="<<LISTEN_CONF<<",ip="<<listenConfTemp.ip<<",port="<<listenConfTemp.port<<",backlog="<<listenConfTemp.backlog);
        if (listenConfTemp.ip.empty() || listenConfTemp.port < 0
                || listenConfTemp.backlog < 0) {
            LOG_ERROR(logger, "ListenConf is wrong,LISTEN_CONF="<<LISTEN_CONF<<",ip="<<listenConfTemp.ip<<",port="<<listenConfTemp.port<<",backlog="<<listenConfTemp.backlog);
            return -1;
        }
        listenConfVector.push_back(listenConfTemp);
    }

    for (int i = 0; i < connect_num; ++i) {
        ostringstream os;
        os << "CONNECT_CONF_" << i;
        string CONNECT_CONF = os.str();
        ConnectConf ConnectConfTemp;
        conf.GetValue(CONNECT_CONF, "ip", ConnectConfTemp.ip);
        conf.GetValue(CONNECT_CONF, "port", ConnectConfTemp.port, -1);

        LOG_DEBUG(logger, "ConnectConf,CONNECT_CONF="<<CONNECT_CONF<<",ip="<<ConnectConfTemp.ip<<",port="<<ConnectConfTemp.port);
        if (ConnectConfTemp.ip.empty() || ConnectConfTemp.port < 0) {
            LOG_ERROR(logger, "ConnectConf is wrong,CONNECT_CONF="<<CONNECT_CONF<<",ip="<<ConnectConfTemp.ip<<",port="<<ConnectConfTemp.port);
            return -1;
        }
        connectConfVector.push_back(ConnectConfTemp);
    }
    return 0;
}

