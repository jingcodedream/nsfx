#include "../TCPSocket.h"
#include <iostream>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/syscall.h>
int main()
{
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
				usleep(1);
				        ret = TCPSocket::Send(cl_fd, buff, sizeof(buff));
						        assert(ret >= 0);
								        close(cl_fd);
										}
