INCLUDE=-I./
OBJDIR=./obj
SRCS=${wildcard *.cpp}
OBJS=${patsubst %.cpp, ${OBJDIR}/%.o, ${SRCS}}
LIBNAME=libnsfx.a

all:${OBJS} tcpsvr lib

FLAGS=-g -D__LOG4CPLUS__

${OBJDIR}/%.o:%.cpp
	g++ -fPIC ${INCLUDE} $< ${FLAGS} -c -o $@

tcpsvr:
	(cd tcpserver;${MAKE} all)

lib:
	ar -cr ${LIBNAME} ${OBJDIR}/*.o

clean:
	rm ${OBJDIR}/*.o
