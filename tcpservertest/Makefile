LIBS=/usr/local/lib/liblog4cplus.a -lpthread ../libnsfx.a

FLAGS=-g -D__LOG4CPLUS__

all:Main.cpp TCPServerTest.h TCPServerTest.cpp
	g++ Main.cpp TCPServerTest.h TCPServerTest.cpp ${LIBS} ${FLAGS} -o TCPServerTest

clean:
	rm TCPServerTest

