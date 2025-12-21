
CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -std=c11 -O2 -I. -Ideps
CXXFLAGS = -Wall -Wextra -std=c++11 -O2 -I. -Ideps

GEN_DIR = z-core
GEN_EXE = $(GEN_DIR)/zdoc_gen
HEADER  = zrand.h
DOC_IN  = README.in
DOC_OUT = README.md

DEPS_DIR = deps
URL_ZMATH = https://raw.githubusercontent.com/z-libs/zmath.h/main/zmath.h
URL_ZSTR  = https://raw.githubusercontent.com/z-libs/zstr.h/main/zstr.h
URL_ZFILE = https://raw.githubusercontent.com/z-libs/zfile.h/main/zfile.h

all: test

get_dependencies:
	@echo "Creating dependencies directory..."
	@mkdir -p $(DEPS_DIR)
	@echo "Downloading zmath.h..."
	@wget -q $(URL_ZMATH) -O $(DEPS_DIR)/zmath.h
	@echo "Downloading zstr.h..."
	@wget -q $(URL_ZSTR) -O $(DEPS_DIR)/zstr.h
	@echo "Downloading zfile.h..."
	@wget -q $(URL_ZFILE) -O $(DEPS_DIR)/zfile.h

clean:
	@echo "Cleaning artifacts..."
	@rm -rf $(DEPS_DIR)
	@rm -f $(GEN_EXE)
	@rm -f tests/runner_c tests/runner_cpp

test: get_dependencies test_c test_cpp

test_c:
	@echo "----------------------------------------"
	@echo "Building C Tests..."
	@$(CC) $(CFLAGS) tests/test_main.c -o tests/runner_c
	@./tests/runner_c

test_cpp:
	@echo "----------------------------------------"
	@echo "Building C++ Tests..."
	@$(CXX) $(CXXFLAGS) tests/test_cpp.cpp -o tests/runner_cpp
	@./tests/runner_cpp

$(GEN_EXE): $(GEN_DIR)/zdoc_gen.c | get_dependencies
	@echo "Compiling Doc Generator..."
	@$(CC) $(CFLAGS) -I$(GEN_DIR) -o $@ $<

docs: $(GEN_EXE)
	@echo "Updating $(DOC_OUT)..."
	@$(GEN_EXE) $(HEADER) $(DOC_OUT) $(DOC_IN)

.PHONY: all get_dependencies clean test test_c test_cpp docs
