CXX=g++
CXXFLAGS=-Wall -Wpedantic -g

LIBS=-lglfw -lGL -lSOIL -lassimp

SRCMODULES=shader_program.cpp camera.cpp mesh.cpp model.cpp skybox.cpp
OBJMODULES=$(SRCMODULES:.cpp=.o)

all: run

run: main_2
	./main_2

main: main.o $(OBJMODULES) camera.hpp
	$(CXX) $(CXXFLAGS) main.o $(OBJMODULES) -o $@ $(LIBS)

main_2: main_2.o $(OBJMODULES) camera.hpp skybox.hpp
	$(CXX) $(CXXFLAGS) main_2.o $(OBJMODULES) -o $@ $(LIBS)

%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) -c $<

shader_program.o: shader_program.cpp shader.hpp
	$(CXX) $(CXXFLAGS) -c shader_program.cpp

clean:
	rm *.o main main_2
