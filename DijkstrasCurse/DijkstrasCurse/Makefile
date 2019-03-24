
CXX=g++
CPPFLAGS=-g -std=c++14
LDFLAGS=-g
LDLIBS=-lm

SRCS=\
	main.cpp \
	ConsoleLinux.cpp \
	levels.cpp

OBJS=$(subst .cpp,.o,$(SRCS))

all: dj

test: dj
	-rm test.data
	-rm test.hex
	-./dj | xdd > test.hex
	-./dj > test.data

debug: dj
	gdb ./dj

dj: $(OBJS)
	g++ $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

ConsoleLinux.o: ConsoleLinux.cpp Console.h e4b.h
main.o: main.cpp defs.h Console.h e4b.h
levels.o: levels.cpp defs.h

clean:
	rm *.o

