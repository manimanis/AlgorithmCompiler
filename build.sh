#!/usr/bin/env bash
set -e

CXX=${CXX:-g++}

echo "Compilation de l'interpréteur Algorithme en C++..."
$CXX -std=c++20 -O2 -Wall -Wextra -Iinclude \
    src/lexer.cpp src/value.cpp src/parser.cpp src/interpreter.cpp \
    src/bytecode.cpp src/bytecode_compiler.cpp src/vm.cpp \
    src/python_transpiler.cpp src/semantic_analyzer.cpp src/main.cpp \
    -o algo

echo "Compilation du test runner..."
$CXX -std=c++20 -O2 -Wall -Wextra -Iinclude \
    src/lexer.cpp src/value.cpp src/parser.cpp src/interpreter.cpp \
    src/bytecode.cpp src/bytecode_compiler.cpp src/vm.cpp \
    src/semantic_analyzer.cpp src/test_runner.cpp \
    -o run_tests

echo "Compilation terminée avec succès!"
