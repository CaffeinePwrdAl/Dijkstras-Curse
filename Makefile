
CXX=g++
CPPFLAGS=-g -std=c++14 -Wall -Wpedantic -Wextra
LDFLAGS=-g
LDLIBS=-lm

SRC_ROOT=src

SRCS=\
	$(SRC_ROOT)/main.cpp \
	$(SRC_ROOT)/console-linux.cpp \
	$(SRC_ROOT)/levels.cpp

OBJS=$(subst .cpp,.o,$(SRCS))

all: dijkstras-curse

dijkstras-curse: $(OBJS)
	g++ $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

console-linux.o: console-linux.cpp console.h e4b.h
main.o: main.cpp defs.h console.h e4b.h
levels.o: levels.cpp defs.h

clean:
	rm $(SRC_ROOT)/*.o
