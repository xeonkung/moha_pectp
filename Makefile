TARGET = gaour
CXXFLAGS = -Wall -ansi -O3

OBJS = Problem.o Solution.o Random.o Timer.o Control.o util.o

all: ${TARGET}

gaour: ga.cpp $(OBJS)
	${CXX} ${CXXFLAGS} -o $@ $^

clean:
	@rm -f *~ *.o ${TARGET} core DEADJOE
