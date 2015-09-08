/*
 * FIFOTest1.cpp
 *
 *  Created on: 2015年8月13日
 *      Author: joe
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
using namespace std;

#define FIFO_SERVER "./tmptest"

int main()
{
    //cout << oct << (01|04000) << endl;
    int ret = mkfifo(FIFO_SERVER, O_CREAT|O_EXCL);
    assert(ret != -1 || errno == EEXIST);
    char buf[1024];
    getcwd(buf, sizeof(buf));
    cout << buf << endl;
    int fd = open(FIFO_SERVER, 01|04000);
    cout << strerror(errno) << endl;
    assert(fd >= 0);

    char w_buf[2 * 4096];
    memset(w_buf, 0, sizeof(w_buf));
    strncpy(w_buf, "hahahahahahahahahahah", sizeof(w_buf));
    int real_wnum = write(fd, w_buf, 100);
    if(real_wnum == -1)
    {
        cout << "error" << endl;
        if(errno == EAGAIN)
            cout << "try again" << endl;
    }
    else
    {

    }
    close(fd);
    return 0;
}
