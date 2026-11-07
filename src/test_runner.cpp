#include "../include/lexer.hpp"
#include "../include/parser.hpp"
#include "../include/interpreter.hpp"
#include "../include/bytecode_compiler.hpp"
#include "../include/vm.hpp"
#include "../include/semantic_analyzer.hpp"
#include "../include/version.hpp"
#include "../include/signature_extractor.hpp"
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
    InterpreterOptions base;
    base.maxIterations = 5000000;
    base.timeoutMs = 10000;
    base.seed = 42;
    InterpreterOptions opts = InterpreterOptions::fromSource(source, base);
    TestDirectives d;
    d.maxIterations = opts.maxIterations;
    d.timeoutMs = opts.timeoutMs;
    d.seed = opts.seed;
    return d;
}

static void runSignatureExtractorUnitTests(int& passed, int& failed, int& totalUnitTests) {
    std::cout << "\n--- Tests unitaires : Extraction des Signatures & TDO / TDNT ---\n";

    // Test 1: Signatures avec TDOL explicite
    {
        totalUnitTests++;
        try {
            std::string src = 
                "Procédure Saisir(@n: entier)\n"
                "Début\n"
                "    n ← 5\n"
                "Fin\n"
                "Fonction Est_Premier(n: entier): booléen\n"
                "Début\n"
                "    premier ← Vrai\n"
                "    Pour d de 2 à n - 1 Faire\n"
                "        Si n mod d = 0 Alors premier ← Faux Fin Si\n"
                "    Fin Pour\n"
                "    Retourner premier\n"
                "Fin\n"
                "TDOL\n"
                "Objet : Type/Nature\n"
                "d : entier\n"
                "premier : booléen\n"
                "Algorithme TestA\n"
                "Début\n"
                "    Saisir(n)\n"
                "Fin\n"
                "TDO\n"
                "Objet : Type/Nature\n"
                "n : entier\n";

            auto res = SignatureExtractor::extractFromSource(src);
            bool ok = true;
            if (res.subprograms.size() != 2) ok = false;
            if (ok && res.subprograms[0].signature.formatSignature() != "Procédure Saisir(@n: entier)") ok = false;
            if (ok && res.subprograms[1].signature.formatSignature() != "Fonction Est_Premier(n: entier): booléen") ok = false;
            if (ok && res.subprograms[1].isTdolGenerated) ok = false;
            if (ok && res.subprograms[1].tdol.size() != 2) ok = false;

            if (ok) {
                std::cout << " [PASS] unit/signatures_explicit_tdol\n";
                passed++;
            } else {
                std::cout << " [FAIL] unit/signatures_explicit_tdol\n";
                failed++;
            }
        } catch (const std::exception& e) {
            std::cout << " [FAIL] unit/signatures_explicit_tdol (" << e.what() << ")\n";
            failed++;
        }
    }

    // Test 2: Inférence et génération automatique de TDOL / TDO
    {
        totalUnitTests++;
        try {
            std::string src = 
                "Fonction Calcul(n: entier): réel\n"
                "Début\n"
                "    total ← 0.0\n"
                "    Pour idx de 1 à n Faire\n"
                "        val ← idx * 2.5\n"
                "        total ← total + val\n"
                "    Fin Pour\n"
                "    msg ← \"Termine\"\n"
                "    ok ← Vrai\n"
                "    Retourner total\n"
                "Fin\n"
                "Algorithme TestB\n"
                "Début\n"
                "    res ← Calcul(10)\n"
                "Fin\n";

            auto res = SignatureExtractor::extractFromSource(src);
            bool ok = true;
            if (res.subprograms.size() != 1) ok = false;
            if (ok && !res.subprograms[0].isTdolGenerated) ok = false;
            if (ok && !res.isMainTdoGenerated) ok = false;

            auto hasVar = [](const std::vector<TdoEntry>& entries, const std::string& varName, const std::string& expectedType) {
                for (const auto& e : entries) {
                    for (const auto& n : e.names) {
                        if (normalize(n) == normalize(varName)) {
                            return normalize(e.typeOrNature) == normalize(expectedType);
                        }
                    }
                }
                return false;
            };

            if (ok && !hasVar(res.subprograms[0].tdol, "idx", "entier")) ok = false;
            if (ok && !hasVar(res.subprograms[0].tdol, "total", "réel")) ok = false;
            if (ok && !hasVar(res.subprograms[0].tdol, "msg", "chaîne")) ok = false;
            if (ok && !hasVar(res.subprograms[0].tdol, "ok", "booléen")) ok = false;
            if (ok && !hasVar(res.mainTdo, "res", "réel")) ok = false;

            if (ok) {
                std::cout << " [PASS] unit/signatures_tdol_auto_generation\n";
                passed++;
            } else {
                std::cout << " [FAIL] unit/signatures_tdol_auto_generation\n";
                failed++;
            }
        } catch (const std::exception& e) {
            std::cout << " [FAIL] unit/signatures_tdol_auto_generation (" << e.what() << ")\n";
            failed++;
        }
    }

    // Test 3: Filtrage TDNT respectif
    {
        totalUnitTests++;
        try {
            std::string src = 
                "Procédure TraiterA(a: TypeA)\n"
                "Début\n"
                "Fin\n"
                "Procédure TraiterB(b: TypeB)\n"
                "Début\n"
                "Fin\n"
                "Algorithme TestC\n"
                "Début\n"
                "Fin\n"
                "TDNT\n"
                "TypeA = Tableau de 5 entier\n"
                "TypeB = Enregistrement x: réel Fin\n"
                "TypeC = Tableau de 10 chaîne\n";

            auto res = SignatureExtractor::extractFromSource(src);
            bool ok = true;
            if (res.subprograms.size() != 2) ok = false;
            if (ok && (res.subprograms[0].respectiveTdnt.size() != 1 || res.subprograms[0].respectiveTdnt[0].name != "TypeA")) ok = false;
            if (ok && (res.subprograms[1].respectiveTdnt.size() != 1 || res.subprograms[1].respectiveTdnt[0].name != "TypeB")) ok = false;
            if (ok && res.globalTdnt.size() != 3) ok = false;

            if (ok) {
                std::cout << " [PASS] unit/signatures_respective_tdnt_filtering\n";
                passed++;
            } else {
                std::cout << " [FAIL] unit/signatures_respective_tdnt_filtering\n";
                failed++;
            }
        } catch (const std::exception& e) {
            std::cout << " [FAIL] unit/signatures_respective_tdnt_filtering (" << e.what() << ")\n";
            failed++;
        }
    }

    // Test 4: Formatage texte et JSON
    {
        totalUnitTests++;
        try {
            std::string src = 
                "Fonction Doubler(x: entier): entier\n"
                "Début\n"
                "    res ← x * 2\n"
                "    Retourner res\n"
                "Fin\n"
                "Algorithme TestD\n"
                "Début\n"
                "Fin\n";

            auto res = SignatureExtractor::extractFromSource(src);
            std::string txt = res.formatText();
            std::string json = res.formatJson();
            bool ok = true;
            if (txt.find("Fonction Doubler(x: entier): entier") == std::string::npos) ok = false;
            if (txt.find("TDOL (généré automatiquement)") == std::string::npos) ok = false;
            if (json.find("\"kind\": \"function\"") == std::string::npos) ok = false;
            if (json.find("\"is_tdol_generated\": true") == std::string::npos) ok = false;

            if (ok) {
                std::cout << " [PASS] unit/signatures_formatting_text_and_json\n";
                passed++;
            } else {
                std::cout << " [FAIL] unit/signatures_formatting_text_and_json\n";
                failed++;
            }
        } catch (const std::exception& e) {
            std::cout << " [FAIL] unit/signatures_formatting_text_and_json (" << e.what() << ")\n";
            failed++;
        }
    }

    // Test 5: Formatage TDNT matrice (Tableau de N lignes * M colonnes de <type>)
    {
        totalUnitTests++;
        try {
            std::string src = 
                "Procédure Remplir(m: MaMatrice)\n"
                "Début\n"
                "Fin\n"
                "Algorithme TestMatriceFormat\n"
                "Début\n"
                "Fin\n"
                "TDNT\n"
                "MaMatrice = Tableau de 3 lignes * 4 colonnes de entier\n";

            auto res = SignatureExtractor::extractFromSource(src);
            std::string txt = res.formatText();
            bool ok = true;
            if (txt.find("MaMatrice = Tableau de 3 lignes * 4 colonnes de entier") == std::string::npos) ok = false;

            if (ok) {
                std::cout << " [PASS] unit/signatures_matrix_format\n";
                passed++;
            } else {
                std::cout << " [FAIL] unit/signatures_matrix_format\n";
                failed++;
            }
        } catch (const std::exception& e) {
            std::cout << " [FAIL] unit/signatures_matrix_format (" << e.what() << ")\n";
            failed++;
        }
    }

    // Test 6: Inférence et découverte automatique du TDNT (Enregistrement et Tableau)
    {
        totalUnitTests++;
        try {
            std::string src = 
                "Procédure AfficherNotes(t: TabNotes, n: entier)\n"
                "Début\n"
                "    Pour i de 0 à n - 1 Faire\n"
                "        Ecrire(t[i])\n"
                "    Fin Pour\n"
                "Fin\n"
                "Fonction EstMajeur(e: Etudiant): booléen\n"
                "Début\n"
                "    Retourner e.age >= 18\n"
                "Fin\n"
                "Algorithme TestTdntInference\n"
                "Début\n"
                "    notes[0] ← 15.5\n"
                "    notes[1] ← 18.0\n"
                "    AfficherNotes(notes, 2)\n"
                "    e.age ← 20\n"
                "    Si EstMajeur(e) Alors\n"
                "        Ecrire(\"Admis\")\n"
                "    Fin Si\n"
                "Fin\n";

            auto res = SignatureExtractor::extractFromSource(src);
            bool ok = true;
            if (!res.isGlobalTdntGenerated) ok = false;
            if (res.globalTdnt.size() < 2) ok = false;

            bool foundEtudiant = false;
            bool foundTabNotes = false;
            for (const auto& td : res.globalTdnt) {
                if (td.name == "Etudiant" && td.typeSpec.kind == TypeKind::Record) {
                    if (!td.typeSpec.fields.empty() && td.typeSpec.fields[0]->names[0] == "age") {
                        foundEtudiant = true;
                    }
                }
                if (td.name == "TabNotes" && td.typeSpec.kind == TypeKind::Array1D) {
                    foundTabNotes = true;
                }
            }
            if (!foundEtudiant || !foundTabNotes) ok = false;

            // Vérifier que le TDO principal a bien typé e : Etudiant et notes : TabNotes
            bool foundTdoE = false;
            bool foundTdoNotes = false;
            for (const auto& entry : res.mainTdo) {
                for (const auto& n : entry.names) {
                    if (n == "e" && entry.typeOrNature == "Etudiant") foundTdoE = true;
                    if (n == "notes" && entry.typeOrNature == "TabNotes") foundTdoNotes = true;
                }
            }
            if (!foundTdoE || !foundTdoNotes) ok = false;

            if (ok) {
                std::cout << " [PASS] unit/signatures_inferred_tdnt_record_and_array\n";
                passed++;
            } else {
                std::cout << " [FAIL] unit/signatures_inferred_tdnt_record_and_array\n";
                failed++;
            }
        } catch (const std::exception& e) {
            std::cout << " [FAIL] unit/signatures_inferred_tdnt_record_and_array (" << e.what() << ")\n";
            failed++;
        }
    }
}

