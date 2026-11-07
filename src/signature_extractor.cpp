
#include "../include/signature_extractor.hpp"
#include "../include/lexer.hpp"
#include "../include/parser.hpp"
#include "../include/common.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace algo {

static std::string prettyTypeName(const std::string& typeName) {
    std::string n = normalize(typeName);
    if (n == "booleen") return "booléen";
    if (n == "reel") return "réel";
    if (n == "chaine") return "chaîne";
    if (n == "caractere") return "caractère";
    if (n == "entier") return "entier";
    return typeName;
}

// Formatage d'une signature de sous-programme
std::string SubprogramSignature::formatSignature() const {
    std::ostringstream oss;
    if (isFunction) {
        oss << "Fonction " << name << "(";
    } else {
        oss << "Procédure " << name << "(";
    }
    for (size_t i = 0; i < params.size(); ++i) {
        if (i > 0) oss << ", ";
        if (params[i].byRef) oss << "@";
        oss << params[i].name << ": " << prettyTypeName(params[i].paramType);
    }
    oss << ")";
    if (isFunction && !returnType.empty()) {
        oss << ": " << prettyTypeName(returnType);
    }
    return oss.str();
}

static std::string formatTypeDeclString(const TypeDecl& td) {
    std::ostringstream oss;
    oss << td.name << " = ";
    switch (td.typeSpec.kind) {
        case TypeKind::Array1D:
            oss << "Tableau de " << (td.arraySize > 0 ? td.arraySize : td.typeSpec.cols) << " " << prettyTypeName(td.elemType);
            break;
        case TypeKind::Array2D:
            oss << "Tableau de " << td.typeSpec.rows << " lignes * " << td.typeSpec.cols << " colonnes de " << prettyTypeName(td.elemType);
            break;
        case TypeKind::Record:
            oss << "Enregistrement\n";
            for (const auto& f : td.typeSpec.fields) {
                oss << "    ";
                for (size_t i = 0; i < f->names.size(); ++i) {
                    if (i > 0) oss << ", ";
                    oss << f->names[i];
                }
                oss << " : " << prettyTypeName(f->varType) << "\n";
            }
            oss << "Fin";
            break;
        case TypeKind::Enum:
            oss << "(";
            for (size_t i = 0; i < td.typeSpec.enumValues.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << "\"" << td.typeSpec.enumValues[i] << "\"";
            }
            oss << ")";
            break;
        case TypeKind::Subrange:
            oss << td.typeSpec.minInt << " .. " << td.typeSpec.maxInt;
            break;
        case TypeKind::Pointer:
            oss << "^" << prettyTypeName(td.elemType);
            break;
        default:
            if (!td.elemType.empty()) oss << prettyTypeName(td.elemType);
            else oss << prettyTypeName(td.typeSpec.name);
            break;
    }
    return oss.str();
}

static void formatTdoTable(std::ostringstream& oss, const std::vector<TdoEntry>& entries, const std::string& tableName, bool isGenerated) {
    oss << tableName;
    if (isGenerated) {
        oss << " (généré automatiquement)";
    }
    oss << " :\n";
    if (entries.empty()) {
        oss << "  (Néant)\n";
        return;
    }
    oss << "  Objet : Type/Nature\n";
    for (const auto& e : entries) {
        oss << "  ";
        for (size_t i = 0; i < e.names.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << e.names[i];
        }
        oss << " : " << (e.isConst ? e.typeOrNature : prettyTypeName(e.typeOrNature));
        if (e.isConst && !e.constValueStr.empty()) {
            oss << " = " << e.constValueStr;
        }
        oss << "\n";
    }
}

std::string SubprogramInfo::formatText() const {
    std::ostringstream oss;
    oss << "Signature :\n  " << signature.formatSignature() << "\n\n";

    oss << "TDNT respectif :\n";
    if (respectiveTdnt.empty()) {
        oss << "  (Néant)\n";
    } else {
        for (const auto& td : respectiveTdnt) {
            oss << "  " << formatTypeDeclString(td) << "\n";
        }
    }
    oss << "\n";

    formatTdoTable(oss, tdol, "TDOL", isTdolGenerated);
    return oss.str();
}

std::string ProgramSignaturesResult::formatText() const {
    std::ostringstream oss;
    oss << "======================================================================\n";
    oss << "  SOUS-PROGRAMMES & TABLES DE DÉCLARATIONS (TDO / TDNT)\n";
    if (!programName.empty()) {
        oss << "  Algorithme : " << programName << "\n";
    }
    oss << "======================================================================\n\n";

    if (subprograms.empty()) {
        oss << "Aucun sous-programme défini dans cet algorithme.\n\n";
    } else {
        for (size_t i = 0; i < subprograms.size(); ++i) {
            oss << "----------------------------------------------------------------------\n";
            oss << "[" << (i + 1) << "] " << subprograms[i].signature.formatSignature() << "\n";
            oss << "----------------------------------------------------------------------\n";
            oss << subprograms[i].formatText() << "\n";
        }
    }

    oss << "======================================================================\n";
    oss << "  ALGORITHME PRINCIPAL : " << (programName.empty() ? "Sans nom" : programName) << "\n";
    oss << "======================================================================\n";

    oss << "TDNT Global";
    if (isGlobalTdntGenerated) {
        oss << " (généré automatiquement)";
    }
    oss << " :\n";
    if (globalTdnt.empty()) {
        oss << "  (Néant)\n";
    } else {
        for (const auto& td : globalTdnt) {
            oss << "  " << formatTypeDeclString(td) << "\n";
        }
    }
    oss << "\n";

    formatTdoTable(oss, mainTdo, "TDO", isMainTdoGenerated);
    oss << "======================================================================\n";

    return oss.str();
}

