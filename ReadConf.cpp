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
#include "ReadConf.h"

#include <iostream>  //test

int ReadConf::Init(const char *fname){

    rfItemMap.clear();
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
        errMsg = "parse conf failed";
        return -4;
    }
    errMsg = "parse conf succ";
    return 0;
}

bool ReadConf::ParseConf(const string &buff)
{
    string context = buff;
    while(context.find("\n") != string::npos)
    {
        string line = context.substr(0, context.find("\n"));

        context = context.substr(context.find("\n") + 1);
        std::cout << line << std::endl;
    }
    return true;
}


