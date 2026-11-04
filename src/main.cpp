#include "../include/lexer.hpp"
#include "../include/parser.hpp"
#include "../include/interpreter.hpp"
#include "../include/python_transpiler.hpp"
#include "../include/semantic_analyzer.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>

using namespace algo;

static std::string readFile(const std::string& path) {
#ifdef _WIN32
    std::ifstream file(std::filesystem::u8path(path), std::ios::binary);
#else
    std::ifstream file(path, std::ios::binary);
#endif
    if (!file) {
        throw std::runtime_error("Impossible d'ouvrir le fichier : " + path);
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

static void writeFile(const std::string& path, const std::string& content) {
#ifdef _WIN32
    std::ofstream file(std::filesystem::u8path(path), std::ios::binary);
#else
    std::ofstream file(path, std::ios::binary);
#endif
    if (!file) {
        throw std::runtime_error("Impossible d'écrire dans le fichier : " + path);
    }
    file << content;
}

int main(int argc, char* argv[]) {
    initConsoleUtf8();

    if (argc < 2) {
        std::cout << "Usage: algo <fichier.alg> [--to-python [out.py]] [--max-iter N] [--seed N]" << std::endl;
        return 1;
    }

    std::string filePath;
    InterpreterOptions options;
    bool toPython = false;
    std::string pyOutputPath;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--max-iter" && i + 1 < argc) {
            options.maxIterations = std::stoull(argv[++i]);
        } else if (arg == "--seed" && i + 1 < argc) {
            options.seed = (uint32_t)std::stoul(argv[++i]);
        } else if (arg == "--to-python" || arg == "--py") {
            toPython = true;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                pyOutputPath = argv[++i];
            }
        } else if (filePath.empty() && arg[0] != '-') {
            filePath = arg;
        }
    }

    if (filePath.empty()) {
        std::cerr << "Erreur: Aucun fichier source fourni." << std::endl;
        return 1;
    }

    try {
        std::string source = readFile(filePath);
        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        Parser parser(std::move(tokens));
        Program prog = parser.parse();

        SemanticAnalyzer analyzer(source);
        analyzer.analyze(prog);

        if (toPython) {
            PythonTranspiler transpiler(prog);
            std::string pyCode = transpiler.transpile();
            if (!pyOutputPath.empty()) {
                writeFile(pyOutputPath, pyCode);
            } else {
                std::cout << pyCode;
            }
            return 0;
        }

        Interpreter interpreter(options);
        interpreter.setOutputCallback([](const std::string& text) {
            std::cout << text;
        });

        interpreter.run(prog);
        return 0;
    } catch (const AlgoError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Erreur: " << e.what() << std::endl;
        return 1;
    }
}
