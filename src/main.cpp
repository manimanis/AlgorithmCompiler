#include "../include/lexer.hpp"
#include "../include/parser.hpp"
#include "../include/interpreter.hpp"
#include "../include/python_transpiler.hpp"
#include "../include/semantic_analyzer.hpp"
#include "../include/bytecode_compiler.hpp"
#include "../include/vm.hpp"
#include "../include/version.hpp"
#include <fstream>
#include <iostream>
#include <iomanip>
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

static std::string getBaseName(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos) {
        return path.substr(pos + 1);
    }
    return path;
}

static void printVersion() {
    std::cout << getFullVersionInfo();
}

static void printHelp(const std::string& progPath) {
    std::string progName = getBaseName(progPath);
    if (progName.empty()) {
        progName = "algo";
    }

    std::cout << "==============================================================================\n"
              << " AlgorithmCompiler v" << getVersionString() << " - Interpréteur, VM Bytecode & Transpilateur\n"
              << " Standard algorithmique de l'enseignement secondaire & supérieur (Bac Tunisie)\n"
              << "==============================================================================\n\n"
              << "UTILISATION :\n"
              << "    " << progName << " <fichier.alg> [OPTIONS]\n"
              << "    " << progName << " [OPTIONS]\n\n"
              << "ARGUMENTS :\n"
              << "    <fichier.alg>                 Chemin du fichier source en pseudo-code algorithmique.\n\n"
              << "OPTIONS PRINCIPALES :\n"
              << "    -h, --help                    Affiche ce message d'aide et quitte.\n"
              << "    -v, --version                 Affiche le numéro de version du compilateur.\n"
              << "    --check                       Effectue uniquement l'analyse syntaxique et sémantique\n"
              << "                                  (vérification des types, portées, arités, etc.)\n"
              << "                                  sans exécuter l'algorithme.\n"
              << "    --vm                          Exécute l'algorithme via la Machine Virtuelle à Bytecode\n"
              << "                                  haute performance (mode par défaut).\n"
              << "    --ast                         Force l'exécution via l'Interpréteur d'AST (Patron Visiteur).\n"
              << "    --compare                     Compare directement la vitesse d'exécution entre AST et VM.\n"
              << "    --time, --bench               Mesure et affiche le temps d'exécution précis en ms.\n"
              << "    --dump-bytecode               Affiche le désassemblage du bytecode généré sans exécuter.\n"
              << "    --to-python, --py [out.py]    Transpile l'algorithme vers du code Python 3.\n"
              << "                                  Si [out.py] est fourni, écrit le code dans ce fichier ;\n"
              << "                                  sinon, affiche le résultat sur la sortie standard.\n\n"
              << "OPTIONS D'EXÉCUTION :\n"
              << "    --max-iter <N>                Nombre maximal d'itérations autorisées dans les boucles\n"
              << "                                  (défaut : 1 000 000) pour prévenir les boucles infinies.\n"
              << "    --timeout <ms>                Délai d'exécution maximal en millisecondes\n"
              << "                                  (défaut : 50 000 ms).\n"
              << "    --seed <N>                    Graine initiale (seed) pour le générateur pseudo-aléatoire\n"
              << "                                  (utilisé par les fonctions Aléa / Hasard).\n\n"
              << "EXEMPLES :\n"
              << "    " << progName << " mon_programme.alg\n"
              << "    " << progName << " mon_programme.alg --vm\n"
              << "    " << progName << " mon_programme.alg --dump-bytecode\n"
              << "    " << progName << " mon_programme.alg --check\n"
              << "    " << progName << " mon_programme.alg --to-python\n"
              << "    " << progName << " mon_programme.alg --to-python sortie.py\n"
              << "    " << progName << " mon_programme.alg --seed 42 --max-iter 2000000\n";
}

