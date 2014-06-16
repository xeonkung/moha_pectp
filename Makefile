TARGET = moha
CXXFLAGS = -Wall -ansi -O3

OBJS = Problem.o Solution.o Random.o Timer.o Control.o util.o Distance.o Ranking.o ObjectiveCmp.o

all: ${TARGET}

moha: ga.cpp $(OBJS)
	${CXX} ${CXXFLAGS} -o $@ $^

clean:
	@rm -f *~ *.o ${TARGET} core DEADJOE
