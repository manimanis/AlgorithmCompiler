@echo off
setlocal

:: Detection et configuration du compilateur g++
if exist "C:\MSYS2\ucrt64\bin\g++.exe" (
    set "PATH=C:\MSYS2\ucrt64\bin;%PATH%"
    set CXX=C:\MSYS2\ucrt64\bin\g++.exe
) else if exist "C:\msys64\ucrt64\bin\g++.exe" (
    set "PATH=C:\msys64\ucrt64\bin;%PATH%"
    set CXX=C:\msys64\ucrt64\bin\g++.exe
) else if exist "C:\msys64\mingw64\bin\g++.exe" (
    set "PATH=C:\msys64\mingw64\bin;%PATH%"
    set CXX=C:\msys64\mingw64\bin\g++.exe
) else (
    set CXX=g++
)

echo Compilation de l'interpreteur Algorithme en C++ avec %CXX%...
%CXX% -std=c++20 -O2 -s -static -Iinclude src/lexer.cpp src/value.cpp src/parser.cpp src/interpreter.cpp src/bytecode.cpp src/bytecode_compiler.cpp src/vm.cpp src/python_transpiler.cpp src/semantic_analyzer.cpp src/signature_extractor.cpp src/flowchart_generator.cpp src/main.cpp -o algo.exe
if %ERRORLEVEL% NEQ 0 (
    echo Erreur de compilation de algo.exe
    exit /b 1
)

echo Compilation du test runner...
%CXX% -std=c++20 -O2 -s -static -Iinclude src/lexer.cpp src/value.cpp src/parser.cpp src/interpreter.cpp src/bytecode.cpp src/bytecode_compiler.cpp src/vm.cpp src/semantic_analyzer.cpp src/signature_extractor.cpp src/flowchart_generator.cpp src/test_runner.cpp -o run_tests.exe
if %ERRORLEVEL% NEQ 0 (
    echo Erreur de compilation de run_tests.exe
    exit /b 1
)

echo Compilation du fuzz runner...
%CXX% -std=c++20 -O2 -s -static -Iinclude src/lexer.cpp src/value.cpp src/parser.cpp src/interpreter.cpp src/bytecode.cpp src/bytecode_compiler.cpp src/vm.cpp src/semantic_analyzer.cpp src/signature_extractor.cpp src/flowchart_generator.cpp src/fuzz_runner.cpp -o fuzz_runner.exe
if %ERRORLEVEL% NEQ 0 (
    echo Erreur de compilation de fuzz_runner.exe
    exit /b 1
)

echo Compilation du runner stub VM autonome...
%CXX% -std=c++20 -O2 -s -static -Iinclude src/value.cpp src/bytecode.cpp src/vm.cpp src/stub_main.cpp -o algo_stub.exe
if %ERRORLEVEL% NEQ 0 (
    echo Erreur de compilation de algo_stub.exe
    exit /b 1
)

echo Compilation terminee avec succes!
endlocal