int main(int argc, char* argv[]) {
    initConsoleUtf8();

    if (argc < 2) {
        std::cout << "Usage: algo <fichier.alg> [OPTIONS]\n"
                  << "Tapez 'algo --help' pour afficher l'aide détaillée." << std::endl;
        return 1;
    }

    std::string filePath;
    InterpreterOptions options;
    bool hasCliMaxIter = false;
    bool hasCliTimeout = false;
    bool hasCliSeed = false;
    bool toPython = false;
    std::string pyOutputPath;
    bool checkOnly = false;
    bool useVM = true;
    bool dumpBytecode = false;
    bool compareModes = false;
    bool showTime = false;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printHelp(argv[0]);
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            printVersion();
            return 0;
        } else if (arg == "--check") {
            checkOnly = true;
        } else if (arg == "--vm") {
            useVM = true;
        } else if (arg == "--ast") {
            useVM = false;
        } else if (arg == "--compare") {
            compareModes = true;
        } else if (arg == "--time" || arg == "--bench") {
            showTime = true;
        } else if (arg == "--dump-bytecode") {
            dumpBytecode = true;
        } else if (arg == "--max-iter" && i + 1 < argc) {
            options.maxIterations = std::stoull(argv[++i]);
            hasCliMaxIter = true;
        } else if ((arg == "--timeout" || arg == "--timeout-ms") && i + 1 < argc) {
            options.timeoutMs = std::stoull(argv[++i]);
            hasCliTimeout = true;
        } else if (arg == "--seed" && i + 1 < argc) {
            options.seed = (uint32_t)std::stoul(argv[++i]);
            hasCliSeed = true;
        } else if (arg == "--to-python" || arg == "--py") {
            toPython = true;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                pyOutputPath = argv[++i];
            }
        } else if (filePath.empty() && arg[0] != '-') {
            filePath = arg;
        } else {
            std::cerr << "Erreur: Option ou argument non reconnu : " << arg << std::endl;
            std::cerr << "Tapez 'algo --help' pour afficher l'aide." << std::endl;
            return 1;
        }
    }

    if (filePath.empty()) {
        std::cerr << "Erreur: Aucun fichier source fourni." << std::endl;
        std::cerr << "Tapez 'algo --help' pour afficher l'aide." << std::endl;
        return 1;
    }

    try {
        std::string source = readFile(filePath);

        InterpreterOptions fileOpts = InterpreterOptions::fromSource(source);
        if (!hasCliMaxIter) options.maxIterations = fileOpts.maxIterations;
        if (!hasCliTimeout) options.timeoutMs = fileOpts.timeoutMs;
        if (!hasCliSeed) options.seed = fileOpts.seed;

        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        Parser parser(std::move(tokens));
        parser.setCurrentFilePath(filePath);
        Program prog = parser.parse();

        SemanticAnalyzer analyzer(source);
        auto diags = analyzer.analyze(prog);

        for (const auto& d : diags) {
            if (!d.isError) {
                std::cerr << d.formatted << "\n";
            }
        }

        if (checkOnly) {
            std::cout << "[OK] Syntaxe et analyse sémantique réussies pour : " << filePath << std::endl;
            return 0;
        }

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

        if (dumpBytecode) {
            BytecodeCompiler compiler;
            BytecodeProgram bcProg = compiler.compile(prog);
            bcProg.mainChunk.disassemble("Bytecode Principal: " + prog.name);
            for (const auto& kv : bcProg.subroutines) {
                kv.second.chunk.disassemble("Sous-programme: " + kv.first);
            }
            return 0;
        }

        if (compareModes) {
            std::string astOutput;
            Interpreter interpreter(options);
            interpreter.setOutputCallback([&astOutput](const std::string& text) {
                astOutput += text;
            });
            auto t0_ast = std::chrono::high_resolution_clock::now();
            interpreter.run(prog);
            auto t1_ast = std::chrono::high_resolution_clock::now();
            double astTimeMs = std::chrono::duration<double, std::milli>(t1_ast - t0_ast).count();

            std::string vmOutput;
            BytecodeCompiler compiler;
            BytecodeProgram bcProg = compiler.compile(prog);
            VM vm(options);
            vm.setOutputCallback([&vmOutput](const std::string& text) {
                vmOutput += text;
            });
            auto t0_vm = std::chrono::high_resolution_clock::now();
            vm.run(bcProg);
            auto t1_vm = std::chrono::high_resolution_clock::now();
            double vmTimeMs = std::chrono::duration<double, std::milli>(t1_vm - t0_vm).count();

            std::cout << "==============================================================================\n"
                      << "              COMPARAISON DES PERFORMANCES : MODE AST vs MODE VM              \n"
                      << "==============================================================================\n"
                      << " Programme : " << filePath << "\n\n"
                      << " Sortie standard identique : " << (astOutput == vmOutput ? "OUI [100% CONFORME]" : "DIFFERENTE") << "\n\n"
                      << "  - Mode AST (Tree-walking Visiteur) : " << std::fixed << std::setprecision(3) << astTimeMs << " ms\n"
                      << "  - Mode VM  (Machine Virtuelle)     : " << std::fixed << std::setprecision(3) << vmTimeMs << " ms\n";
            if (vmTimeMs > 0.0001) {
                double speedup = astTimeMs / vmTimeMs;
                std::cout << "\n -> Gain de vitesse (Speedup)        : " << std::fixed << std::setprecision(2) << speedup << "x plus rapide avec la VM !\n";
            }
            std::cout << "==============================================================================\n";
            return 0;
        }

        if (useVM) {
            BytecodeCompiler compiler;
            BytecodeProgram bcProg = compiler.compile(prog);
            VM vm(options);
            vm.setOutputCallback([](const std::string& text) {
                std::cout << text;
            });
            auto t0 = std::chrono::high_resolution_clock::now();
            vm.run(bcProg);
            auto t1 = std::chrono::high_resolution_clock::now();
            if (showTime) {
                double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
                std::cout << "\n[VM] Temps d'exécution : " << std::fixed << std::setprecision(3) << ms << " ms\n";
            }
            return 0;
        }

        Interpreter interpreter(options);
        interpreter.setOutputCallback([](const std::string& text) {
            std::cout << text;
        });

        auto t0 = std::chrono::high_resolution_clock::now();
        interpreter.run(prog);
        auto t1 = std::chrono::high_resolution_clock::now();
        if (showTime) {
            double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
            std::cout << "\n[AST] Temps d'exécution : " << std::fixed << std::setprecision(3) << ms << " ms\n";
        }
        return 0;
    } catch (const AlgoError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Erreur: " << e.what() << std::endl;
        return 1;
    }
}
