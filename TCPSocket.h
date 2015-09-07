/*
 * TCPSocket.h
 *
 *  Created on: 2014-6-26
 *      Author: yongjinliu
 */
// 支持跨平台: linux, windows

#ifndef _TCPSOCKET_H_
#define _TCPSOCKET_H_


#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>

class TCPSocket
{
public:
    //监听端口,ip表示监听哪个网卡接口的ip,NULL时监听所有网卡;reuse:端口在已监听情况下,是否允许再次监听该端口
    static int Listen(int port, bool block=false, int backlog=SOMAXCONN, bool reuse=true);
    static int Listen(const char *ip, int port, bool block=false, int backlog=128, bool reuse=true);
    //接收链接
    static int Accept(int fd);
    //接收链接
    static int Accept(int fd, struct sockaddr_in &peer_addr);


    //连接到指定的ip和端口,wait_ms在block=false时有效,表示等待连接的毫秒数,在这个时间内还未连接上的话返回失败.
    //连接成功返回fd,失败返回负数(具体值表示哪一步失败)
    static int Connect(const char *ip, int port, bool block=false, int wait_ms=1000);


    //接收最多size个字节数,成功返回实际接收的字节数.0表示对方断开连接;负数表示失败(如果是非阻塞模式,看具体的errno)
    static int Recv(int fd, char *buffer, int size);
    //接收指定的size字节数.成功返回值大小为size;0表示对方断开连接;负数表示失败(如果是非阻塞模式,看具体的errno)
    static int RecvAll(int fd, char *buffer, int size);

    //发送最多size字节,成功返回实际发送的字节数
    static int Send(int fd, const char *buffer, int size);
    //发送全部指定的size字节,成功返回实际发送的字节数
    static int SendAll(int fd, const char *buffer, int size);


    //创建socket,返回值大于0表示fd, 小于0表示失败(具体值表示哪一步失败)
    static int CreateSocket(bool block=true, bool close_exec=false);


    //判断addr是否是点分的ip地址
    static bool IsIp(const char *addr);
    //是否阻塞模式:0非阻塞;1阻塞;-1错误
    static int IsBlock(int fd);
    //设置socket为阻塞状态
    static int SetBlock(int fd);
    //设置socket为非阻塞状态
    static int SetNoBlock(int fd);
    //设置close_exec
    static bool SetCloseExec(int fd);

private:
    TCPSocket(){};
    void operator =(TCPSocket &ts);
};



inline
bool TCPSocket::IsIp(const char *addr)
{
    if(addr == NULL)
        return false;
    int d[4];
    int32_t i = sscanf(addr, "%d.%d.%d.%d",&d[0], &d[1], &d[2], &d[3]);
    if(i != 4)
        return false;
    if(d[0]<0||d[0]>255
            ||d[1]<0||d[1]>255
            ||d[2]<0||d[2]>255
            ||d[3]<0||d[3]>255)
        return false;
    for(i=0; addr[i]!='\0'; ++i) //判断是否有其他非法字符
        if(addr[i]!='.' && (addr[i]<'0'||addr[i]>'9'))
            return false;
    return true;
}

inline
int TCPSocket::IsBlock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if(flags == -1)
        return -1;
    return ((flags&O_NONBLOCK)==0)?1:0;
}

inline
int TCPSocket::SetBlock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if(flags == -1)
        return -1;
    if((flags&O_NONBLOCK) == 0)
        return 0;
    flags &= ~O_NONBLOCK;
    if(fcntl(fd, F_SETFL, flags) == -1)
        return -2;
    return 0;
}

inline
int TCPSocket::SetNoBlock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if(flags == -1)
        return -1;
    if((flags&O_NONBLOCK) != 0)
        return 0;
    flags |= O_NONBLOCK;
    if(fcntl(fd, F_SETFL, flags) == -1)
        return -2;
    return 0;
}

inline
bool TCPSocket::SetCloseExec(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if(flags == -1)
        return false;
    flags |= FD_CLOEXEC; //close on exec
    if(fcntl(fd, F_SETFL, flags) == -1)
        return false;
    return true;
}

inline
int TCPSocket::CreateSocket(bool block/*=true*/, bool close_exec/*=false*/)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1)
        return -1;

    if(block==true && close_exec==false)
        return fd;
    
    int flags = fcntl(fd, F_GETFL, 0);
    if(flags == -1)
    {
        close(fd);
        return -2;
    }
    if(block == false)  //no block
        flags |= O_NONBLOCK;
    if(close_exec == true)
        flags |= FD_CLOEXEC; //close on exec

    if(fcntl(fd, F_SETFL, flags) == -1)
    {
        close(fd);
        return -3;
    }
    return fd;
}

