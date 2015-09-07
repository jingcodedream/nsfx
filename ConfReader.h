/*
 * ConfReader.h
 *
 *  Created on: 2014-8-8
 *      Author: yongjinliu
 */

#ifndef CONFREADER_H_
#define CONFREADER_H_

#include <string>
#include <map>
using std::string;
using std::map;

namespace enet
{

class ConfReader
{
public:
    int Init(const char *conf_file);
    string GetErrMsg(){return m_ErrMsg;}
    void GetValue(const char *section_key, const char *item_key, int &value, int default_value=-1);
    void GetValue(const char *section_key, const char *item_key, string &value, const char *default_value="");

    void AllConf(string &conf_str);
private:
    typedef map<string/*item key*/, string/*item value*/> ConfItemMap;
    typedef map<string/*section key*/, ConfItemMap> ConfSectionMap;
    ConfSectionMap m_ConfMap;
    string m_ErrMsg;

    bool ParseConf(const char *data);
    const char* NextLine(const char *cur_pos);
    const char* GetKey(const char *cur_pos, const char *&key);
    const char* GetValue(const char *cur_pos, const char *&value);
};

}
#endif /* CONFREADER_H_ */
