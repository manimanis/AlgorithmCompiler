#include "../include/lexer.hpp"
#include "../include/parser.hpp"
#include "../include/interpreter.hpp"
#include "../include/semantic_analyzer.hpp"
#include "../include/version.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>

namespace fs = std::filesystem;
using namespace algo;

static std::string readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return "";
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

static std::vector<std::string> readLines(const std::string& path) {
    std::vector<std::string> lines;
    std::ifstream file(path);
    if (!file) return lines;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        lines.push_back(line);
    }
    return lines;
}

static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

static std::string normalizeOutput(const std::string& str) {
    std::istringstream iss(str);
    std::string line;
    std::string res;
    while (std::getline(iss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        // Trim trailing whitespace
        size_t last = line.find_last_not_of(" \t");
        if (last != std::string::npos) {
            line = line.substr(0, last + 1);
        } else {
            line.clear();
        }
        res += line + "\n";
    }
    // Trim trailing empty lines
    while (res.size() >= 2 && res[res.size() - 1] == '\n' && res[res.size() - 2] == '\n') {
        res.pop_back();
    }
    return res;
}

static bool containsIgnoreCase(const std::string& haystack, const std::string& needle) {
    std::string h = toLower(normalize(haystack));
    std::string n = toLower(normalize(needle));
    return h.find(n) != std::string::npos;
}

struct TestDirectives {
    uint64_t maxIterations = 5000000;
    uint64_t timeoutMs = 10000;
    uint32_t seed = 42;
};

static TestDirectives parseDirectives(const std::string& source) {
    TestDirectives d;
    std::istringstream iss(source);
    std::string line;
    while (std::getline(iss, line)) {
        std::string trimmed = trim(line);
        if (trimmed.rfind("//", 0) != 0) {
            // Stop at first non-comment line
            if (!trimmed.empty()) break;
            continue;
        }
        // Look for directives: // @max-iter: N
        size_t pos = trimmed.find("@max-iter:");
        if (pos != std::string::npos) {
            try {
                d.maxIterations = std::stoull(trim(trimmed.substr(pos + 10)));
            } catch (...) {}
        }
        pos = trimmed.find("@timeout:");
        if (pos != std::string::npos) {
            try {
                d.timeoutMs = std::stoull(trim(trimmed.substr(pos + 9)));
            } catch (...) {}
        }
        pos = trimmed.find("@seed:");
        if (pos != std::string::npos) {
            try {
                d.seed = (uint32_t)std::stoul(trim(trimmed.substr(pos + 6)));
            } catch (...) {}
        }
    }
    return d;
}

int main(int argc, char* argv[]) {
    initConsoleUtf8();

    std::string testRoot = "tests";
    if (argc > 1) {
        testRoot = argv[1];
    }

    std::cout << "========================================================\n";
    std::cout << "  Suite de tests AlgorithmCompiler v" << getVersionString() << " (C++20)\n";
    std::cout << "========================================================\n";
    std::cout << "Dossier cible : " << testRoot << "\n\n";

    if (!fs::exists(testRoot)) {
        std::cerr << "Dossier introuvable : " << testRoot << "\n";
        return 1;
    }

    std::vector<fs::path> testFiles;
    for (const auto& entry : fs::recursive_directory_iterator(testRoot)) {
        if (entry.is_regular_file() && entry.path().extension() == ".alg") {
            testFiles.push_back(entry.path());
        }
    }
    std::sort(testFiles.begin(), testFiles.end());

    if (testFiles.empty()) {
        std::cout << "Aucun fichier de test .alg trouvé dans " << testRoot << "\n";
        return 0;
    }

    int passed = 0;
    int failed = 0;
    auto tStart = std::chrono::steady_clock::now();

    for (const auto& path : testFiles) {
        std::string relPath = fs::relative(path, testRoot).generic_string();
        fs::path parent = path.parent_path();
        std::string stem = path.stem().string();

        fs::path inPath = parent / (stem + ".in");
        fs::path outPath = parent / (stem + ".out");
        fs::path errPath = parent / (stem + ".err");
        fs::path warnPath = parent / (stem + ".warn");

        bool hasInput = fs::exists(inPath);
        bool hasExpectedOutput = fs::exists(outPath);
        bool hasExpectedError = fs::exists(errPath);
        bool hasExpectedWarn = fs::exists(warnPath);

        std::string expectedError;
        if (hasExpectedError) {
            expectedError = trim(readFile(errPath.string()));
        }

        std::string expectedWarn;
        if (hasExpectedWarn) {
            expectedWarn = trim(readFile(warnPath.string()));
        }

        std::string expectedOutput;
        if (hasExpectedOutput) {
            expectedOutput = normalizeOutput(readFile(outPath.string()));
        }

        std::vector<std::string> inputs;
        if (hasInput) {
            inputs = readLines(inPath.string());
        }

        bool testPassed = false;
        std::string errorMsg;
        bool warnMatched = !hasExpectedWarn;

        try {
            std::string source = readFile(path.string());
            TestDirectives directives = parseDirectives(source);

            Lexer lexer(source);
            auto tokens = lexer.tokenize();

            Parser parser(std::move(tokens));
            Program prog = parser.parse();

            SemanticAnalyzer analyzer(source);
            auto diags = analyzer.analyze(prog);

            if (hasExpectedWarn) {
                for (const auto& d : diags) {
                    if (!d.isError && (expectedWarn.empty() || containsIgnoreCase(d.message, expectedWarn) || containsIgnoreCase(d.formatted, expectedWarn))) {
                        warnMatched = true;
                        break;
                    }
                }
            }

            InterpreterOptions opts;
            opts.maxIterations = directives.maxIterations;
            opts.timeoutMs = directives.timeoutMs;
            opts.seed = directives.seed;

            Interpreter interpreter(opts);
            for (const auto& inVal : inputs) {
                interpreter.addInput(inVal);
            }

            std::string actualOutputRaw;
            interpreter.setOutputCallback([&actualOutputRaw](const std::string& str) {
                actualOutputRaw += str;
            });

            interpreter.run(prog);

            if (hasExpectedError) {
                testPassed = false;
                errorMsg = "Exception attendue ('" + expectedError + "') mais aucune levée";
            } else if (!warnMatched) {
                testPassed = false;
                errorMsg = "Avertissement attendu ('" + expectedWarn + "') mais aucun n'a été émis";
            } else if (hasExpectedOutput) {
                std::string actualOutput = normalizeOutput(actualOutputRaw);
                if (actualOutput == expectedOutput) {
                    testPassed = true;
                } else {
                    testPassed = false;
                    errorMsg = "Différence de sortie:\n--- Attendu ---\n" + expectedOutput +
                               "\n--- Obtenu ---\n" + actualOutput;
                }
            } else {
                // No output or error expected, execution completed successfully
                testPassed = true;
            }
        } catch (const AlgoError& e) {
            if (hasExpectedError) {
                if (expectedError.empty() || containsIgnoreCase(e.what(), expectedError)) {
                    testPassed = true;
                } else {
                    testPassed = false;
                    errorMsg = "Message d'erreur inattendu. Attendu: '" + expectedError +
                               "', obtenu: '" + e.what() + "'";
                }
            } else {
                testPassed = false;
                errorMsg = std::string("Erreur Algorithme inattendue: ") + e.what();
            }
        } catch (const std::exception& e) {
            if (hasExpectedError) {
                if (expectedError.empty() || containsIgnoreCase(e.what(), expectedError)) {
                    testPassed = true;
                } else {
                    testPassed = false;
                    errorMsg = "Message d'exception inattendu: " + std::string(e.what());
                }
            } else {
                testPassed = false;
                errorMsg = std::string("Exception inattendue: ") + e.what();
            }
        }

        if (testPassed) {
            passed++;
            std::cout << " [PASS] " << relPath << "\n";
        } else {
            failed++;
            std::cout << " [FAIL] " << relPath << "\n";
            std::istringstream miss(errorMsg);
            std::string errLine;
            while (std::getline(miss, errLine)) {
                std::cout << "        " << errLine << "\n";
            }
        }
    }

    auto tEnd = std::chrono::steady_clock::now();
    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStart).count();

    std::cout << "\n========================================================\n";
    std::cout << "                  RÉSULTATS DES TESTS                   \n";
    std::cout << "========================================================\n";
    std::cout << " Total tests : " << testFiles.size() << "\n";
    std::cout << " Réussis     : " << passed << "\n";
    std::cout << " Échoués     : " << failed << "\n";
    std::cout << " Durée       : " << durationMs << " ms\n";
    std::cout << "========================================================\n";

    // Nettoyage automatique des fichiers temporaires créés par les tests
    static const std::vector<std::string> tempFiles = {
        "Archive_Globale.dat", "classement_compagnies.txt", "codes_acces.dat",
        "compagnies.dat", "donnees_ex1.txt", "Expéditeurs.txt", "Fidele.txt",
        "f_data1.tmp", "f_rec.tmp", "f_texte1.tmp", "f_texte2.tmp", "f_texte3.tmp",
        "f_nested_rec.dat", "f_nested_eleves.dat", "f_clients_tunis.dat",
        "Gagnants.txt", "NF.dat", "resultats_ex1.dat", "source.txt"
    };
    for (const auto& tf : tempFiles) {
        std::error_code ec;
#ifdef _WIN32
        std::filesystem::remove(std::filesystem::u8path(tf), ec);
#else
        std::filesystem::remove(std::filesystem::path(tf), ec);
#endif
    }

    return (failed == 0) ? 0 : 1;
}
