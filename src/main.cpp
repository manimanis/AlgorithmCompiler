#include "../include/lexer.hpp"
#include "../include/parser.hpp"
#include "../include/interpreter.hpp"
#include "../include/python_transpiler.hpp"
#include "../include/semantic_analyzer.hpp"
#include "../include/bytecode_compiler.hpp"
#include "../include/vm.hpp"
#include "../include/version.hpp"
#include "../include/signature_extractor.hpp"
#include "../include/flowchart_generator.hpp"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <filesystem>

using namespace algo;

static std::string readFile(const std::string& path) {
#ifdef _WIN32
    std::ifstream file(utf8Path(path), std::ios::binary);
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
    std::ofstream file(utf8Path(path), std::ios::binary);
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
              << "    " << progName << " <fichier.algb> [OPTIONS]\n"
              << "    " << progName << " [OPTIONS]\n\n"
              << "ARGUMENTS :\n"
              << "    <fichier.alg>                 Chemin du fichier source en pseudo-code algorithmique.\n"
              << "    <fichier.algb>                Fichier binaire de bytecode précompilé à exécuter directement.\n\n"
              << "OPTIONS PRINCIPALES :\n"
              << "    -h, --help                    Affiche ce message d'aide et quitte.\n"
              << "    -v, --version                 Affiche le numéro de version du compilateur.\n"
              << "    -b, --build-exe [out.exe]     Compile l'algorithme sous forme d'un programme exécutable\n"
              << "                                  autonome (.exe) indépendant, sans dépendance externe.\n"
              << "    -c, --compile-bc [out.algb]   Compile l'algorithme en fichier binaire de bytecode (.algb).\n"
              << "    -s, --signatures, --tdo [out] Renvoie la signature de tous les sous-programmes et leurs\n"
              << "                                  tables TDO/TDOL et TDNT respectifs (génère automatiquement\n"
              << "                                  le TDO/TDOL s'il n'existe pas dans le code source).\n"
              << "    --json                        Formate la sortie en JSON (utilisable avec --signatures).\n"
              << "    -f, --flowchart, --mermaid [out] Génère l'organigramme (algorigramme) au format Mermaid.\n"
              << "    --flowchart-html [out.html]   Génère une page HTML autonome interactive avec zoom & export.\n"
              << "    --leak-check                  Active la détection des fuites de mémoire pour les pointeurs.\n"
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
              << "    " << progName << " mon_programme.alg --flowchart organigramme.mmd\n"
              << "    " << progName << " mon_programme.alg --flowchart-html organigramme.html\n"
              << "    " << progName << " mon_programme.alg --leak-check\n"
              << "    " << progName << " mon_programme.alg --build-exe mon_programme.exe\n"
              << "    " << progName << " mon_programme.alg --to-python\n";
}