static std::string escapeJson(const std::string& s) {
    std::ostringstream o;
    for (char c : s) {
        if (c == '"') o << "\\\"";
        else if (c == '\\') o << "\\\\";
        else if (c == '\b') o << "\\b";
        else if (c == '\f') o << "\\f";
        else if (c == '\n') o << "\\n";
        else if (c == '\r') o << "\\r";
        else if (c == '\t') o << "\\t";
        else o << c;
    }
    return o.str();
}

std::string ProgramSignaturesResult::formatJson() const {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"program\": \"" << escapeJson(programName) << "\",\n";
    oss << "  \"subprograms\": [\n";
    for (size_t i = 0; i < subprograms.size(); ++i) {
        const auto& sp = subprograms[i];
        oss << "    {\n";
        oss << "      \"name\": \"" << escapeJson(sp.signature.name) << "\",\n";
        oss << "      \"kind\": \"" << (sp.signature.isFunction ? "function" : "procedure") << "\",\n";
        oss << "      \"signature\": \"" << escapeJson(sp.signature.formatSignature()) << "\",\n";
        oss << "      \"return_type\": \"" << escapeJson(sp.signature.returnType) << "\",\n";
        oss << "      \"params\": [\n";
        for (size_t j = 0; j < sp.signature.params.size(); ++j) {
            const auto& p = sp.signature.params[j];
            oss << "        {\"name\": \"" << escapeJson(p.name) << "\", \"type\": \"" << escapeJson(p.paramType)
                << "\", \"by_ref\": " << (p.byRef ? "true" : "false") << "}"
                << (j + 1 < sp.signature.params.size() ? ",\n" : "\n");
        }
        oss << "      ],\n";
        oss << "      \"is_tdol_generated\": " << (sp.isTdolGenerated ? "true" : "false") << ",\n";
        oss << "      \"tdol\": [\n";
        for (size_t j = 0; j < sp.tdol.size(); ++j) {
            const auto& t = sp.tdol[j];
            oss << "        {\"names\": [";
            for (size_t k = 0; k < t.names.size(); ++k) {
                if (k > 0) oss << ", ";
                oss << "\"" << escapeJson(t.names[k]) << "\"";
            }
            oss << "], \"type\": \"" << escapeJson(t.typeOrNature) << "\", \"is_const\": "
                << (t.isConst ? "true" : "false") << "}" << (j + 1 < sp.tdol.size() ? ",\n" : "\n");
        }
        oss << "      ],\n";
        oss << "      \"respective_tdnt\": [";
        for (size_t j = 0; j < sp.respectiveTdnt.size(); ++j) {
            if (j > 0) oss << ", ";
            oss << "\"" << escapeJson(sp.respectiveTdnt[j].name) << "\"";
        }
        oss << "]\n";
        oss << "    }" << (i + 1 < subprograms.size() ? ",\n" : "\n");
    }
    oss << "  ],\n";
    oss << "  \"is_global_tdnt_generated\": " << (isGlobalTdntGenerated ? "true" : "false") << ",\n";
    oss << "  \"global_tdnt\": [";
    for (size_t j = 0; j < globalTdnt.size(); ++j) {
        if (j > 0) oss << ", ";
        oss << "\"" << escapeJson(globalTdnt[j].name) << "\"";
    }
    oss << "],\n";
    oss << "  \"is_main_tdo_generated\": " << (isMainTdoGenerated ? "true" : "false") << ",\n";
    oss << "  \"main_tdo\": [\n";
    for (size_t j = 0; j < mainTdo.size(); ++j) {
        const auto& t = mainTdo[j];
        oss << "    {\"names\": [";
        for (size_t k = 0; k < t.names.size(); ++k) {
            if (k > 0) oss << ", ";
            oss << "\"" << escapeJson(t.names[k]) << "\"";
        }
        oss << "], \"type\": \"" << escapeJson(t.typeOrNature) << "\", \"is_const\": "
            << (t.isConst ? "true" : "false") << "}" << (j + 1 < mainTdo.size() ? ",\n" : "\n");
    }
    oss << "  ]\n";
    oss << "}\n";
    return oss.str();
}

SignatureExtractor::SignatureExtractor(const Program& prog)
    : prog_(prog) {}

ProgramSignaturesResult SignatureExtractor::extractFromProgram(const Program& prog) {
    SignatureExtractor extractor(prog);
    return extractor.extract();
}

ProgramSignaturesResult SignatureExtractor::extractFromSource(const std::string& source, const std::string& filePath) {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    if (!filePath.empty()) {
        parser.setCurrentFilePath(filePath);
    }
    Program prog = parser.parse();
    return extractFromProgram(prog);
}

static bool isBuiltinType(const std::string& typeName) {
    std::string n = normalize(typeName);
    if (n.empty()) return true;
    if (n[0] == '^') n = n.substr(1);
    if (n == "entier" || n == "réel" || n == "reel" ||
        n == "booléen" || n == "booleen" ||
        n == "chaîne" || n == "chaine" ||
        n == "caractère" || n == "caractere" ||
        n == "fichier texte" || n == "fichier_texte" ||
        n == "fichier" || n == "nil" || n == "null" ||
        n == "void" || n == "tableau" || n == "enregistrement" ||
        n == "pointeur" || n == "inconnu") {
        return true;
    }
    return false;
}

