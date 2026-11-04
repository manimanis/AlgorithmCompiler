#include "../include/lexer.hpp"
#include "../include/parser.hpp"
#include "../include/interpreter.hpp"
#include "../include/python_transpiler.hpp"
#include "../include/semantic_analyzer.hpp"
#include "../include/version.hpp"
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
              << " AlgorithmCompiler v" << getVersionString() << " - Interpréteur, Analyseur Sémantique & Transpilateur\n"
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
    bool toPython = false;
    std::string pyOutputPath;
    bool checkOnly = false;

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
        } else if (arg == "--max-iter" && i + 1 < argc) {
            options.maxIterations = std::stoull(argv[++i]);
        } else if ((arg == "--timeout" || arg == "--timeout-ms") && i + 1 < argc) {
            options.timeoutMs = std::stoull(argv[++i]);
        } else if (arg == "--seed" && i + 1 < argc) {
            options.seed = (uint32_t)std::stoul(argv[++i]);
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
        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        Parser parser(std::move(tokens));
        Program prog = parser.parse();

        SemanticAnalyzer analyzer(source);
        analyzer.analyze(prog);

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
