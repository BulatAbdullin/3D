CC=gcc
CXX=g++

CCFLAGS=-Wall -Wpedantic -g
CXXFLAGS=$(CFLAGS)

LIBS=-lglfw -lGL

C_SRCMODULES=callback.c
C_OBJMODULES=$(C_SRCMODULES:.c=.o)

all: run

run: main
	./main

main: main.o $(C_OBJMODULES)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c $<

callback.o: callback.c callback.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm *.o main
