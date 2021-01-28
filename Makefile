CXX=g++
CXXFLAGS=-Wall -Wpedantic -g

LIBS=-lglfw -lGL -lSOIL -lassimp

SRCMODULES=shader_program.cpp camera.cpp mesh.cpp model.cpp
OBJMODULES=$(SRCMODULES:.cpp=.o)

all: run

run: main
	./main

main: main.o $(OBJMODULES) camera.hpp
	$(CXX) $(CXXFLAGS) main.o $(OBJMODULES) -o $@ $(LIBS)

%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) -c $<

shader_program.o: shader_program.cpp shader.hpp
	$(CXX) $(CXXFLAGS) -c shader_program.cpp

clean:
	rm *.o main
