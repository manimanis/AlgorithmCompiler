#!/usr/bin/env bash
set -e

echo "=============================================================================="
echo " Compilation de AlgorithmCompiler vers WebAssembly (Wasm) avec Emscripten"
echo "=============================================================================="

if ! command -v em++ &> /dev/null; then
    echo "[ERREUR] em++ introuvable dans le PATH."
    echo "Veuillez installer et activer l'Emscripten SDK (emsdk) :"
    echo "  git clone https://github.com/emscripten-core/emsdk.git"
    echo "  cd emsdk && ./emsdk install latest && ./emsdk activate latest"
    echo "  source ./emsdk_env.sh"
    exit 1
fi

mkdir -p web

echo "Compilation de algo.wasm et web/algo.js..."
em++ -std=c++20 -O3 -Iinclude \
    src/lexer.cpp src/value.cpp src/parser.cpp src/interpreter.cpp \
    src/bytecode.cpp src/bytecode_compiler.cpp src/vm.cpp \
    src/semantic_analyzer.cpp src/signature_extractor.cpp \
    src/python_transpiler.cpp src/flowchart_generator.cpp src/wasm_api.cpp \
    -o web/algo.js \
    -s WASM=1 \
    -s "EXPORTED_FUNCTIONS=['_algo_run','_algo_check','_algo_to_python','_algo_flowchart','_algo_signatures','_algo_version','_malloc','_free']" \
    -s "EXPORTED_RUNTIME_METHODS=['ccall','cwrap','UTF8ToString','stringToUTF8']" \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s MODULARIZE=1 \
    -s EXPORT_NAME="createAlgoModule"

echo "[SUCCÈS] Module WebAssembly généré avec succès dans web/algo.js et web/algo.wasm !"
echo "Lancez un serveur local pour tester : npx serve web"
