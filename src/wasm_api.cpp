#include "../include/lexer.hpp"
#include "../include/parser.hpp"
#include "../include/interpreter.hpp"
#include "../include/bytecode_compiler.hpp"
#include "../include/vm.hpp"
#include "../include/semantic_analyzer.hpp"
#include "../include/python_transpiler.hpp"
#include "../include/signature_extractor.hpp"
#include "../include/flowchart_generator.hpp"
#include "../include/version.hpp"
#include <string>
#include <sstream>
#include <vector>
#include <cstring>

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#define ALGO_WASM_EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define ALGO_WASM_EXPORT
#endif

namespace algo {

static std::string g_wasmResultBuffer;

static const char* exportString(const std::string& str) {
    g_wasmResultBuffer = str;
    return g_wasmResultBuffer.c_str();
}

} // namespace algo

extern "C" {

ALGO_WASM_EXPORT
const char* algo_version() {
    return algo::exportString(algo::getFullVersionInfo());
}

ALGO_WASM_EXPORT
const char* algo_run(const char* sourceCode, const char* stdinInput, int maxIter, int timeoutMs) {
    if (!sourceCode) return algo::exportString("Erreur: Code source vide.");

    try {
        std::string source(sourceCode);
        algo::InterpreterOptions opts;
        if (maxIter > 0) opts.maxIterations = static_cast<uint64_t>(maxIter);
        if (timeoutMs > 0) opts.timeoutMs = static_cast<uint64_t>(timeoutMs);
        opts.leakCheck = true; // Activer la détection des fuites de mémoire par défaut sur le Web

        algo::InterpreterOptions fileOpts = algo::InterpreterOptions::fromSource(source, opts);

        algo::Lexer lexer(source);
        auto tokens = lexer.tokenize();

        algo::Parser parser(std::move(tokens));
        algo::Program prog = parser.parse();

        algo::BytecodeCompiler compiler;
        algo::BytecodeProgram bcProg = compiler.compile(prog);

        std::string outputResult;
        algo::VM vm(fileOpts);
        vm.setOutputCallback([&outputResult](const std::string& text) {
            outputResult += text;
        });

        if (stdinInput && stdinInput[0] != '\0') {
            std::istringstream iss(stdinInput);
            std::string line;
            while (std::getline(iss, line)) {
                if (!line.empty() && line.back() == '\r') line.pop_back();
                vm.addInput(line);
            }
        }

        vm.run(bcProg);
        return algo::exportString(outputResult);

    } catch (const algo::AlgoError& e) {
        return algo::exportString(e.formatStackTrace(sourceCode));
    } catch (const std::exception& e) {
        return algo::exportString(std::string("Erreur: ") + e.what());
    }
}

ALGO_WASM_EXPORT
const char* algo_check(const char* sourceCode) {
    if (!sourceCode) return algo::exportString("Erreur: Code source vide.");

    try {
        std::string source(sourceCode);
        algo::Lexer lexer(source);
        auto tokens = lexer.tokenize();

        algo::Parser parser(std::move(tokens));
        algo::Program prog = parser.parse();

        algo::SemanticAnalyzer analyzer(source);
        auto diags = analyzer.analyze(prog);

        if (diags.empty()) {
            return algo::exportString("{\"status\":\"success\",\"message\":\"[OK] Aucune erreur syntaxique ou sémantique détectée !\"}");
        }

        std::ostringstream oss;
        oss << "{\n  \"status\": \"diagnostics\",\n  \"diagnostics\": [\n";
        for (size_t i = 0; i < diags.size(); ++i) {
            const auto& d = diags[i];
            oss << "    {\n"
                << "      \"isError\": " << (d.isError ? "true" : "false") << ",\n"
                << "      \"line\": " << d.line << ",\n"
                << "      \"column\": " << d.column << ",\n"
                << "      \"object\": \"" << d.objectName << "\",\n"
                << "      \"message\": \"" << d.message << "\",\n"
                << "      \"formatted\": \"" << d.formatted << "\"\n"
                << "    }" << (i + 1 < diags.size() ? ",\n" : "\n");
        }
        oss << "  ]\n}";
        return algo::exportString(oss.str());

    } catch (const algo::AlgoError& e) {
        return algo::exportString(std::string("{\"status\":\"error\",\"message\":\"") + e.what() + "\"}");
    } catch (const std::exception& e) {
        return algo::exportString(std::string("{\"status\":\"error\",\"message\":\"") + e.what() + "\"}");
    }
}

ALGO_WASM_EXPORT
const char* algo_to_python(const char* sourceCode) {
    if (!sourceCode) return algo::exportString("");

    try {
        std::string source(sourceCode);
        algo::Lexer lexer(source);
        auto tokens = lexer.tokenize();

        algo::Parser parser(std::move(tokens));
        algo::Program prog = parser.parse();

        algo::PythonTranspiler transpiler(prog);
        return algo::exportString(transpiler.transpile());

    } catch (const std::exception& e) {
        return algo::exportString(std::string("# Erreur lors de la transpilation Python: ") + e.what());
    }
}

ALGO_WASM_EXPORT
const char* algo_flowchart(const char* sourceCode, int format) {
    if (!sourceCode) return algo::exportString("");

    try {
        std::string source(sourceCode);
        algo::Lexer lexer(source);
        auto tokens = lexer.tokenize();

        algo::Parser parser(std::move(tokens));
        algo::Program prog = parser.parse();

        algo::FlowchartFormat fmt = algo::FlowchartFormat::Mermaid;
        if (format == 1) fmt = algo::FlowchartFormat::Markdown;
        else if (format == 2) fmt = algo::FlowchartFormat::Html;

        algo::FlowchartGenerator gen(prog);
        return algo::exportString(gen.generate(fmt));

    } catch (const std::exception& e) {
        return algo::exportString(std::string("flowchart TD\n    err[\"Erreur lors de la génération: ") + e.what() + "\"]");
    }
}

ALGO_WASM_EXPORT
const char* algo_signatures(const char* sourceCode, int asJson) {
    if (!sourceCode) return algo::exportString("");

    try {
        std::string source(sourceCode);
        auto sigResult = algo::SignatureExtractor::extractFromSource(source);
        return algo::exportString(asJson ? sigResult.formatJson() : sigResult.formatText());

    } catch (const std::exception& e) {
        return algo::exportString(std::string("Erreur d'extraction des signatures : ") + e.what());
    }
}

} // extern "C"
