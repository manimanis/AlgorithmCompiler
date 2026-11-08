@echo off
setlocal

echo ==============================================================================
echo Compilation de AlgorithmCompiler vers WebAssembly (Wasm) avec Emscripten
echo ==============================================================================

:: Detection automatique de emsdk si em++ n'est pas directement dans le PATH
where em++ >nul 2>nul
if %ERRORLEVEL% EQU 0 goto :compile

if exist "C:\emsdk\emsdk_env.bat" (
    echo Activation automatique de Emscripten depuis C:\emsdk...
    call "C:\emsdk\emsdk_env.bat" >nul 2>nul
    goto :check_em
)

if exist "%USERPROFILE%\emsdk\emsdk_env.bat" (
    echo Activation automatique de Emscripten depuis %USERPROFILE%\emsdk...
    call "%USERPROFILE%\emsdk\emsdk_env.bat" >nul 2>nul
    goto :check_em
)

:check_em
where em++ >nul 2>nul
if %ERRORLEVEL% NEQ 0 goto :no_emscripten
goto :compile

:no_emscripten
echo [ERREUR] em++ introuvable dans le PATH.
echo Pour compiler vers WebAssembly, veuillez installer et activer l'Emscripten SDK:
echo   1. git clone https://github.com/emscripten-core/emsdk.git C:\emsdk
echo   2. cd /d C:\emsdk ^&^& emsdk.bat install latest ^&^& emsdk.bat activate latest
echo   3. emsdk_env.bat
echo.
echo Note : L'interface web/index.html fonctionne egalement directement dans un navigateur.
exit /b 1

:compile
if not exist "web" mkdir web

echo Compilation de algo.wasm et web/algo.js...
call em++ -std=c++20 -O3 -Iinclude ^
    src/lexer.cpp src/value.cpp src/parser.cpp src/interpreter.cpp ^
    src/bytecode.cpp src/bytecode_compiler.cpp src/vm.cpp ^
    src/semantic_analyzer.cpp src/signature_extractor.cpp ^
    src/python_transpiler.cpp src/flowchart_generator.cpp src/wasm_api.cpp ^
    -o web/algo.js ^
    -s WASM=1 ^
    -s "EXPORTED_FUNCTIONS=['_algo_run','_algo_check','_algo_to_python','_algo_flowchart','_algo_signatures','_algo_version','_malloc','_free']" ^
    -s "EXPORTED_RUNTIME_METHODS=['ccall','cwrap','UTF8ToString','stringToUTF8']" ^
    -s ALLOW_MEMORY_GROWTH=1 ^
    -s MODULARIZE=1 ^
    -s EXPORT_NAME="createAlgoModule"

if %ERRORLEVEL% NEQ 0 goto :build_failed
goto :build_success

:build_failed
echo [ERREUR] Echec de la compilation WebAssembly.
exit /b 1

:build_success
echo ==============================================================================
echo [SUCCES] Module WebAssembly genere dans web/algo.js et web/algo.wasm !
echo ==============================================================================
echo Vous pouvez tester l'application en lancant un serveur local :
echo   npx serve web   ou   python -m http.server -d web 8080
endlocal
