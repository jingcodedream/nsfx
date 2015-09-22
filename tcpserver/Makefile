INCLUDE=-I./ -I../
OBJDIR=../obj
SRCS=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp, $(OBJDIR)/%.o, $(SRCS))

all:${OBJS}

FLAGS=-g -D__LOG4CPLUS__

${OBJDIR}/%.o:%.cpp %.h
	g++ -fPIC ${INCLUDE} $< ${FLAGS} -c -o $@

clean:
	rm ${OBJS}