int main(int argc, char* argv[]) {
    initConsoleUtf8();

    // Mode autonome : si le binaire actuel intègre du bytecode concaténé (signature ALGBEXEC),
    // l'exécuter directement en ignorant l'analyseur de ligne de commande standard
    std::string selfExe = getSelfExecutablePath();
    if (isStandaloneExecutable(selfExe)) {
        return runStandaloneExecutable(selfExe, argc, argv);
    }

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
    bool buildExe = false;
    std::string exeOutputPath;
    bool compileBc = false;
    std::string bcOutputPath;
    bool checkOnly = false;
    bool useVM = true;
    bool dumpBytecode = false;
    bool compareModes = false;
    bool showTime = false;
    bool showSignatures = false;
    std::string signaturesOutputPath;
    bool outputJson = false;
    bool generateFlowchart = false;
    bool generateFlowchartHtml = false;
    std::string flowchartOutputPath;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printHelp(argv[0]);
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            printVersion();
            return 0;
        } else if (arg == "-s" || arg == "--signatures" || arg == "--tdo") {
            showSignatures = true;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                signaturesOutputPath = argv[++i];
            }
        } else if (arg == "--json") {
            outputJson = true;
        } else if (arg == "-f" || arg == "--flowchart" || arg == "--mermaid") {
            generateFlowchart = true;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                flowchartOutputPath = argv[++i];
            }
        } else if (arg == "--flowchart-html") {
            generateFlowchartHtml = true;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                flowchartOutputPath = argv[++i];
            }
        } else if (arg == "--leak-check") {
            options.leakCheck = true;
        } else if (arg == "-b" || arg == "--build-exe") {
            buildExe = true;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                exeOutputPath = argv[++i];
            }
        } else if (arg == "-c" || arg == "--compile-bc") {
            compileBc = true;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                bcOutputPath = argv[++i];
            }
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
            options.hasSeed = true;
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

    std::string source;
    try {
        // Exécution directe si le fichier fourni est un fichier de bytecode binaire (.algb)
        bool isBytecodeFile = (filePath.size() >= 5 && filePath.substr(filePath.size() - 5) == ".algb");
        if (isBytecodeFile) {
            std::ifstream is(filePath, std::ios::binary);
            if (!is) {
                std::cerr << "Erreur: Impossible d'ouvrir le fichier bytecode : " << filePath << std::endl;
                return 1;
            }
            BytecodeProgram bcProg = deserializeProgram(is);
            if (dumpBytecode) {
                bcProg.mainChunk.disassemble("Bytecode Principal: " + bcProg.name);
                for (const auto& kv : bcProg.subroutines) {
                    kv.second.chunk.disassemble("Sous-programme: " + kv.first);
                }
                return 0;
            }
            if (buildExe) {
                if (exeOutputPath.empty()) {
                    std::filesystem::path p(filePath);
#ifdef _WIN32
                    p.replace_extension(".exe");
#else
                    p.replace_extension("");
#endif
                    exeOutputPath = p.string();
                }
                std::string err;
                if (!buildStandaloneExecutable(bcProg, exeOutputPath, err)) {
                    std::cerr << "Erreur lors de la génération de l'exécutable autonome : " << err << std::endl;
                    return 1;
                }
                std::cout << "[OK] Exécutable autonome généré avec succès : " << exeOutputPath << std::endl;
                return 0;
            }
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

        source = readFile(filePath);

        InterpreterOptions fileOpts = InterpreterOptions::fromSource(source);
        if (!hasCliMaxIter) options.maxIterations = fileOpts.maxIterations;
        if (!hasCliTimeout) options.timeoutMs = fileOpts.timeoutMs;
        if (!hasCliSeed && fileOpts.hasSeed) {
            options.seed = fileOpts.seed;
            options.hasSeed = true;
        }

        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        Parser parser(std::move(tokens));
        parser.setCurrentFilePath(filePath);
        Program prog = parser.parse();

        if (showSignatures) {
            SignatureExtractor extractor(prog);
            auto sigResult = extractor.extract();
            std::string outputText = outputJson ? sigResult.formatJson() : sigResult.formatText();
            if (!signaturesOutputPath.empty()) {
                writeFile(signaturesOutputPath, outputText);
                std::cout << "[OK] Signatures et TDO/TDNT exportés vers : " << signaturesOutputPath << std::endl;
            } else {
                std::cout << outputText;
            }
            return 0;
        }

        if (generateFlowchart || generateFlowchartHtml) {
            FlowchartGenerator generator(prog);
            std::string fcOutput = generateFlowchartHtml ? generator.generateHtml() : generator.generateMermaid();
            if (!flowchartOutputPath.empty()) {
                writeFile(flowchartOutputPath, fcOutput);
                std::cout << "[OK] Organigramme exporté vers : " << flowchartOutputPath << std::endl;
            } else {
                std::cout << fcOutput;
            }
            return 0;
        }

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

        if (compileBc) {
            BytecodeCompiler compiler;
            BytecodeProgram bcProg = compiler.compile(prog);
            if (bcOutputPath.empty()) {
                std::filesystem::path p(filePath);
                p.replace_extension(".algb");
                bcOutputPath = p.string();
            }
            std::ofstream os(bcOutputPath, std::ios::binary);
            if (!os) {
                std::cerr << "Erreur: Impossible de créer le fichier bytecode : " << bcOutputPath << std::endl;
                return 1;
            }
            serializeProgram(os, bcProg);
            std::cout << "[OK] Bytecode binaire compilé avec succès : " << bcOutputPath << std::endl;
            return 0;
        }

        if (buildExe) {
            BytecodeCompiler compiler;
            BytecodeProgram bcProg = compiler.compile(prog);
            if (exeOutputPath.empty()) {
                std::filesystem::path p(filePath);
#ifdef _WIN32
                p.replace_extension(".exe");
#else
                p.replace_extension("");
#endif
                exeOutputPath = p.string();
            }
            std::string err;
            if (!buildStandaloneExecutable(bcProg, exeOutputPath, err)) {
                std::cerr << "Erreur lors de la génération de l'exécutable autonome : " << err << std::endl;
                return 1;
            }
            std::cout << "[OK] Exécutable autonome généré avec succès : " << exeOutputPath << std::endl;
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
        if (!source.empty() || !e.callStack.empty()) {
            std::cerr << e.formatStackTrace(source) << std::endl;
        } else {
            std::cerr << e.what() << std::endl;
        }
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Erreur: " << e.what() << std::endl;
        return 1;
    }
}
