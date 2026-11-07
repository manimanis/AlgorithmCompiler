# ==============================================================================
# Makefile pour AlgorithmCompiler - Support de la compilation & cross-compilation
# ==============================================================================

# Détection de l'hôte
ifeq ($(OS),Windows_NT)
    HOST_OS := windows
else
    HOST_OS := $(shell uname -s 2>/dev/null | tr '[:upper:]' '[:lower:]')
    ifeq ($(HOST_OS),)
        HOST_OS := linux
    endif
endif

# Cible par défaut : identique à l'hôte sauf si spécifié
TARGET_OS ?= $(HOST_OS)

# Support du préfixe standard de cross-compilation :
# Exemple : make CROSS_COMPILE=x86_64-w64-mingw32-
ifneq ($(CROSS_COMPILE),)
    CXX := $(CROSS_COMPILE)g++
    ifneq ($(findstring mingw,$(CROSS_COMPILE))$(findstring w64,$(CROSS_COMPILE)),)
        TARGET_OS := windows
    endif
endif

# Configuration selon la cible (Windows ou Linux)
ifeq ($(TARGET_OS),windows)
    BIN_EXT := .exe
    STATIC_FLAGS ?= -static -static-libgcc -static-libstdc++
    ifneq ($(HOST_OS),windows)
        # Cross-compilation depuis Linux/macOS vers Windows
        CXX ?= x86_64-w64-mingw32-g++
    else
        # Compilation native sur Windows
        ifeq ($(origin CXX),default)
            ifneq ($(wildcard C:/MSYS2/ucrt64/bin/g++.exe),)
                MSYS_BIN := C:\MSYS2\ucrt64\bin
                CXX := C:/MSYS2/ucrt64/bin/g++.exe
            else ifneq ($(wildcard C:/msys64/ucrt64/bin/g++.exe),)
                MSYS_BIN := C:\msys64\ucrt64\bin
                CXX := C:/msys64/ucrt64/bin/g++.exe
            else
                CXX := g++
            endif
            ifneq ($(MSYS_BIN),)
                export PATH := $(MSYS_BIN);$(PATH)
            endif
        endif
    endif
else
    BIN_EXT :=
    STATIC_FLAGS ?=
    CXX ?= g++
endif

# Flags de compilation C++20
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -Iinclude
LDFLAGS ?= $(STATIC_FLAGS) -s

# Fichiers sources et objets
SRCS_COMMON = src/lexer.cpp src/value.cpp src/parser.cpp src/interpreter.cpp src/bytecode.cpp src/bytecode_compiler.cpp src/vm.cpp src/semantic_analyzer.cpp src/signature_extractor.cpp
SRCS_MAIN   = src/python_transpiler.cpp src/main.cpp
SRCS_TEST   = src/test_runner.cpp
SRCS_FUZZ   = src/fuzz_runner.cpp
SRCS_STUB   = src/stub_main.cpp

OBJS_COMMON = $(SRCS_COMMON:.cpp=.o)
OBJS_MAIN   = $(SRCS_MAIN:.cpp=.o)
OBJS_TEST   = $(SRCS_TEST:.cpp=.o)
OBJS_FUZZ   = $(SRCS_FUZZ:.cpp=.o)
OBJS_STUB   = $(SRCS_STUB:.cpp=.o)

TARGET      := algo$(BIN_EXT)
TEST_TARGET := run_tests$(BIN_EXT)
FUZZ_TARGET := fuzz_runner$(BIN_EXT)
STUB_TARGET := algo_stub$(BIN_EXT)

# Nettoyage multi-plateforme
ifeq ($(HOST_OS),windows)
    CLEAN_CMD := del /Q /F src\*.o *.exe algo run_tests fuzz_runner algo_stub 2>nul || exit 0
else
    CLEAN_CMD := rm -f src/*.o algo run_tests fuzz_runner algo_stub algo.exe run_tests.exe fuzz_runner.exe algo_stub.exe
endif

# ==============================================================================
# Règles de compilation
# ==============================================================================

all: $(TARGET) $(TEST_TARGET) $(FUZZ_TARGET) $(STUB_TARGET)

$(TARGET): $(OBJS_COMMON) $(OBJS_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(TEST_TARGET): $(OBJS_COMMON) $(OBJS_TEST)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(FUZZ_TARGET): $(OBJS_COMMON) $(OBJS_FUZZ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(STUB_TARGET): src/value.o src/bytecode.o src/vm.o $(OBJS_STUB)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Cibles explicites de cross-compilation
windows:
	$(MAKE) TARGET_OS=windows all

win64:
	$(MAKE) TARGET_OS=windows CROSS_COMPILE=x86_64-w64-mingw32- all

win32:
	$(MAKE) TARGET_OS=windows CROSS_COMPILE=i686-w64-mingw32- all

linux:
	$(MAKE) TARGET_OS=linux all

clean:
	$(CLEAN_CMD)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

fuzz: $(FUZZ_TARGET)
	./$(FUZZ_TARGET) 1000

bench: $(TARGET)
	./$(TARGET) benchmarks/01_heavy_loop.alg --vm --bench
	./$(TARGET) benchmarks/02_fibonacci_deep.alg --vm --bench
	./$(TARGET) benchmarks/03_tri_selection.alg --vm --bench
	./$(TARGET) benchmarks/05_crible_premiers.alg --vm --bench

help:
	@echo ==============================================================================
	@echo  Options de compilation et cross-compilation pour AlgorithmCompiler
	@echo ==============================================================================
	@echo  make               : Compile pour la plateforme locale
	@echo  make windows       : Compile ou cross-compile pour Windows (.exe statique)
	@echo  make win64         : Cross-compile Windows 64-bit avec x86_64-w64-mingw32-g++
	@echo  make win32         : Cross-compile Windows 32-bit avec i686-w64-mingw32-g++
	@echo  make linux         : Compile pour Linux (binaire natif ELF)
	@echo  make test          : Compile et execute la suite de 185 tests unitaires
	@echo  make clean         : Nettoie les fichiers objets (.o) et executables
	@echo ==============================================================================

.PHONY: all windows win64 win32 linux clean test help
