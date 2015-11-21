/*
 * Packet.h
 *
 *  Created on: 2015年11月20日
 *      Author: joe
 */

#ifndef PACKET_H_
#define PACKET_H_

enum PacketResult
{
    PACKETWAIT,
    PACKETINVALID,
    PACKETOK
};

#ifndef htonll
#define htonll(x) (((uint64_t)(htonl((uint32_t)((x)&0xffffffff)))<<32) | htonl((uint32_t)(((x)>>32)&0xffffffff)))
#define ntohll(x) (((uint64_t)(ntohl((uint32_t)((x)&0xffffffff)))<<32) | ntohl((uint32_t)(((x)>>32)&0xffffffff)))
#endif

struct PacketHead
{
    char magic_num[4];
    int32_t comment_ret;
    uint32_t body_size;
    uint32_t cmd;
    uint64_t tid;
    uint32_t uid;
};

const uint32_t max_body_size = 1000000;

class Packet
{
    ~Packet(){}
    int setHead(char *head_buff, uint32_t head_size, uint32_t body_size, uint32_t cmd, uint32_t tid, uint32_t uid, int32_t comment_ret);
    PacketResult isValid(char *data, uint32_t size, uint32_t &head_size, uint32_t &body_size);
};

int Packet::setHead(char *head_buff, uint32_t head_size, uint32_t body_size, uint32_t cmd, uint32_t tid, uint32_t uid, int32_t comment_ret)
{
    if(head_size < sizeof(PacketHead))
        return -1;
    if(body_size > max_body_size)
        return -2;
    PacketHead *header = (PacketHead*)head_buff;
    header->magic_num[0] = 'P';
    header->magic_num[1] = 'A';
    header->magic_num[2] = 'C';
    header->magic_num[3] = 'K';

    header->comment_ret = htonl(comment_ret);
    header->cmd = htonl(cmd);
    header->tid = htonll(tid);
    header->uid = htonl(uid);
    return sizeof(PacketHead);
}

PacketResult Packet::isValid(char *data, uint32_t size, uint32_t &head_size, uint32_t &body_size)
{
    if(size < sizeof(PacketHead))
        return PACKETWAIT;
    if(data[0] == 'P' && data[1] == 'A' && data[2] == 'C' && data[3] == 'K')
    {
        head_size = sizeof(PacketHead);
        PacketHead *head_data = (PacketHead*)data;
        uint32_t temp = ntohl(head_data->body_size);
        if(temp > max_body_size)
            return PACKETINVALID;
        body_size = temp;
        return PACKETOK;
    }
    return PACKETINVALID;
}


#endif /* PACKET_H_ */