int main(int argc, char* argv[]) {
    initConsoleUtf8();

    bool useAstMode = false;
    std::string testRoot = "tests";
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--ast") {
            useAstMode = true;
        } else if (arg == "--vm") {
            useAstMode = false;
        } else if (arg[0] != '-') {
            testRoot = arg;
        }
    }

    std::cout << "========================================================\n";
    std::cout << "  Suite de tests AlgorithmCompiler v" << getVersionString() << " (C++20)\n";
    std::cout << "  Mode d'exécution : " << (useAstMode ? "Interpréteur AST" : "Machine Virtuelle Bytecode (VM)") << "\n";
    std::cout << "========================================================\n";
    std::cout << "Dossier cible : " << testRoot << "\n\n";

    if (!fs::exists(testRoot)) {
        std::cerr << "Dossier introuvable : " << testRoot << "\n";
        return 1;
    }

    std::vector<fs::path> testFiles;
    if (fs::is_regular_file(testRoot)) {
        if (fs::path(testRoot).extension() == ".alg") {
            testFiles.push_back(testRoot);
        }
    } else {
        for (const auto& entry : fs::recursive_directory_iterator(testRoot)) {
            if (entry.is_regular_file() && entry.path().extension() == ".alg") {
                testFiles.push_back(entry.path());
            }
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
        std::string relPath = fs::is_regular_file(testRoot) ? path.filename().string() : fs::relative(path, testRoot).generic_string();
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
            parser.setCurrentFilePath(path.string());
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

            std::string actualOutputRaw;

            if (useAstMode) {
                Interpreter interpreter(opts);
                for (const auto& inVal : inputs) {
                    interpreter.addInput(inVal);
                }
                interpreter.setOutputCallback([&actualOutputRaw](const std::string& str) {
                    actualOutputRaw += str;
                });
                interpreter.run(prog);
            } else {
                BytecodeCompiler compiler;
                BytecodeProgram bcProg = compiler.compile(prog);

                // Test de validation systématique de la sérialisation / désérialisation binaire
                std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
                serializeProgram(ss, bcProg);
                BytecodeProgram deserializedProg = deserializeProgram(ss);

                VM vm(opts);
                for (const auto& inVal : inputs) {
                    vm.addInput(inVal);
                }
                vm.setOutputCallback([&actualOutputRaw](const std::string& str) {
                    actualOutputRaw += str;
                });
                vm.run(deserializedProg);
            }

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

    int unitTestsCount = 0;
    runSignatureExtractorUnitTests(passed, failed, unitTestsCount);

    auto tEnd = std::chrono::steady_clock::now();
    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStart).count();

    std::cout << "\n========================================================\n";
    std::cout << "                  RÉSULTATS DES TESTS                   \n";
    std::cout << "========================================================\n";
    std::cout << " Total tests : " << (testFiles.size() + unitTestsCount) << "\n";
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
        std::filesystem::remove(utf8Path(tf), ec);
#else
        std::filesystem::remove(std::filesystem::path(tf), ec);
#endif
    }

    return (failed == 0) ? 0 : 1;
}
