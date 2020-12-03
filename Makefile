CC=gcc
CXX=g++

CCFLAGS=-Wall -Wpedantic -g
CXXFLAGS=$(CFLAGS)

LIBS=-lglfw -lGL

C_SRCMODULES=
C_OBJMODULES=$(C_SRCMODULES:.c=.o)

all: run

run: main
	./main

main: main.o $(C_OBJMODULES)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm *.o main
