/*
 * FIFOTest2.cpp
 *
 *  Created on: 2015年8月13日
 *      Author: joe
 */


#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
using namespace std;

#define FIFO_SERVER "./tmptest"

int main()
{
    int fd = open(FIFO_SERVER, O_RDONLY);
    cout << strerror(errno) << endl;
    assert(fd >= 0);
    char r_buf[4096 * 2];
    memset(r_buf, 0, sizeof(r_buf));
    sleep(10);
    int ret_size = read(fd, r_buf, sizeof(r_buf));
    cout << r_buf << endl;

    close(fd);
}

