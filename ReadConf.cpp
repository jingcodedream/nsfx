/*
 * ReadConf.cpp
 *
 *  Created on: 2015年9月8日
 *      Author: joe
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <assert.h>
#include "ReadConf.h"

IMPL_LOGGER(ReadConf, logger);

int ReadConf::Init(const char *fname)
{

    keyValueMap.clear();
    groupMap.clear();
    errMsg.clear();

    if(fname == NULL)
    {
        errMsg = "fname is NULL";
        return -1;
    }

    struct stat f_stat;
    if(stat(fname, &f_stat) == -1)
    {
        errMsg = strerror(errno);
        return errno;
    }

    FILE *fp = fopen(fname, "rb");
    if(fp == NULL)
    {
        errMsg = "Open conf file failed";
        return -2;
    }
    char *buff = (char *)malloc(f_stat.st_size);
    if(buff == NULL)
    {
        errMsg = "malloc buff failed";
        fclose(fp);
        return -3;
    }

    if(fread(buff, 1, f_stat.st_size, fp) != f_stat.st_size)
    {
        errMsg = "read conf file failed";
        fclose(fp);
        free(buff);
        return -4;
    }

    //现在直接读这个字符串
    string context = buff;
    free(buff);
    fclose(fp);

    if(!ParseConf(context))
    {
        //errMsg = "parse conf failed";
        return -4;
    }
    errMsg = "parse conf succ";
    return 0;
}

bool ReadConf::ParseConf(const string &buff)
{
    string context = buff;
    KeyValueMap *KeyValueMapTemp = NULL;
    while(context.find("\n") != string::npos)
    {
        string line = context.substr(0, context.find("\n"));
        string effecStr = line.substr(0, line.find("#"));
        LRtrim(effecStr); //得到有效的字符串
        if(effecStr.empty())
        {
            context = context.substr(context.find("\n") + 1);
            continue;
        }

        if(effecStr[0] == '[')
        {
            if(effecStr.find("]") == string::npos || effecStr[effecStr.size() - 1] != ']')//如果不完整或者有多余字符都是错的
            {
                errMsg = effecStr + " not find ']' or ']' not the end char";
                return false;
            }

            string attrGroup;
            attrGroup = effecStr.substr(effecStr.find("[") + 1, effecStr.find("]") - effecStr.find("[") - 1);//选出组的名称

            if(attrGroup.empty()
                    || find_if(attrGroup.begin(), attrGroup.end(), std::ptr_fun(::isspace)) != attrGroup.end())//如果是空的或者含有空格都是错的
            {
                errMsg = attrGroup + " hava space char";
                return false;
            }

            GroupMap::iterator groupMapIt = groupMap.find(attrGroup);
            if(groupMapIt != groupMap.end())
            {
                errMsg = attrGroup + " is redefind";
                return false;
            }
            KeyValueMap tempKeyValueMap;
            //插入组中
            std::pair<GroupMap::iterator, bool> groupMapInsertPair = groupMap.insert(std::make_pair<string, KeyValueMap>(attrGroup, tempKeyValueMap));
            if(groupMapInsertPair.second == false)
            {
                errMsg = "insert " + attrGroup + " into groupMap failed";
                return false;
            }
            groupMapIt = groupMapInsertPair.first;
            KeyValueMapTemp = &(groupMapIt->second);
        }
        else
        {
            if(KeyValueMapTemp == NULL)
            {
                errMsg = "there is no groups";
                return false;
            }

            if(effecStr.find("=") == string::npos)
            {
                errMsg = effecStr + " there is no '='";
                return false;
            }

            string key,value;
            key = effecStr.substr(0, effecStr.find("="));
            LRtrim(key);
            value = effecStr.substr(effecStr.find("=") + 1);
            LRtrim(value);

            if(key.empty() || value.empty())
            {
                errMsg = "key='" + key + "' is empty or value='" + value + "' is empty";
                return false;
            }
            if(find_if(key.begin(), key.end(), std::ptr_fun(::isspace)) != key.end()
                    || find_if(value.begin(), value.end(), std::ptr_fun(::isspace)) != value.end())
            {
                errMsg = "key='" + key + "' has space or value='" + value + "' has space";
                return false;
            }

            std::pair<KeyValueMap::iterator, bool> keyValueMapTempInsertPair = KeyValueMapTemp->insert(std::make_pair<string, string>(key,value));
            if(keyValueMapTempInsertPair.second == false)
            {
                errMsg = "insert key'" + key + "' value'"+ value +"' into keyValueMap failed";
                return false;
            }
        }
        context = context.substr(context.find("\n") + 1);
    }
    return true;
}

void ReadConf::GetValue(const string &group, const string &key, int &value, int default_value)
{
    assert(!group.empty() || !group.empty());
    if(groupMap.find(group) == groupMap.end())
    {
        value = default_value;
        return;
    }

    GroupMap::iterator groupMapTempIt= groupMap.find(group);
    KeyValueMap keyValueMapTemp = groupMapTempIt->second;
    if(keyValueMapTemp.find(key) == keyValueMapTemp.end())
    {
        value = default_value;
        return;
    }

    KeyValueMap::iterator keyValueMapTempIt = keyValueMapTemp.find(key);
    if(sscanf(keyValueMapTempIt->second.c_str(), "%d", &value) != 1)
    {
        value = default_value;
        return;
    }
    return;
}


void ReadConf::GetValue(const string &group, const string &key, string &value, const string &default_value)
{
    assert(!group.empty() || !group.empty());
    if(groupMap.find(group) == groupMap.end())
    {
        value = default_value;
        return;
    }

    GroupMap::iterator groupMapTempIt= groupMap.find(group);
    KeyValueMap keyValueMapTemp = groupMapTempIt->second;
    if(keyValueMapTemp.find(key) == keyValueMapTemp.end())
    {
        value = default_value;
        return;
    }

    KeyValueMap::iterator keyValueMapTempIt = keyValueMapTemp.find(key);
    value = keyValueMapTempIt->second;
    return;

}

/*void ReadConf::Strim(string &effecStr)
{
    if(effecStr.empty())
    {
        return;
    }
    int i = 0, j = 0;
    string tempStr = effecStr;
    for(;i < tempStr.size(); ++i)
    {
        if(tempStr[i] != ' ' && tempStr[i] != '\t' && tempStr[i] != '\n')
        {
            effecStr[j++] = tempStr[i];
        }
    }
    effecStr.erase(j);
    return;
    //std::cout << retStr << std::endl;
}*/

