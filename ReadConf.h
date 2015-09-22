/*
 * ReadConf.h
 *
 *  Created on: 2015年9月8日
 *      Author: joe
 */

#ifndef READCONF_H_
#define READCONF_H_

//将配置文件读成一个map
#include "Logger.h"

#include <string>
#include <map>
#include <algorithm>
using std::string;

class ReadConf
{
public:
    ReadConf(){}
    ~ReadConf(){}
    string GetErrMsg(){return errMsg;}
    int Init(const char *fname);

    void GetValue(const string &group, const string &key, int &value, int default_value=-1);
    void GetValue(const string &group, const string &key, string &value, const string &default_value="");

private:
    typedef std::map<string, string> KeyValueMap;//键值对
    typedef std::map<string, KeyValueMap> GroupMap;//键值对所属的组
    KeyValueMap keyValueMap;
    GroupMap groupMap;
    string errMsg;

    bool ParseConf(const string &buff);//解析传入的字符串
    void Strim(string &retStr);//去掉所有空格
    void Ltrim(string &str) //去掉首空格
    {
        str.erase(str.begin(), std::find_if(str.begin(), str.end(),
                                            std::not1(std::ptr_fun(::isspace))));

    }
    void Rtrim(string &str) //去掉尾部空格
    {
        str.erase(std::find_if(str.rbegin(), str.rend(),
                               std::not1(std::ptr_fun(::isspace))).base(),
                  str.end());

    }

    void LRtrim(string &str) //去掉首尾空格
    {
        Ltrim(str);
        Rtrim(str);
    }

private:
    DECL_LOGGER(logger);
};

#endif /* READCONF_H_ */
