# Default output name
EXE ?= weak
ifeq ($(OS),Windows_NT)
	EXE := $(EXE).exe
endif

# Compiler and flags
CXX := g++
CXXFLAGS := -O3 -march=native -std=c++17

SOURCES := $(wildcard *.cpp)

# Detect current GCC version (major)
GCC_VERSION := $(shell $(CXX) -dumpfullversion | cut -d. -f1 2>/dev/null || echo 0)

all:
	@if [ $(GCC_VERSION) -lt 13 ]; then \
		echo "Error: GCC version $(GCC_VERSION) detected. Please use GCC 13 or newer."; \
		exit 1; \
	else \
		$(CXX) $(CXXFLAGS) $(SOURCES) -o $(EXE); \
	fi

clean:
	rm -f *.o *.exe Engine-* weak

