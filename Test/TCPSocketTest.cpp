#include "../TCPSocket.h"
#include <iostream>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/syscall.h>
using namespace std;

#define MAXSIZE 65535

int main(int args, char **argv)
{
    //int fd = TCPSocket::Socket();
    /*time_t start = time(NULL);

    int fd = TCPSocket::Connect("12.0.0.1", 64447, false, 5000);
    cout << fd << endl;
    time_t end = time(NULL);
    cout << end - start << endl;*/

    //int fd = socket(AF_INET, SOCK_STREAM, 0);

    /*int fd = TCPSocket::Listen(64447, false);
    if(fd < 0)
    {
        cout << "fd=" << fd << endl;
        return -1;
    }

    int cl_fd = TCPSocket::Accept(fd);
    if(cl_fd < 0)
    {
        cout << cl_fd << endl;
        cout << ETIMEDOUT << endl;
        return -1;
    }*/


    if(fork() == 0)
    {
        sleep(1);
        int cl_fd = TCPSocket::Connect("127.0.0.1", 64545);
        assert(cl_fd > 0);
        char buff[1024];
        for(int i = 0; i < 1023; ++i)
        {
            buff[i] = 'a';
        }
        //snprintf(buff, sizeof(buff), "%s", "hello boy");
        int ret = TCPSocket::Send(cl_fd, buff, sizeof(buff));
        assert(ret >= 0);
        ret = TCPSocket::Send(cl_fd, buff, sizeof(buff));
        assert(ret >= 0);
        sleep(10);
        close(cl_fd);
    }
    else
    {
        int listen_fd = TCPSocket::Listen(64545, true);
        assert(listen_fd >= 0);
        int client_fd = TCPSocket::Accept(listen_fd);
        assert(client_fd >= 0);
        //int flags = fcntl(client_fd, F_GETFL, 0);
        //flags |= O_NONBLOCK;
        //int ret = fcntl(client_fd, F_SETFL, flags);
        //assert(ret == 0);
        int ret = 0;
        char buff[2048];
        //sleep (3);
        while(true)
        {

            ret = TCPSocket::Recv(client_fd, buff, sizeof(buff));
            if(ret < 0)
            {
                cout << strerror(errno) << endl;
                break;
            }
            if(ret == 0)
            {
                break;
            }

            assert(ret >= 0);
            cout << ret << endl;
            //cout << buff << endl;
            for(int i = 0; i< sizeof(buff); ++i)
            {
                cout << buff[i] ;
            }
            cout << endl;
            memset(buff, 0, sizeof(buff));
        }
        //sleep(2);
        close(client_fd);
        close(listen_fd);
    }
    /*struct stat buf;
    FILE *fl = fopen("../Makefile", "r");
    if(fl == NULL)
    {
        cout << "open failed" << endl;
    }
    char path[1024];
    if(getcwd(path, sizeof(path)) == NULL)
    {
        cout << "getcwd failed" << endl;
    }
    cout << path << endl;
    if(stat("../ConfReader.cpp", &buf) == -1)
    {
        return errno;
    }
    cout << buf.st_size << endl;*/

    return 0;
}
