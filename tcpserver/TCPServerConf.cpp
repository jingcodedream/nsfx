/*
 * TCPServerConf.cpp
 *
 *  Created on: 2015年9月9日
 *      Author: joe
 */

#include "TCPServerConf.h"

#include <iostream>   //test
using namespace std; //test

int TCPServerConf::Init(ReadConf &conf)
{
    string TCP_SERVER = "TCP_SERVER";
    conf.GetValue(TCP_SERVER, "listen_num", listen_num, 0);
    for(int i = 0;i < listen_num;++i)
    {
        string LISTEN_CONF = "LISTEN_CONF_" + i;
        ListenConf listenConfTemp;
        conf.GetValue(LISTEN_CONF, "ip", listenConfTemp.ip);
        conf.GetValue(LISTEN_CONF, "port", listenConfTemp.port, -1);
        conf.GetValue(LISTEN_CONF, "backlog", listenConfTemp.backlog, -1);

        if(listenConfTemp.ip.empty() || listenConfTemp.port < 0 || listenConfTemp.backlog < 0)
        {
            cout << "去你妹的，配置没填对" << endl;
            return -1;
        }
        listenConfVector.push_back(listenConfTemp);
    }

    conf.GetValue(TCP_SERVER, "connect_num", connect_num, 0);
    for(int i = 0;i < connect_num;++i)
    {
        string CONNECT_CONF_ = "CONNECT_CONF_" + i;
        ConnectConf ConnectConfTemp;
        conf.GetValue(CONNECT_CONF_, "ip", ConnectConfTemp.ip);
        conf.GetValue(CONNECT_CONF_, "port", ConnectConfTemp.port, -1);

        if(ConnectConfTemp.ip.empty() || ConnectConfTemp.port < 0)
        {
            cout << "去你妹的，配置没填对" << endl;
            return -1;
        }
        connectConfVector.push_back(ConnectConfTemp);
    }
    return 0;
}


