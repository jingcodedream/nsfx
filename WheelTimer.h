/*
 * WheelTimer.h
 *
 *  Created on: 2015年11月17日
 *      Author: joe
 */

#ifndef WHEELTIMER_H_
#define WHEELTIMER_H_

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ObjectPool.h"

const int wheel_slot_num = 64;
const int wheel_ms = 0;
const int wheel_second = 1;
const int wheel_minute = 2;
const int wheel_hour = 3;
const int wheel_num = 4;

struct TimerNode {
    int temp;
    uint64_t ts;
    struct TimerNode *pre;
    struct TimerNode *next;
};

struct TimerWheel{
    int index; //当前轮的指向
    struct TimerNode slot[wheel_slot_num];
};

class WheelTimer {
public:
    WheelTimer():m_now_ms(0){
        memset(m_timer_wheel, 0, sizeof(m_timer_wheel));
    }

    void start();
    TimerNode *add(uint64_t timeout_ms, int temp);
    bool del(TimerNode *timer_node);
    bool checkTimeout(uint64_t timeout_ms);
private:
    uint64_t m_now_ms;
    struct TimerWheel m_timer_wheel[wheel_num];
    ObjectPool<TimerNode> m_timer_node_pool;

    inline int getHourIndex(uint64_t now_ms)
    {
        return (now_ms >> 22) & 0x3F;
    }
    inline int getMinuteIndex(uint64_t now_ms)
    {
        return (now_ms >> 16) & 0x3F;
    }
    inline int getSecondIndex(uint64_t now_ms)
    {
        return (now_ms >> 10) & 0x3F;
    }
    inline int getMsIndex(uint64_t now_ms)
    {
        return (now_ms >> 4) & 0x3F;
    }
    inline void insertAfter(TimerNode* at, TimerNode* node) //插入某个节点后面
    {
        if(at->next != NULL)
            at->next->pre = node;
        node->next = at->next;
        at->next = node;
        node->pre = at;
    }
};

#endif /* WHEELTIMER_H_ */
