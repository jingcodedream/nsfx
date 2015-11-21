/*
 * WheelTimer.cpp
 *
 *  Created on: 2015年11月17日
 *      Author: joe
 */

#include <sys/time.h>
#include <iostream>

#include "WheelTimer.h"


void WheelTimer::start()
{
    struct timeval tm;
    gettimeofday(&tm, NULL);
    m_now_ms = tm.tv_sec * 1000 + tm.tv_usec/1000;
    m_timer_wheel[wheel_hour].index = getHourIndex(m_now_ms);
    m_timer_wheel[wheel_minute].index = getMinuteIndex(m_now_ms);
    m_timer_wheel[wheel_second].index = getSecondIndex(m_now_ms);
    m_timer_wheel[wheel_ms].index = getMsIndex(m_now_ms);
}

TimerNode *WheelTimer::add(uint64_t timeout_ms, int temp)
{
    TimerNode *temp_head;
    if(getHourIndex(m_now_ms) != m_timer_wheel[wheel_hour].index)
    {
        temp_head = &m_timer_wheel[wheel_hour].slot[getHourIndex(m_now_ms)];
    }
    else if(getMinuteIndex(m_now_ms) != m_timer_wheel[wheel_minute].index)
    {
        temp_head = &m_timer_wheel[wheel_minute].slot[getMinuteIndex(m_now_ms)];
    }
    else if(getSecondIndex(m_now_ms) != m_timer_wheel[wheel_second].index)
    {
        temp_head = &m_timer_wheel[wheel_second].slot[getSecondIndex(m_now_ms)];
    }
    else
    {
        temp_head = &m_timer_wheel[wheel_ms].slot[getMsIndex(m_now_ms)];
    }

    TimerNode *temp_node = m_timer_node_pool.get();
    temp_node->temp = temp;
    temp_node->ts = timeout_ms;
    insertAfter(temp_head, temp_node);
    return temp_node;
}

bool WheelTimer::del(TimerNode *timer_node)
{
    if(timer_node->pre == NULL)
        return false;
    if(timer_node->pre->next != timer_node)
        return false;
    if(timer_node->next && timer_node->next->pre!=timer_node)
        return false;
    timer_node->pre->next = timer_node->next;
    if(timer_node->next)
        timer_node->next->pre = timer_node->pre;
    m_timer_node_pool.put(timer_node);
    return true;
}

bool WheelTimer::checkTimeout(uint64_t timeout_ms)
{
    if(timeout_ms < m_now_ms)  //外部时间被修改
    {
        return false;
    }
    else if((timeout_ms-m_now_ms) > 0xFFFFFFF)  //超过一个周期时间没有检查
    {
        return false;
    }

    for(int count = (timeout_ms>>4) - (m_now_ms>>4);count > 0; --count)
    {
        TimerNode *temp_node = m_timer_wheel[wheel_ms].slot[m_timer_wheel[wheel_ms].index].next;
        if(temp_node != NULL)
        {
            std::cout << temp_node->temp << std::endl;
            TimerNode *temp_temp_node = temp_node;
            temp_node = temp_node->next;
            m_timer_node_pool.put(temp_temp_node);
        }
        m_timer_wheel[wheel_ms].slot[m_timer_wheel[wheel_ms].index].next = NULL;
        m_timer_wheel[wheel_ms].index = (m_timer_wheel[wheel_ms].index + 1)%wheel_slot_num;
        if(m_timer_wheel[wheel_ms].index == 0)//毫秒针转过一圈
        {
            int wheel = wheel_second;
            for(; wheel < wheel_num; ++wheel)
            {
                m_timer_wheel[wheel].index = (m_timer_wheel[wheel].index + 1)%wheel_slot_num;
                if(m_timer_wheel[wheel].index == 0)
                    continue;
                break;
            }

            if(wheel == wheel_num)
            {
                wheel = wheel_hour;
            }

            //始钟降级
            for(; wheel > wheel_ms; --wheel)
            {
                TimerNode *temp_node = m_timer_wheel[wheel].slot[m_timer_wheel[wheel].index].next;
                if(temp_node == NULL)
                {
                    continue;
                }

                while(temp_node != NULL)
                {
                    int index_to = 0;
                    if(wheel == wheel_hour)
                        index_to = getMinuteIndex(temp_node->ts);
                    else if(wheel == wheel_minute)
                        index_to = getSecondIndex(temp_node->ts);
                    else if(wheel == wheel_second)
                        index_to = getMsIndex(temp_node->ts);
                    TimerNode *temp_temp_node = temp_node;
                    temp_node = temp_node->next;

                    insertAfter(&m_timer_wheel[wheel-1].slot[index_to], temp_temp_node);
                }
                m_timer_wheel[wheel].slot[m_timer_wheel[wheel].index].next == NULL;
            }
        }
    }
    m_now_ms = timeout_ms;
    return true;
}