std::vector<TypeDecl> SignatureExtractor::inferMissingTypeDecls(
    std::map<std::string, std::string>& outVarToCustomType) const {

    std::vector<TypeDecl> inferredDecls;
    std::map<std::string, std::string> customTypeOriginalNames; // normalized -> original name

    // 1. Collecter tous les noms de types personnalisés mentionnés dans les signatures
    auto registerCustomType = [&](const std::string& t) {
        if (t.empty()) return;
        std::string raw = t;
        if (raw[0] == '^') raw = raw.substr(1);
        if (isBuiltinType(raw)) return;
        for (const auto& td : prog_.typeDecls) {
            if (normalize(td.name) == normalize(raw)) return;
        }
        customTypeOriginalNames[normalize(raw)] = raw;
    };

    for (const auto& proc : prog_.procedures) {
        for (const auto& p : proc.params) registerCustomType(p.paramType);
        for (const auto& lv : proc.localVars) registerCustomType(lv.varType);
    }
    for (const auto& func : prog_.functions) {
        for (const auto& p : func.params) registerCustomType(p.paramType);
        registerCustomType(func.returnType);
        for (const auto& lv : func.localVars) registerCustomType(lv.varType);
    }
    for (const auto& gv : prog_.varDecls) {
        registerCustomType(gv.varType);
    }

    // 2. Associer les paramètres formels à leurs types
    for (const auto& proc : prog_.procedures) {
        for (const auto& p : proc.params) {
            if (!isBuiltinType(p.paramType)) {
                outVarToCustomType[normalize(p.name)] = p.paramType;
            }
        }
    }
    for (const auto& func : prog_.functions) {
        for (const auto& p : func.params) {
            if (!isBuiltinType(p.paramType)) {
                outVarToCustomType[normalize(p.name)] = p.paramType;
            }
        }
    }

    // 3. Détecter les appels de sous-programmes pour lier les arguments aux types de paramètres
    auto scanCalls = [&](const std::vector<std::shared_ptr<Stmt>>& stmts, auto& self) -> void {
        for (const auto& stmt : stmts) {
            if (!stmt) continue;
            if (auto call = std::dynamic_pointer_cast<CallStmt>(stmt)) {
                std::string calleeName = normalize(call->name);
                for (const auto& p : prog_.procedures) {
                    if (normalize(p.name) == calleeName) {
                        for (size_t k = 0; k < call->args.size() && k < p.params.size(); ++k) {
                            if (!isBuiltinType(p.params[k].paramType)) {
                                if (auto v = std::dynamic_pointer_cast<VariableExpr>(call->args[k])) {
                                    outVarToCustomType[normalize(v->name)] = p.params[k].paramType;
                                }
                            }
                        }
                        break;
                    }
                }
            } else if (auto ifSt = std::dynamic_pointer_cast<IfStmt>(stmt)) {
                auto scanExprCalls = [&](const std::shared_ptr<Expr>& expr, auto& scanExprSelf) -> void {
                    if (!expr) return;
                    if (auto call = std::dynamic_pointer_cast<CallExpr>(expr)) {
                        std::string calleeName = normalize(call->name);
                        for (const auto& f : prog_.functions) {
                            if (normalize(f.name) == calleeName) {
                                for (size_t k = 0; k < call->args.size() && k < f.params.size(); ++k) {
                                    if (!isBuiltinType(f.params[k].paramType)) {
                                        if (auto v = std::dynamic_pointer_cast<VariableExpr>(call->args[k])) {
                                            outVarToCustomType[normalize(v->name)] = f.params[k].paramType;
                                        }
                                    }
                                }
                                break;
                            }
                        }
                        for (const auto& a : call->args) scanExprSelf(a, scanExprSelf);
                    } else if (auto un = std::dynamic_pointer_cast<UnaryExpr>(expr)) {
                        scanExprSelf(un->operand, scanExprSelf);
                    } else if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
                        scanExprSelf(bin->left, scanExprSelf);
                        scanExprSelf(bin->right, scanExprSelf);
                    }
                };
                scanExprCalls(ifSt->condition, scanExprCalls);
                self(ifSt->thenBlock, self);
                for (const auto& eb : ifSt->elseIfBlocks) {
                    scanExprCalls(eb.condition, scanExprCalls);
                    self(eb.body, self);
                }
                self(ifSt->elseBlock, self);
            } else if (auto forSt = std::dynamic_pointer_cast<ForStmt>(stmt)) {
                self(forSt->body, self);
            } else if (auto wh = std::dynamic_pointer_cast<WhileStmt>(stmt)) {
                self(wh->body, self);
            } else if (auto rep = std::dynamic_pointer_cast<RepeatStmt>(stmt)) {
                self(rep->body, self);
            }
        }
    };

    scanCalls(prog_.mainBody, scanCalls);
    for (const auto& proc : prog_.procedures) scanCalls(proc.body, scanCalls);
    for (const auto& func : prog_.functions) scanCalls(func.body, scanCalls);

    // 4. Analyser les accès aux membres (enregistrements) et tableaux
    struct FieldCollector {
        std::string name;
        std::string type = "entier";
    };
    std::map<std::string, std::vector<FieldCollector>> recordFields;
    std::map<std::string, std::set<std::string>> recordFieldNames;

    struct ArrayCollector {
        std::string elemType = "entier";
        int size = 0;
        bool is2D = false;
        int rows = 0;
        int cols = 0;
    };
    std::map<std::string, ArrayCollector> arrayInfos;

    auto recordField = [&](const std::string& typeNorm, const std::string& fName, const std::string& fType) {
        if (recordFieldNames[typeNorm].find(normalize(fName)) == recordFieldNames[typeNorm].end()) {
            recordFieldNames[typeNorm].insert(normalize(fName));
            FieldCollector fc;
            fc.name = fName;
            fc.type = (fType.empty() || fType == "inconnu") ? "entier" : fType;
            recordFields[typeNorm].push_back(fc);
        } else {
            for (auto& f : recordFields[typeNorm]) {
                if (normalize(f.name) == normalize(fName)) {
                    if (f.type == "entier" && !fType.empty() && fType != "inconnu" && fType != "entier") {
                        f.type = fType;
                    }
                    break;
                }
            }
        }
    };

    auto scanBodiesForTypes = [&](const std::vector<std::shared_ptr<Stmt>>& stmts, auto& self) -> void {
        for (const auto& stmt : stmts) {
            if (!stmt) continue;

            if (auto ma = std::dynamic_pointer_cast<MemberAssignStmt>(stmt)) {
                std::shared_ptr<Expr> root = ma->target;
                while (auto memAcc = std::dynamic_pointer_cast<MemberAccessExpr>(root)) {
                    root = memAcc->object;
                }
                if (auto v = std::dynamic_pointer_cast<VariableExpr>(root)) {
                    std::string varNorm = normalize(v->name);
                    std::string tName;
                    auto it = outVarToCustomType.find(varNorm);
                    if (it != outVarToCustomType.end()) {
                        tName = it->second;
                    } else {
                        tName = "Type_" + v->name;
                        outVarToCustomType[varNorm] = tName;
                        customTypeOriginalNames[normalize(tName)] = tName;
                    }
                    std::map<std::string, std::string> emptyMap;
                    std::string valType = inferExprType(ma->value, emptyMap, prog_.typeDecls);
                    recordField(normalize(tName), ma->member, valType);
                }
            } else if (auto aas = std::dynamic_pointer_cast<ArrayAssignStmt>(stmt)) {
                std::string arrName = aas->target;
                if (arrName.empty() && aas->targetExpr) {
                    if (auto v = std::dynamic_pointer_cast<VariableExpr>(aas->targetExpr)) arrName = v->name;
                }
                if (!arrName.empty()) {
                    std::string varNorm = normalize(arrName);
                    std::string tName;
                    auto it = outVarToCustomType.find(varNorm);
                    if (it != outVarToCustomType.end()) {
                        tName = it->second;
                    }
                    if (!tName.empty()) {
                        std::string tNorm = normalize(tName);
                        std::map<std::string, std::string> emptyMap;
                        std::string valType = inferExprType(aas->value, emptyMap, prog_.typeDecls);
                        if (!valType.empty() && valType != "inconnu") {
                            arrayInfos[tNorm].elemType = valType;
                        }
                        if (aas->index2) {
                            arrayInfos[tNorm].is2D = true;
                            if (auto n1 = std::dynamic_pointer_cast<NumberExpr>(aas->index)) {
                                arrayInfos[tNorm].rows = std::max(arrayInfos[tNorm].rows, (int)n1->value + 1);
                            }
                            if (auto n2 = std::dynamic_pointer_cast<NumberExpr>(aas->index2)) {
                                arrayInfos[tNorm].cols = std::max(arrayInfos[tNorm].cols, (int)n2->value + 1);
                            }
                        } else {
                            if (auto n1 = std::dynamic_pointer_cast<NumberExpr>(aas->index)) {
                                arrayInfos[tNorm].size = std::max(arrayInfos[tNorm].size, (int)n1->value + 1);
                            }
                        }
                    }
                }
            } else if (auto ret = std::dynamic_pointer_cast<ReturnStmt>(stmt)) {
                auto scanExprMembers = [&](const std::shared_ptr<Expr>& expr, auto& scanExprSelf) -> void {
                    if (!expr) return;
                    if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
                        if (auto mem = std::dynamic_pointer_cast<MemberAccessExpr>(bin->left)) {
                            if (auto v = std::dynamic_pointer_cast<VariableExpr>(mem->object)) {
                                auto it = outVarToCustomType.find(normalize(v->name));
                                if (it != outVarToCustomType.end()) {
                                    std::map<std::string, std::string> emptyMap;
                                    std::string rType = inferExprType(bin->right, emptyMap, prog_.typeDecls);
                                    recordField(normalize(it->second), mem->member, rType);
                                }
                            }
                        }
                        scanExprSelf(bin->left, scanExprSelf);
                        scanExprSelf(bin->right, scanExprSelf);
                    } else if (auto mem = std::dynamic_pointer_cast<MemberAccessExpr>(expr)) {
                        if (auto v = std::dynamic_pointer_cast<VariableExpr>(mem->object)) {
                            auto it = outVarToCustomType.find(normalize(v->name));
                            if (it != outVarToCustomType.end()) {
                                recordField(normalize(it->second), mem->member, "entier");
                            }
                        }
                    } else if (auto un = std::dynamic_pointer_cast<UnaryExpr>(expr)) {
                        scanExprSelf(un->operand, scanExprSelf);
                    } else if (auto call = std::dynamic_pointer_cast<CallExpr>(expr)) {
                        for (const auto& a : call->args) scanExprSelf(a, scanExprSelf);
                    }
                };
                scanExprMembers(ret->value, scanExprMembers);
            } else if (auto ifSt = std::dynamic_pointer_cast<IfStmt>(stmt)) {
                self(ifSt->thenBlock, self);
                for (const auto& eb : ifSt->elseIfBlocks) self(eb.body, self);
                self(ifSt->elseBlock, self);
            } else if (auto forSt = std::dynamic_pointer_cast<ForStmt>(stmt)) {
                self(forSt->body, self);
            } else if (auto wh = std::dynamic_pointer_cast<WhileStmt>(stmt)) {
                self(wh->body, self);
            } else if (auto rep = std::dynamic_pointer_cast<RepeatStmt>(stmt)) {
                self(rep->body, self);
            }
        }
    };

    scanBodiesForTypes(prog_.mainBody, scanBodiesForTypes);
    for (const auto& proc : prog_.procedures) scanBodiesForTypes(proc.body, scanBodiesForTypes);
    for (const auto& func : prog_.functions) scanBodiesForTypes(func.body, scanBodiesForTypes);

    // 5. Construire les TypeDecl inférées
    for (const auto& kv : customTypeOriginalNames) {
        std::string norm = kv.first;
        std::string orig = kv.second;

        if (recordFields.find(norm) != recordFields.end() && !recordFields[norm].empty()) {
            TypeDecl td;
            td.name = orig;
            td.typeSpec.kind = TypeKind::Record;
            td.typeSpec.name = orig;
            for (const auto& f : recordFields[norm]) {
                auto vd = std::make_shared<VarDecl>();
                vd->names.push_back(f.name);
                vd->varType = f.type;
                td.typeSpec.fields.push_back(vd);
            }
            inferredDecls.push_back(std::move(td));
        } else if (arrayInfos.find(norm) != arrayInfos.end()) {
            const auto& ai = arrayInfos[norm];
            TypeDecl td;
            td.name = orig;
            td.elemType = ai.elemType.empty() ? "entier" : ai.elemType;
            td.typeSpec.elemType = td.elemType;
            if (ai.is2D) {
                td.typeSpec.kind = TypeKind::Array2D;
                td.typeSpec.rows = std::max(ai.rows, 2);
                td.typeSpec.cols = std::max(ai.cols, 2);
            } else {
                td.typeSpec.kind = TypeKind::Array1D;
                td.arraySize = std::max(ai.size, 2);
                td.typeSpec.rows = td.arraySize;
            }
            inferredDecls.push_back(std::move(td));
        }
    }

    return inferredDecls;
}