inline
int TCPSocket::Connect(const char *ip, int port, bool block/*=false*/, int wait_ms/*=1000*/)
{
    int fd = -1;
    if((fd=CreateSocket(block, true)) == -1)
        return -1;

    //连接到服务器
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if(IsIp(ip))
    {
        addr.sin_addr.s_addr = inet_addr(ip);
    }
    else
    {
        struct hostent *hent = gethostbyname(ip);
        if(hent==NULL || (hent->h_addrtype!=AF_INET && hent->h_addrtype!=AF_INET6))
        {
            close(fd);
            return -2;
        }
        addr.sin_addr.s_addr = inet_addr(hent->h_addr);
    }

    if(connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == 0) //成功直接返回
    {
        return fd;
    }
    if(block || (errno!=EINPROGRESS&&errno!=EWOULDBLOCK&&errno!=EINTR))
    {
        close(fd);
        return -3;
    }

    if(wait_ms == 0) //不需要等直接返回
        return fd;
        
    //非阻塞并且等待建立连接，利用select模拟超时设置
    struct timeval tval;
    fd_set rset, wset;
    FD_ZERO(&rset);
    FD_SET(fd, &rset);
    FD_ZERO(&wset);
    FD_SET(fd, &wset);

    tval.tv_sec = wait_ms/1000;
    tval.tv_usec = (wait_ms%1000)*1000;

    int32_t tmp = select(fd+1, (fd_set*)&rset, (fd_set*)&wset, (fd_set*)NULL, &tval);
    if (tmp <= 0)    //如果超时了没有
    {
        close(fd);
        return -4;
    }

    if(FD_ISSET(fd, &rset) || FD_ISSET(fd, &wset))
    {
        int error;
        int len = sizeof(error);

        tmp = getsockopt(fd, SOL_SOCKET, SO_ERROR, (void*)&error, (socklen_t*)&len);
        if(tmp<0 || (tmp==0&&error!=0))    //查看是否有错误
        {
            close(fd);
            return -5;
        }
    }
    return fd;
}

inline
int TCPSocket::Listen(int port, bool block/*=false*/, int backlog/*=128*/, bool reuse/*=true*/)
{
    return Listen(NULL, port, block, backlog, reuse);
}

inline
int TCPSocket::Listen(const char *ip, int port, bool block/*=false*/, int backlog/*=128*/, bool reuse/*=true*/)
{
    int fd = CreateSocket(block, true);
    if(fd == -1)
        return -1;


    if(reuse == true)
    {
        int _reuse = 1;

        if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void*)&_reuse, sizeof(_reuse)) == -1)
        {
            close(fd);
            return -2;
        }
    }

    //绑定到端口
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if(ip==NULL ||ip[0]=='\0')
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    else if(IsIp(ip)) //ip地址
        addr.sin_addr.s_addr = inet_addr(ip);
    else //域名
    {
        struct hostent *hent = gethostbyname(ip);
        if(hent==NULL || (hent->h_addrtype!=AF_INET && hent->h_addrtype!=AF_INET6))
        {
            close(fd);
            return -3;
        }
        addr.sin_addr.s_addr = inet_addr(hent->h_addr);
    }

    if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
        close(fd);
        return -4;
    }

    if(listen(fd, backlog) == -1)
    {
        close(fd);
        return -5;
    }

    return fd;
}

inline
int TCPSocket::Recv(int fd, char *buffer, int size)
{
    while(true)
    {
        int temp = recv(fd, buffer, size, 0);
        if(temp<0 && errno==EINTR)
        {
            continue;
        }
        return temp;
    }
    return 0;
}

inline
int TCPSocket::RecvAll(int fd, char *buffer, int size)
{
    int total_recv = 0;
    while(total_recv < size)
    {
        int temp = recv(fd, buffer+total_recv, size-total_recv, 0);
        if(temp==0 || (temp==-1&&errno!=EAGAIN&&errno!=EINTR&&errno!=EWOULDBLOCK))
        {
            return temp;
        }

        if(temp == -1)
        {
            continue;
        }

        total_recv += temp;
    }
    return total_recv;
}

inline
int TCPSocket::Send(int fd, const char *buffer, int size)
{
    return send(fd, buffer, size, MSG_NOSIGNAL);
}

inline
int TCPSocket::SendAll(int fd, const char *buffer, int size)
{
    int total_send = 0;
    while(total_send < size)
    {
        int temp = send(fd, buffer+total_send, size-total_send, MSG_NOSIGNAL);

        if(temp==-1 && errno!=EAGAIN && errno!=EINTR && errno!=EWOULDBLOCK)
        {
            return temp;
        }

        if(temp == -1)
        {
            continue;
        }

        total_send += temp;
    }
    return total_send;
}

inline
int TCPSocket::Accept(int fd)
{
    int new_fd = accept(fd, NULL, NULL);
    return new_fd >=0 ?new_fd:-1;
}

inline
int TCPSocket::Accept(int fd, struct sockaddr_in &peer_addr)
{
    socklen_t peer_addr_len=sizeof(peer_addr);
    int new_fd = accept(fd, (struct sockaddr*)&peer_addr, &peer_addr_len);
    return new_fd >=0 ?new_fd:-1;
}


#endif /* _TCPSOCKET_H_ */
