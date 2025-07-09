# Default output name
EXE ?= weak
ifeq ($(OS),Windows_NT)
	EXE := $(EXE).exe
endif

# Compiler and flags
CXX := g++
CXXFLAGS := -O3 -march=native -std=c++17

SOURCES := $(wildcard *.cpp)

all:
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(EXE)

clean:
	rm -f *.o *.exe Engine-* weak
