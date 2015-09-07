/*
 * ConfReader.cpp
 *
 *  Created on: 2014-8-8
 *      Author: yongjinliu
 */

#include "ConfReader.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

namespace enet
{

int ConfReader::Init(const char *conf_file)
{
    m_ConfMap.clear();
    m_ErrMsg.clear();

    if(conf_file == NULL)
    {
        m_ErrMsg = "conf_file == NULL";
        return -1;
    }
    struct stat stat_buf;
    if(stat(conf_file, &stat_buf) == -1)
    {
        m_ErrMsg = "stat file failed";
        return -2;
    }
    FILE *fp = fopen(conf_file, "rb");
    if(fp == NULL)
    {
        m_ErrMsg = "open conf_file failed";
        return -3;
    }
    char *buffer = (char*)malloc(stat_buf.st_size+1);
    if(buffer == NULL)
    {
        m_ErrMsg = "malloc failed";
        fclose(fp);
        return -4;
    }
    size_t read_size = fread(buffer, 1, stat_buf.st_size, fp);
    if(read_size != stat_buf.st_size)
    {
        m_ErrMsg = "read conf_file failed";
        free(buffer);
        fclose(fp);
        return -5;
    }
    buffer[stat_buf.st_size] = '\0';
    if(!ParseConf(buffer))
    {
        free(buffer);
        fclose(fp);
    	return -6;
    }

    m_ErrMsg = "init conf succ";
    free(buffer);
    fclose(fp);
    return 0;
}

const char* ConfReader::GetKey(const char *cur_pos, const char *&key)
{
    key = NULL;
    while(*cur_pos==' ' || *cur_pos=='\t')
        ++cur_pos;
    if(*cur_pos=='#' || *cur_pos=='\r' || *cur_pos=='\n' || *cur_pos=='\0')  //注释或空白行
        return cur_pos;
    key = cur_pos;
    while(*cur_pos!='=' && *cur_pos!=' ' && *cur_pos!='\t' && *cur_pos!='\r' && *cur_pos!='\n' && *cur_pos!='\0' && *cur_pos!='#')
        ++cur_pos;
    return cur_pos;
}

const char* ConfReader::GetValue(const char *cur_pos, const char *&value)
{
	value = NULL;
    while(*cur_pos==' ' || *cur_pos=='\t' || *cur_pos=='=')
        ++cur_pos;
    if(*cur_pos=='#' || *cur_pos=='\r' || *cur_pos=='\n' || *cur_pos=='\0')  //不完整的行
        return cur_pos;
    value = cur_pos;
    while(*cur_pos!=' ' && *cur_pos!='\t' && *cur_pos!='\r' && *cur_pos!='\n' && *cur_pos!='\0' && *cur_pos!='#')
        ++cur_pos;
    return cur_pos;
}

const char* ConfReader::NextLine(const char *cur_pos)
{
    while(*cur_pos!='\r' && *cur_pos!='\n' && *cur_pos!='\0')
        ++cur_pos;
    if(*cur_pos == '\0')
        return NULL;
    ++cur_pos;
    return cur_pos;
}

bool ConfReader::ParseConf(const char *data)
{
    const char *parse_pos = data;
    ConfItemMap *item_map = NULL;
    do
    {
        const char *key = NULL;
        parse_pos = GetKey(parse_pos, key);
        if(key != NULL)
        {
            if(key[0] == '[')  //section name
            {
                string str_key;
                str_key.assign(key+1, parse_pos-key-2);
                ConfSectionMap::iterator section_it = m_ConfMap.find(str_key);
                if(section_it != m_ConfMap.end())
                {
                    m_ErrMsg = "section name duplicate:";
                    m_ErrMsg += str_key;
                    return false;
                }
                ConfItemMap item_map_temp;
                std::pair<ConfSectionMap::iterator, bool> insert_ret = m_ConfMap.insert(std::make_pair(str_key, item_map_temp));
                if(insert_ret.second == false)
                {
                    m_ErrMsg = "insert section name into map failed";
                    return false;
                }
                section_it = insert_ret.first;
                item_map = &(section_it->second);
            }
            else
            {
                string str_key;
                str_key.assign(key, parse_pos-key);
                if(item_map == NULL)
                {
                    m_ErrMsg = "item name does't include in section:";
                    m_ErrMsg += str_key;
                    return false;
                }
                ConfItemMap::iterator item_it = item_map->find(str_key);
                if(item_it != item_map->end())
                {
                    m_ErrMsg = "item name already exists in cur section:";
                    m_ErrMsg += str_key;
                    return false;
                }
                const char *value;
                parse_pos = GetValue(parse_pos, value);
                if(value == NULL)
                {
                    m_ErrMsg = "no value for item name:";
                    m_ErrMsg += str_key;
                    return false;
                }
                string str_value;
                str_value.assign(value, parse_pos-value);
                item_map->insert(std::make_pair(str_key, str_value));
            }
        }
    }while((parse_pos=NextLine(parse_pos)) != NULL);

    return true;
}

void ConfReader::GetValue(const char *section_key, const char *item_key, int &value, int default_value/*=-1*/)
{
    assert(section_key != NULL && item_key!=NULL);
    ConfSectionMap::iterator section_it = m_ConfMap.find(section_key);
    if(section_it == m_ConfMap.end())
    {
        value = default_value;
        return;
    }

    ConfItemMap &item_map = section_it->second;
    ConfItemMap::iterator item_it = item_map.find(item_key);
    if(item_it == item_map.end())
    {
        value = default_value;
        return;
    }

    string &str_value = item_it->second;
    if(sscanf(str_value.c_str(), "%d", &value) != 1)
        value = default_value;
}

void ConfReader::GetValue(const char *section_key, const char *item_key, string &value, const char *default_value/*=""*/)
{
    assert(section_key != NULL && item_key!=NULL);
    ConfSectionMap::iterator section_it = m_ConfMap.find(section_key);
    if(section_it == m_ConfMap.end())
    {
        value = default_value;
        return;
    }

    ConfItemMap &item_map = section_it->second;
    ConfItemMap::iterator item_it = item_map.find(item_key);
    if(item_it == item_map.end())
    {
        value = default_value;
        return;
    }

    value = item_it->second;
}

void ConfReader::AllConf(string &conf_str)
{
    conf_str.clear();
    ConfSectionMap::iterator section_it;
    for(section_it=m_ConfMap.begin(); section_it!=m_ConfMap.end(); ++section_it)
    {
        char buf[1024];
        snprintf(buf, 1024, "[%s]\n", section_it->first.c_str());
        conf_str.append(buf);

        ConfItemMap &item_map = section_it->second;
        ConfItemMap::iterator item_it;
        for(item_it=item_map.begin(); item_it!=item_map.end(); ++item_it)
        {
            snprintf(buf, 1024, "%s = %s\n", item_it->first.c_str(), item_it->second.c_str());
            conf_str.append(buf);
        }
        snprintf(buf, 1024, "\n\n");
        conf_str.append(buf);
    }
}

}

