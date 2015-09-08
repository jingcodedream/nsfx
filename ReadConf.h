/*
 * ReadConf.h
 *
 *  Created on: 2015年9月8日
 *      Author: joe
 */

#ifndef READCONF_H_
#define READCONF_H_

#include <string>
#include <map>
using std::string;

class ReadConf{
public:
    ReadConf(){}
    ~ReadConf(){}
    string GetErrMsg(){return errMsg;}
    //ReadConf operator=(const ReadConf &cf){return cf;}//fault

    int Init(const char *fname);
private:
    typedef std::map<string, string> RfItemMap;
    RfItemMap rfItemMap;
    string errMsg;

    bool ParseConf(const string &buff);
};

#endif /* READCONF_H_ */