ProgramSignaturesResult SignatureExtractor::extract() const {
    ProgramSignaturesResult res;
    res.programName = prog_.name;

    // 0. Découverte / Inférence des déclarations TDNT manquantes
    std::map<std::string, std::string> varToCustomType;
    std::vector<TypeDecl> inferredTypes = inferMissingTypeDecls(varToCustomType);

    std::vector<TypeDecl> allTdnt = prog_.typeDecls;
    for (auto& td : inferredTypes) {
        bool exists = false;
        for (const auto& existing : allTdnt) {
            if (normalize(existing.name) == normalize(td.name)) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            allTdnt.push_back(td);
        }
    }

    res.globalTdnt = allTdnt;
    res.isGlobalTdntGenerated = (prog_.typeDecls.empty() && !allTdnt.empty());

    // 1. Traitement des procédures
    for (const auto& proc : prog_.procedures) {
        SubprogramInfo info;
        info.signature.isFunction = false;
        info.signature.name = proc.name;
        info.signature.params = proc.params;
        info.signature.returnType = "";
        info.signature.line = proc.line;

        info.tdol = extractOrInferTdo(
            proc.body, proc.params, proc.localVars, proc.constDecls, true, info.isTdolGenerated, varToCustomType, allTdnt);

        info.respectiveTdnt = extractRespectiveTdnt(proc.params, "", info.tdol, allTdnt);
        res.subprograms.push_back(std::move(info));
    }

    // 2. Traitement des fonctions
    for (const auto& func : prog_.functions) {
        SubprogramInfo info;
        info.signature.isFunction = true;
        info.signature.name = func.name;
        info.signature.params = func.params;
        info.signature.returnType = func.returnType;
        info.signature.line = func.line;

        info.tdol = extractOrInferTdo(
            func.body, func.params, func.localVars, func.constDecls, true, info.isTdolGenerated, varToCustomType, allTdnt);

        info.respectiveTdnt = extractRespectiveTdnt(func.params, func.returnType, info.tdol, allTdnt);
        res.subprograms.push_back(std::move(info));
    }

    // 3. Traitement du programme principal
    std::vector<ParamDecl> noParams;
    res.mainTdo = extractOrInferTdo(
        prog_.mainBody, noParams, prog_.varDecls, prog_.constDecls, false, res.isMainTdoGenerated, varToCustomType, allTdnt);

    return res;
}

