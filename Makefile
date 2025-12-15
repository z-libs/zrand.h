
CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -std=c11 -O2 -I.
CXXFLAGS = -Wall -Wextra -std=c++11 -O2 -I.

all: get_zmath_h

get_zmath_h:
	@echo "Using wget to add 'zmath.h'..."
	wget -q "https://raw.githubusercontent.com/z-libs/zmath.h/main/zmath.h" -O "zmath.h"

clean:
	@echo "Removing 'zmath.h'..."
	@rm zmath.h

test: get_zmath_h test_c test_cpp clean

test_c:
	@echo "----------------------------------------"
	@echo "Building C Tests..."
	@$(CC) $(CFLAGS) tests/test_main.c -o tests/runner_c
	@./tests/runner_c
	@rm tests/runner_c

test_cpp:
	@echo "----------------------------------------"
	@echo "Building C++ Tests..."
	@$(CXX) $(CXXFLAGS) tests/test_cpp.cpp -o tests/runner_cpp
	@./tests/runner_cpp
	@rm tests/runner_cpp

.PHONY: all get_zmath_h test test_c test_cpp clean

