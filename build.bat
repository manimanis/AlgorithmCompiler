@echo off
setlocal

:: Détection et configuration du compilateur g++
if exist "C:\msys64\ucrt64\bin\g++.exe" (
    set "PATH=C:\msys64\ucrt64\bin;%PATH%"
    set CXX=C:\msys64\ucrt64\bin\g++.exe
) else if exist "C:\msys64\mingw64\bin\g++.exe" (
    set "PATH=C:\msys64\mingw64\bin;%PATH%"
    set CXX=C:\msys64\mingw64\bin\g++.exe
) else (
    set CXX=g++
)

echo Compilation de l'interpreteur Algorithme en C++ avec %CXX%...
%CXX% -std=c++20 -O2 -static -Iinclude src/lexer.cpp src/value.cpp src/parser.cpp src/interpreter.cpp src/python_transpiler.cpp src/semantic_analyzer.cpp src/main.cpp -o algo.exe
if %ERRORLEVEL% NEQ 0 (
    echo Erreur de compilation de algo.exe
    exit /b 1
)

echo Compilation du test runner...
%CXX% -std=c++20 -O2 -static -Iinclude src/lexer.cpp src/value.cpp src/parser.cpp src/interpreter.cpp src/semantic_analyzer.cpp src/test_runner.cpp -o run_tests.exe
if %ERRORLEVEL% NEQ 0 (
    echo Erreur de compilation de run_tests.exe
    exit /b 1
)

echo Compilation terminee avec succes!
endlocal