// Inférence du type d'une expression
std::string SignatureExtractor::inferExprType(
    const std::shared_ptr<Expr>& expr,
    const std::map<std::string, std::string>& knownTypes,
    const std::vector<TypeDecl>& allTdnt) const {
    if (!expr) return "inconnu";

    if (std::dynamic_pointer_cast<NumberExpr>(expr)) return "entier";
    if (std::dynamic_pointer_cast<RealExpr>(expr)) return "réel";
    if (std::dynamic_pointer_cast<StringExpr>(expr)) return "chaîne";
    if (std::dynamic_pointer_cast<BoolExpr>(expr)) return "booléen";
    if (std::dynamic_pointer_cast<NilExpr>(expr)) return "pointeur";

    if (auto v = std::dynamic_pointer_cast<VariableExpr>(expr)) {
        auto it = knownTypes.find(normalize(v->name));
        if (it != knownTypes.end()) return it->second;
        return "inconnu";
    }

    if (auto un = std::dynamic_pointer_cast<UnaryExpr>(expr)) {
        std::string op = normalize(un->op);
        if (op == "non") return "booléen";
        return inferExprType(un->operand, knownTypes, allTdnt);
    }

    if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
        std::string op = normalize(bin->op);
        if (op == "=" || op == "<>" || op == "!=" || op == "≠" ||
            op == "<" || op == "<=" || op == "≤" ||
            op == ">" || op == ">=" || op == "≥" ||
            op == "et" || op == "ou" || op == "dans") {
            return "booléen";
        }
        if (op == "/" || op == "÷") return "réel";
        if (op == "div" || op == "mod") return "entier";

        std::string lt = inferExprType(bin->left, knownTypes, allTdnt);
        std::string rt = inferExprType(bin->right, knownTypes, allTdnt);
        if (lt == "réel" || rt == "réel") return "réel";
        if (lt == "chaîne" || rt == "chaîne") return "chaîne";
        if (lt == "entier" && rt == "entier") return "entier";
        if (lt != "inconnu") return lt;
        if (rt != "inconnu") return rt;
        return "entier";
    }

    if (auto call = std::dynamic_pointer_cast<CallExpr>(expr)) {
        std::string fn = normalize(call->name);
        for (const auto& f : prog_.functions) {
            if (normalize(f.name) == fn) {
                return f.returnType;
            }
        }
        // Fonctions prédéfinies standard
        if (fn == "racinecarree" || fn == "racine" || fn == "sqrt" ||
            fn == "sin" || fn == "cos" || fn == "tan" || fn == "exp" || fn == "ln") {
            return "réel";
        }
        if (fn == "ent" || fn == "tronc" || fn == "arrondi" || fn == "long" ||
            fn == "pos" || fn == "ord" || fn == "alea" || fn == "hasard" ||
            fn == "taille_fichier" || fn == "position_fichier") {
            return "entier";
        }
        if (fn == "chr") return "caractère";
        if (fn == "souschaine" || fn == "convch" || fn == "majus" || fn == "minus") {
            return "chaîne";
        }
        if (fn == "fin_fichier") return "booléen";
        if (fn == "valeur") {
            if (!call->args.empty()) {
                if (auto strLit = std::dynamic_pointer_cast<StringExpr>(call->args[0])) {
                    if (strLit->value.find('.') != std::string::npos || strLit->value.find('e') != std::string::npos) {
                        return "réel";
                    }
                }
            }
            return "entier";
        }
        if (fn == "abs" || fn == "carre") {
            if (!call->args.empty()) return inferExprType(call->args[0], knownTypes, allTdnt);
            return "entier";
        }
    }

    if (auto arr = std::dynamic_pointer_cast<ArrayAccessExpr>(expr)) {
        std::string tName = arr->name;
        if (tName.empty() && arr->target) {
            if (auto v = std::dynamic_pointer_cast<VariableExpr>(arr->target)) {
                tName = v->name;
            }
        }
        auto it = knownTypes.find(normalize(tName));
        if (it != knownTypes.end()) {
            std::string tn = it->second;
            for (const auto& td : allTdnt) {
                if (normalize(td.name) == normalize(tn)) {
                    return td.elemType.empty() ? td.typeSpec.elemType : td.elemType;
                }
            }
        }
        return "entier";
    }

    if (auto mem = std::dynamic_pointer_cast<MemberAccessExpr>(expr)) {
        if (auto v = std::dynamic_pointer_cast<VariableExpr>(mem->object)) {
            auto it = knownTypes.find(normalize(v->name));
            if (it != knownTypes.end()) {
                std::string recType = it->second;
                if (!recType.empty() && recType[0] == '^') recType = recType.substr(1);
                for (const auto& td : allTdnt) {
                    if (normalize(td.name) == normalize(recType) && td.typeSpec.kind == TypeKind::Record) {
                        for (const auto& f : td.typeSpec.fields) {
                            for (const auto& fn : f->names) {
                                if (normalize(fn) == normalize(mem->member)) {
                                    return f->varType;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return "inconnu";
}

// Extraction ou génération de la table des déclarations (TDO / TDOL)
std::vector<TdoEntry> SignatureExtractor::extractOrInferTdo(
    const std::vector<std::shared_ptr<Stmt>>& body,
    const std::vector<ParamDecl>& params,
    const std::vector<VarDecl>& declaredVars,
    const std::vector<ConstDecl>& declaredConsts,
    bool isSubprogram,
    bool& outGenerated,
    const std::map<std::string, std::string>& varToCustomType,
    const std::vector<TypeDecl>& allTdnt) const {

    // Cas 1 : Table déclarée explicitement dans le code
    if (!declaredVars.empty() || !declaredConsts.empty()) {
        outGenerated = false;
        std::vector<TdoEntry> entries;

        // Constantes
        for (const auto& cd : declaredConsts) {
            TdoEntry entry;
            entry.names.push_back(cd.name);
            entry.isConst = true;
            if (cd.value.type != ValueType::Null) {
                entry.typeOrNature = "Constante";
                entry.constValueStr = cd.value.asString();
            } else {
                entry.typeOrNature = "Constante";
            }
            entries.push_back(entry);
        }

        // Variables déclarées
        for (const auto& vd : declaredVars) {
            TdoEntry entry;
            entry.names = vd.names;
            entry.typeOrNature = vd.varType;
            entry.isConst = false;
            entries.push_back(entry);
        }

        return entries;
    }

    // Cas 2 : Table absente -> génération automatique par analyse du corps
    outGenerated = true;

    // Collecte des identifiants exclus (paramètres, noms de sous-programmes, mots-clés)
    std::set<std::string> excluded;
    for (const auto& p : params) {
        excluded.insert(normalize(p.name));
    }
    for (const auto& p : prog_.procedures) {
        excluded.insert(normalize(p.name));
    }
    for (const auto& f : prog_.functions) {
        excluded.insert(normalize(f.name));
    }
    // Si c'est un sous-programme, exclure les constantes globales
    if (isSubprogram) {
        for (const auto& c : prog_.constDecls) {
            excluded.insert(normalize(c.name));
        }
    }

    std::map<std::string, std::string> varTypes; // nom normalisé -> type
    std::vector<std::string> varOrder;           // ordre d'apparition

    auto recordVar = [&](const std::string& name, const std::string& inferredType) {
        std::string n = normalize(name);
        if (n.empty() || excluded.find(n) != excluded.end()) return;
        if (n == "vrai" || n == "faux" || n == "nil" || n == "null") return;

        std::string finalType = inferredType;
        auto itCustom = varToCustomType.find(n);
        if (itCustom != varToCustomType.end() && !itCustom->second.empty()) {
            finalType = itCustom->second;
        }

        if (varTypes.find(n) == varTypes.end()) {
            varTypes[n] = finalType;
            varOrder.push_back(name);
        } else {
            std::string cur = varTypes[n];
            if (cur == "inconnu" && finalType != "inconnu") {
                varTypes[n] = finalType;
            } else if ((cur == "entier" || cur == "Tableau" || cur == "Enregistrement") &&
                       !isBuiltinType(finalType) && finalType != "inconnu") {
                varTypes[n] = finalType;
            }
        }
    };

    // Parcours de l'AST pour détecter les variables et inférer leurs types
    std::function<void(const std::shared_ptr<Expr>&)> scanExpr = [&](const std::shared_ptr<Expr>& expr) {
        if (!expr) return;
        if (auto v = std::dynamic_pointer_cast<VariableExpr>(expr)) {
            recordVar(v->name, "inconnu");
        } else if (auto un = std::dynamic_pointer_cast<UnaryExpr>(expr)) {
            scanExpr(un->operand);
        } else if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
            scanExpr(bin->left);
            scanExpr(bin->right);
        } else if (auto call = std::dynamic_pointer_cast<CallExpr>(expr)) {
            std::string calleeName = normalize(call->name);
            for (const auto& f : prog_.functions) {
                if (normalize(f.name) == calleeName) {
                    for (size_t k = 0; k < call->args.size() && k < f.params.size(); ++k) {
                        if (auto v = std::dynamic_pointer_cast<VariableExpr>(call->args[k])) {
                            recordVar(v->name, f.params[k].paramType);
                        }
                    }
                    break;
                }
            }
            for (const auto& a : call->args) scanExpr(a);
        } else if (auto arr = std::dynamic_pointer_cast<ArrayAccessExpr>(expr)) {
            if (!arr->name.empty()) {
                std::string arrType = "Tableau";
                auto itCustom = varToCustomType.find(normalize(arr->name));
                if (itCustom != varToCustomType.end()) {
                    arrType = itCustom->second;
                } else {
                    for (const auto& td : allTdnt) {
                        if (td.typeSpec.kind == TypeKind::Array1D || td.typeSpec.kind == TypeKind::Array2D) {
                            arrType = td.name;
                            break;
                        }
                    }
                }
                recordVar(arr->name, arrType);
            }
            if (arr->target) scanExpr(arr->target);
            if (arr->index) scanExpr(arr->index);
            if (arr->index2) scanExpr(arr->index2);
        } else if (auto mem = std::dynamic_pointer_cast<MemberAccessExpr>(expr)) {
            if (auto v = std::dynamic_pointer_cast<VariableExpr>(mem->object)) {
                std::string recType = "Enregistrement";
                auto itCustom = varToCustomType.find(normalize(v->name));
                if (itCustom != varToCustomType.end()) {
                    recType = itCustom->second;
                } else {
                    for (const auto& td : allTdnt) {
                        if (td.typeSpec.kind == TypeKind::Record) {
                            for (const auto& f : td.typeSpec.fields) {
                                for (const auto& fn : f->names) {
                                    if (normalize(fn) == normalize(mem->member)) {
                                        recType = td.name;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                recordVar(v->name, recType);
            } else {
                scanExpr(mem->object);
            }
        } else if (auto deref = std::dynamic_pointer_cast<DerefExpr>(expr)) {
            scanExpr(deref->target);
        }
    };

    std::function<void(const std::vector<std::shared_ptr<Stmt>>&)> scanStmts = [&](const std::vector<std::shared_ptr<Stmt>>& stmts) {
        for (const auto& stmt : stmts) {
            if (!stmt) continue;

            if (auto as = std::dynamic_pointer_cast<AssignStmt>(stmt)) {
                scanExpr(as->value);
                std::string t = inferExprType(as->value, varTypes, allTdnt);
                recordVar(as->target, t);
            } else if (auto ma = std::dynamic_pointer_cast<MemberAssignStmt>(stmt)) {
                std::shared_ptr<Expr> root = ma->target;
                while (auto memAcc = std::dynamic_pointer_cast<MemberAccessExpr>(root)) {
                    root = memAcc->object;
                }
                if (auto v = std::dynamic_pointer_cast<VariableExpr>(root)) {
                    std::string varNorm = normalize(v->name);
                    std::string recType = "Enregistrement";
                    auto itCustom = varToCustomType.find(varNorm);
                    if (itCustom != varToCustomType.end()) {
                        recType = itCustom->second;
                    } else {
                        for (const auto& td : allTdnt) {
                            if (td.typeSpec.kind == TypeKind::Record) {
                                for (const auto& f : td.typeSpec.fields) {
                                    for (const auto& fn : f->names) {
                                        if (normalize(fn) == normalize(ma->member)) {
                                            recType = td.name;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    recordVar(v->name, recType);
                }
                scanExpr(ma->target);
                scanExpr(ma->value);
            } else if (auto aas = std::dynamic_pointer_cast<ArrayAssignStmt>(stmt)) {
                std::string arrName = aas->target;
                if (arrName.empty() && aas->targetExpr) {
                    if (auto v = std::dynamic_pointer_cast<VariableExpr>(aas->targetExpr)) {
                        arrName = v->name;
                    }
                }
                if (!arrName.empty()) {
                    std::string arrType = "Tableau";
                    auto itCustom = varToCustomType.find(normalize(arrName));
                    if (itCustom != varToCustomType.end()) {
                        arrType = itCustom->second;
                    } else {
                        for (const auto& td : allTdnt) {
                            if (td.typeSpec.kind == TypeKind::Array1D || td.typeSpec.kind == TypeKind::Array2D) {
                                arrType = td.name;
                                break;
                            }
                        }
                    }
                    recordVar(arrName, arrType);
                }
                scanExpr(aas->index);
                if (aas->index2) scanExpr(aas->index2);
                scanExpr(aas->value);
            } else if (auto forSt = std::dynamic_pointer_cast<ForStmt>(stmt)) {
                recordVar(forSt->varName, "entier");
                scanExpr(forSt->start);
                scanExpr(forSt->end);
                if (forSt->step) scanExpr(forSt->step);
                scanStmts(forSt->body);
            } else if (auto wh = std::dynamic_pointer_cast<WhileStmt>(stmt)) {
                scanExpr(wh->condition);
                scanStmts(wh->body);
            } else if (auto rep = std::dynamic_pointer_cast<RepeatStmt>(stmt)) {
                scanStmts(rep->body);
                scanExpr(rep->condition);
            } else if (auto ifSt = std::dynamic_pointer_cast<IfStmt>(stmt)) {
                scanExpr(ifSt->condition);
                scanStmts(ifSt->thenBlock);
                for (const auto& eb : ifSt->elseIfBlocks) {
                    scanExpr(eb.condition);
                    scanStmts(eb.body);
                }
                scanStmts(ifSt->elseBlock);
            } else if (auto sw = std::dynamic_pointer_cast<SwitchStmt>(stmt)) {
                scanExpr(sw->expr);
                for (const auto& sc : sw->cases) {
                    for (const auto& val : sc.values) scanExpr(val);
                    scanStmts(sc.body);
                }
            } else if (auto ret = std::dynamic_pointer_cast<ReturnStmt>(stmt)) {
                if (ret->value) scanExpr(ret->value);
            } else if (auto wr = std::dynamic_pointer_cast<WriteStmt>(stmt)) {
                for (const auto& e : wr->args) scanExpr(e);
            } else if (auto rd = std::dynamic_pointer_cast<ReadStmt>(stmt)) {
                for (const auto& tgt : rd->targets) {
                    if (tgt.targetExpr) {
                        if (auto v = std::dynamic_pointer_cast<VariableExpr>(tgt.targetExpr)) {
                            recordVar(v->name, "inconnu");
                        } else {
                            scanExpr(tgt.targetExpr);
                        }
                    }
                }
            } else if (auto alloc = std::dynamic_pointer_cast<AllocStmt>(stmt)) {
                if (auto v = std::dynamic_pointer_cast<VariableExpr>(alloc->target)) {
                    std::string ptrType = "Pointeur";
                    auto itCustom = varToCustomType.find(normalize(v->name));
                    if (itCustom != varToCustomType.end()) {
                        ptrType = itCustom->second;
                    } else {
                        for (const auto& td : allTdnt) {
                            if (td.typeSpec.kind == TypeKind::Pointer) {
                                ptrType = td.name;
                                break;
                            }
                        }
                    }
                    recordVar(v->name, ptrType);
                } else {
                    scanExpr(alloc->target);
                }
            } else if (auto fr = std::dynamic_pointer_cast<FreeStmt>(stmt)) {
                scanExpr(fr->target);
            } else if (auto call = std::dynamic_pointer_cast<CallStmt>(stmt)) {
                std::string calleeName = normalize(call->name);
                for (const auto& p : prog_.procedures) {
                    if (normalize(p.name) == calleeName) {
                        for (size_t k = 0; k < call->args.size() && k < p.params.size(); ++k) {
                            if (auto v = std::dynamic_pointer_cast<VariableExpr>(call->args[k])) {
                                recordVar(v->name, p.params[k].paramType);
                            }
                        }
                        break;
                    }
                }
                for (const auto& a : call->args) scanExpr(a);
            }
        }
    };

    scanStmts(body);

    // Résolution finale des variables dont le type est resté "inconnu" : défaut "entier"
    for (auto& kv : varTypes) {
        if (kv.second == "inconnu") {
            auto itCustom = varToCustomType.find(kv.first);
            if (itCustom != varToCustomType.end() && !itCustom->second.empty()) {
                kv.second = itCustom->second;
            } else {
                kv.second = "entier";
            }
        }
    }

    // Regroupement des variables par type identique (ex: i, j : entier)
    std::map<std::string, std::vector<std::string>> typeToVars;
    for (const auto& origName : varOrder) {
        std::string n = normalize(origName);
        std::string t = varTypes[n];
        typeToVars[t].push_back(origName);
    }

    std::vector<TdoEntry> generated;
    for (const auto& kv : typeToVars) {
        TdoEntry entry;
        entry.names = kv.second;
        entry.typeOrNature = kv.first;
        entry.isConst = false;
        generated.push_back(std::move(entry));
    }

    return generated;
}

// Extraction des types TDNT effectivement utilisés dans un sous-programme
std::vector<TypeDecl> SignatureExtractor::extractRespectiveTdnt(
    const std::vector<ParamDecl>& params,
    const std::string& returnType,
    const std::vector<TdoEntry>& tdol,
    const std::vector<TypeDecl>& allTdnt) const {

    std::set<std::string> usedTypeNames;

    auto checkTypeStr = [&](const std::string& t) {
        std::string norm = normalize(t);
        if (norm.empty()) return;
        if (norm[0] == '^') norm = norm.substr(1);
        usedTypeNames.insert(norm);
    };

    for (const auto& p : params) {
        checkTypeStr(p.paramType);
    }
    if (!returnType.empty()) {
        checkTypeStr(returnType);
    }
    for (const auto& entry : tdol) {
        if (!entry.isConst) {
            checkTypeStr(entry.typeOrNature);
        }
    }

    std::vector<TypeDecl> result;
    std::set<std::string> added;

    for (const auto& td : allTdnt) {
        std::string tdNorm = normalize(td.name);
        if (usedTypeNames.find(tdNorm) != usedTypeNames.end() && added.find(tdNorm) == added.end()) {
            result.push_back(td);
            added.insert(tdNorm);

            // Si le type dépend d'un autre type défini dans TDNT (ex: pointeur vers enregistrement)
            if (!td.elemType.empty()) {
                std::string elemNorm = normalize(td.elemType);
                if (elemNorm[0] == '^') elemNorm = elemNorm.substr(1);
                for (const auto& td2 : allTdnt) {
                    if (normalize(td2.name) == elemNorm && added.find(elemNorm) == added.end()) {
                        result.push_back(td2);
                        added.insert(elemNorm);
                    }
                }
            }
        }
    }

    return result;
}

} // namespace algo
