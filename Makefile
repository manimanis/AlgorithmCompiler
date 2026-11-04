CXX ?= g++
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -Iinclude
LDFLAGS ?=

SRCS_COMMON = src/lexer.cpp src/value.cpp src/parser.cpp src/interpreter.cpp src/semantic_analyzer.cpp
OBJS_COMMON = $(SRCS_COMMON:.cpp=.o)

SRCS_MAIN = src/python_transpiler.cpp src/main.cpp
OBJS_MAIN = $(SRCS_MAIN:.cpp=.o)

SRCS_TEST = src/test_runner.cpp
OBJS_TEST = $(SRCS_TEST:.cpp=.o)

ifeq ($(OS),Windows_NT)
    BIN_EXT := .exe
    CLEAN_CMD := del /Q src\*.o *.exe 2>nul || exit 0
else
    BIN_EXT :=
    CLEAN_CMD := rm -f src/*.o algo run_tests algo.exe run_tests.exe
endif

TARGET := algo$(BIN_EXT)
TEST_TARGET := run_tests$(BIN_EXT)

all: $(TARGET) $(TEST_TARGET)

$(TARGET): $(OBJS_COMMON) $(OBJS_MAIN)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

$(TEST_TARGET): $(OBJS_COMMON) $(OBJS_TEST)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(CLEAN_CMD)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

.PHONY: all clean test
