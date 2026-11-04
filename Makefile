CXX = C:/msys64/ucrt64/bin/g++.exe
CXXFLAGS = -std=c++20 -O2 -Wall -Wextra -Iinclude
LDFLAGS = -static

SRCS_COMMON = src/lexer.cpp src/value.cpp src/parser.cpp src/interpreter.cpp
OBJS_COMMON = $(SRCS_COMMON:.cpp=.o)

all: algo.exe run_tests.exe

algo.exe: $(OBJS_COMMON) src/main.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

run_tests.exe: $(OBJS_COMMON) src/test_runner.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	del /Q src\*.o *.exe

test: run_tests.exe
	./run_tests.exe
