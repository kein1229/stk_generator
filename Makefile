CC = g++
CPPFLAGS = -g 

SRCS = $(wildcard *.cpp)
OBJECTS = $(SRCS:.cpp=.o) #Call.o, CallMgr.o, Data.o, main.o, mc.o
all : graphgen

graphgen: $(OBJECTS)
	$(CC) $(CPPFLAGS) -o $@ $^
clean:
	rm -f *.o
	rm -f graphgen 
