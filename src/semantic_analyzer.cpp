#include "../include/semantic_analyzer.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace algo {

static int levenshtein(const std::string& s1, const std::string& s2) {
    size_t len1 = s1.size(), len2 = s2.size();
    std::vector<std::vector<int>> d(len1 + 1, std::vector<int>(len2 + 1));
    for (size_t i = 0; i <= len1; ++i) d[i][0] = (int)i;
    for (size_t j = 0; j <= len2; ++j) d[0][j] = (int)j;
    for (size_t i = 1; i <= len1; ++i) {
        for (size_t j = 1; j <= len2; ++j) {
            int cost = (toLower(std::string(1, s1[i - 1])) == toLower(std::string(1, s2[j - 1]))) ? 0 : 1;
            d[i][j] = std::min({ d[i - 1][j] + 1, d[i][j - 1] + 1, d[i - 1][j - 1] + cost });
        }
    }
    return d[len1][len2];
}

SemanticAnalyzer::SemanticAnalyzer(const std::string& sourceCode) : sourceCode_(sourceCode) {
    splitSourceLines();
}

void SemanticAnalyzer::splitSourceLines() {
    sourceLines_.clear();
    std::istringstream iss(sourceCode_);
    std::string line;
    while (std::getline(iss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        sourceLines_.push_back(line);
    }
}

void SemanticAnalyzer::pushScope() {
    scopes_.emplace_back();
}

void SemanticAnalyzer::popScope() {
    if (!scopes_.empty()) {
        scopes_.pop_back();
    }
}

bool SemanticAnalyzer::declareSymbol(const std::string& name, const std::string& typeName, const TypeSpec& ts, SymbolKind kind, int line) {
    if (scopes_.empty()) pushScope();
    std::string norm = normalize(name);
    auto& curScope = scopes_.back();
    if (curScope.find(norm) != curScope.end()) {
        return false;
    }
    Symbol sym;
    sym.name = name;
    sym.typeName = typeName.empty() ? ts.name : typeName;
    sym.typeSpec = ts;
    sym.kind = kind;
    sym.declLine = line;
    sym.isUsed = false;
    curScope[norm] = sym;
    return true;
}

const Symbol* SemanticAnalyzer::findSymbol(const std::string& name, bool markUsed) {
    std::string norm = normalize(name);
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->find(norm);
        if (found != it->end()) {
            if (markUsed) {
                found->second.isUsed = true;
            }
            return &found->second;
        }
    }
    return nullptr;
}

std::vector<std::string> SemanticAnalyzer::getVisibleSymbolNames() const {
    std::vector<std::string> names;
    std::set<std::string> seen;
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        for (const auto& kv : *it) {
            if (seen.insert(kv.first).second) {
                names.push_back(kv.second.name);
            }
        }
    }
    static const std::vector<std::string> builtins = {
        "Racine", "Exp", "Ln", "Log", "Abs", "Ent", "Arrondi", "Carre",
        "Long", "Chr", "Ord", "ConvCh", "Valeur", "Sous_Chaine", "Pos",
        "Majus", "Minus", "Alea", "Graine", "Fin_fichier", "Lire", "Ecrire",
        "EstNum", "Est_num", "Effacer", "Sin", "Cos", "Tan", "Arctan", "Atan",
        "Frac", "Puissance", "Inserer", "Taille_Fichier", "Taille", "Position_Fichier",
        "Pointeur", "Aller_A", "Pointer"
    };
    for (const auto& b : builtins) {
        if (seen.insert(normalize(b)).second) {
            names.push_back(b);
        }
    }
    return names;
}

std::string SemanticAnalyzer::findClosestMatch(const std::string& target, const std::vector<std::string>& candidates, int maxDist) {
    if (target.empty() || candidates.empty()) return "";
    int bestDist = maxDist + 1;
    std::string bestCandidate;
    std::string normTarget = normalize(target);

    for (const auto& cand : candidates) {
        std::string normCand = normalize(cand);
        if (normCand == normTarget) continue;
        int d = levenshtein(normTarget, normCand);
        if (d < bestDist && d <= maxDist) {
            bestDist = d;
            bestCandidate = cand;
        }
    }
    return bestCandidate;
}

std::string SemanticAnalyzer::formatDiagnostic(int line, int col, const std::string& symbol, const std::string& msg, const std::string& suggestion) {
    std::ostringstream ss;
    if (line > 0 && line <= (int)sourceLines_.size()) {
        const std::string& srcLine = sourceLines_[line - 1];
        if (col <= 0 && !symbol.empty()) {
            size_t p = toLower(srcLine).find(toLower(symbol));
            if (p != std::string::npos) {
                col = (int)p + 1;
            } else {
                col = 1;
            }
        }
        if (col <= 0) col = 1;

        ss << "Erreur sémantique (ligne " << line << ", colonne " << col << ") :\n";
        ss << "    " << srcLine << "\n";
        ss << "    " << std::string(col > 1 ? col - 1 : 0, ' ') << "^";
        if (symbol.size() > 1) {
            ss << std::string(symbol.size() - 1, '~');
        }
        ss << "\n";
    } else {
        if (line > 0) ss << "Erreur sémantique ligne " << line << " : ";
        else ss << "Erreur sémantique : ";
    }

    ss << msg;
    if (!suggestion.empty()) {
        ss << "\nSuggestion : Vouliez-vous dire '" << suggestion << "' ?";
    }
    return ss.str();
}

void SemanticAnalyzer::reportError(int line, int col, const std::string& objectName, const std::string& msg, const std::string& suggestion) {
    SemanticDiagnostic diag;
    diag.isError = true;
    diag.line = line;
    diag.column = col;
    diag.objectName = objectName;
    diag.message = msg;
    diag.suggestion = suggestion;
    diag.formatted = formatDiagnostic(line, col, objectName, msg, suggestion);
    diagnostics_.push_back(diag);
}

void SemanticAnalyzer::reportWarning(int line, int col, const std::string& objectName, const std::string& msg) {
    SemanticDiagnostic diag;
    diag.isError = false;
    diag.line = line;
    diag.column = col;
    diag.objectName = objectName;
    diag.message = msg;

    std::ostringstream ss;
    if (line > 0 && line <= (int)sourceLines_.size()) {
        const std::string& srcLine = sourceLines_[line - 1];
        if (col <= 0 && !objectName.empty()) {
            size_t p = toLower(srcLine).find(toLower(objectName));
            if (p != std::string::npos) {
                col = (int)p + 1;
            } else {
                col = 1;
            }
        }
        if (col <= 0) col = 1;

        ss << "Avertissement (ligne " << line << ", colonne " << col << ") :\n";
        ss << "    " << srcLine << "\n";
        ss << "    " << std::string(col > 1 ? col - 1 : 0, ' ') << "^";
        if (objectName.size() > 1) {
            ss << std::string(objectName.size() - 1, '~');
        }
        ss << "\n";
    } else {
        if (line > 0) ss << "Avertissement ligne " << line << " : ";
        else ss << "Avertissement : ";
    }
    ss << msg;
    diag.formatted = ss.str();
    diagnostics_.push_back(diag);
}

std::vector<SemanticDiagnostic> SemanticAnalyzer::analyze(const Program& prog) {
    diagnostics_.clear();
    scopes_.clear();
    typeMap_.clear();
    procMap_.clear();
    funcMap_.clear();
    assignedVars_.clear();
    warnedUnassignedVars_.clear();
    constNames_.clear();
    enumValueToType_.clear();

    pushScope(); // Global scope

    collectTypes(prog);
    collectGlobalDeclarations(prog);

    for (const auto& proc : prog.procedures) {
        checkProcedure(proc);
    }
    for (const auto& func : prog.functions) {
        checkFunction(func);
    }

    checkBlock(prog.mainBody);

    // Détection des variables déclarées dans le TDO mais jamais utilisées
    for (const auto& vd : prog.varDecls) {
        std::string vt = normalize(vd.varType);
        if (vt == "procedure" || vt == "fonction" || normalize(vd.typeSpec.name) == "procedure" || normalize(vd.typeSpec.name) == "fonction") {
            continue;
        }
        for (const auto& name : vd.names) {
            std::string norm = normalize(name);
            auto it = scopes_[0].find(norm);
            if (it != scopes_[0].end() && !it->second.isUsed) {
                reportWarning(vd.line, -1, name,
                              "La variable '" + name + "' est déclarée dans le TDO mais n'est jamais utilisée.");
            }
        }
    }

    for (const auto& d : diagnostics_) {
        if (d.isError) {
            throw AlgoError(d.formatted, d.line);
        }
    }

    return diagnostics_;
}

void SemanticAnalyzer::collectTypes(const Program& prog) {
    for (const auto& td : prog.typeDecls) {
        typeMap_[normalize(td.name)] = td;
        declareSymbol(td.name, "type", td.typeSpec, SymbolKind::Type, -1);
        if (td.typeSpec.kind == TypeKind::Enum) {
            for (const auto& ev : td.typeSpec.enumValues) {
                declareSymbol(ev, td.name, td.typeSpec, SymbolKind::Constant, td.line);
                constNames_.insert(normalize(ev));
                enumValueToType_[normalize(ev)] = td.name;
                assignedVars_.insert(normalize(ev));
            }
        }
    }
}

void SemanticAnalyzer::collectGlobalDeclarations(const Program& prog) {
    for (const auto& cd : prog.constDecls) {
        checkExpr(cd.valueExpr);
        declareSymbol(cd.name, "constante", TypeSpec(), SymbolKind::Constant, cd.line);
        constNames_.insert(normalize(cd.name));
        assignedVars_.insert(normalize(cd.name));
    }
    for (const auto& proc : prog.procedures) {
        procMap_[normalize(proc.name)] = proc;
        declareSymbol(proc.name, "procedure", TypeSpec(), SymbolKind::Procedure, proc.line);
    }
    for (const auto& func : prog.functions) {
        funcMap_[normalize(func.name)] = func;
        declareSymbol(func.name, func.returnType, TypeSpec(), SymbolKind::Function, func.line);
    }
    for (const auto& vd : prog.varDecls) {
        std::string vt = normalize(vd.varType);
        if (vt == "procedure" || vt == "fonction") continue;
        for (const auto& name : vd.names) {
            declareSymbol(name, vd.varType, vd.typeSpec, SymbolKind::Variable, vd.line);
        }
    }
}

void SemanticAnalyzer::checkProcedure(const ProcedureDecl& proc) {
    pushScope();
    for (const auto& cd : proc.constDecls) {
        checkExpr(cd.valueExpr);
        declareSymbol(cd.name, "constante", TypeSpec(), SymbolKind::Constant, cd.line);
        constNames_.insert(normalize(cd.name));
        assignedVars_.insert(normalize(cd.name));
    }
    for (const auto& p : proc.params) {
        TypeSpec ts;
        ts.name = p.paramType;
        assignedVars_.insert(normalize(p.name));
        if (!declareSymbol(p.name, p.paramType, ts, SymbolKind::Variable, proc.line)) {
            reportError(proc.line > 0 ? proc.line : 1, 1, p.name,
                        "Le paramètre '" + p.name + "' est déclaré plus d'une fois dans l'en-tête de la procédure '" + proc.name + "'.");
        }
    }
    for (const auto& vd : proc.localVars) {
        std::string vt = normalize(vd.varType);
        if (vt == "procedure" || vt == "fonction") continue;
        for (const auto& name : vd.names) {
            bool isParam = false;
            for (const auto& p : proc.params) {
                if (normalize(p.name) == normalize(name)) {
                    isParam = true;
                    break;
                }
            }
            if (isParam) {
                reportError(vd.line > 0 ? vd.line : proc.line, 1, name,
                            "Le paramètre '" + name + "' est déjà déclaré dans l'en-tête de la procédure '" + proc.name + "' et ne doit pas être redéclaré dans la TDOL.");
            } else {
                declareSymbol(name, vd.varType, vd.typeSpec, SymbolKind::Variable, vd.line);
            }
        }
    }

    checkBlock(proc.body);
    popScope();
}

void SemanticAnalyzer::checkFunction(const FunctionDecl& func) {
    pushScope();
    currentFunctionName_ = func.name;
    currentFunctionReturnType_ = func.returnType;
    hasReturnInCurrentFunction_ = false;

    for (const auto& cd : func.constDecls) {
        checkExpr(cd.valueExpr);
        declareSymbol(cd.name, "constante", TypeSpec(), SymbolKind::Constant, cd.line);
        constNames_.insert(normalize(cd.name));
        assignedVars_.insert(normalize(cd.name));
    }
    for (const auto& p : func.params) {
        TypeSpec ts;
        ts.name = p.paramType;
        assignedVars_.insert(normalize(p.name));
        if (!declareSymbol(p.name, p.paramType, ts, SymbolKind::Variable, func.line)) {
            reportError(func.line > 0 ? func.line : 1, 1, p.name,
                        "Le paramètre '" + p.name + "' est déclaré plus d'une fois dans l'en-tête de la fonction '" + func.name + "'.");
        }
    }
    for (const auto& vd : func.localVars) {
        std::string vt = normalize(vd.varType);
        if (vt == "procedure" || vt == "fonction") continue;
        for (const auto& name : vd.names) {
            bool isParam = false;
            for (const auto& p : func.params) {
                if (normalize(p.name) == normalize(name)) {
                    isParam = true;
                    break;
                }
            }
            if (isParam) {
                reportError(vd.line > 0 ? vd.line : func.line, 1, name,
                            "Le paramètre '" + name + "' est déjà déclaré dans l'en-tête de la fonction '" + func.name + "' et ne doit pas être redéclaré dans la TDOL.");
            } else {
                declareSymbol(name, vd.varType, vd.typeSpec, SymbolKind::Variable, vd.line);
            }
        }
    }

    checkBlock(func.body);

    if (!hasReturnInCurrentFunction_) {
        reportError(func.line > 0 ? func.line : 1, 1, func.name,
                    "La fonction '" + func.name + "' doit comporter au moins une instruction de retour ('Retourner').");
    }

    currentFunctionName_.clear();
    currentFunctionReturnType_.clear();
    hasReturnInCurrentFunction_ = false;
    popScope();
}

void SemanticAnalyzer::checkBlock(const std::vector<std::shared_ptr<Stmt>>& stmts) {
    bool foundReturn = false;
    int deadCodeLine = -1;
    for (const auto& s : stmts) {
        if (!s) continue;
        if (foundReturn && deadCodeLine == -1) {
            deadCodeLine = s->line;
        }
        checkStmt(s);
        if (std::dynamic_pointer_cast<ReturnStmt>(s)) {
            foundReturn = true;
        }
    }
    if (foundReturn && deadCodeLine != -1) {
        reportWarning(deadCodeLine, -1, "Retourner",
                      "Code mort détecté : instruction(s) inaccessible(s) après 'Retourner'.");
    }
}

static std::string resolveElemType(const Symbol* sym, const std::map<std::string, TypeDecl>& typeMap) {
    if (!sym) return "inconnu";
    if (!sym->typeSpec.elemType.empty()) return sym->typeSpec.elemType;
    auto it = typeMap.find(normalize(sym->typeName));
    if (it != typeMap.end()) {
        if (!it->second.typeSpec.elemType.empty()) {
            return it->second.typeSpec.elemType;
        }
    }
    std::string tn = normalize(sym->typeName);
    if (tn == "chaine" || tn == "caractere") return "caractere";
    return sym->typeName;
}

SemanticAnalyzer::ExprTypeInfo SemanticAnalyzer::resolveExprTypeInfo(const std::shared_ptr<Expr>& expr) {
    ExprTypeInfo info;
    info.typeName = "inconnu";
    if (!expr) return info;

    if (std::dynamic_pointer_cast<NumberExpr>(expr)) {
        info.typeName = "entier";
        return info;
    }
    if (std::dynamic_pointer_cast<RealExpr>(expr)) {
        info.typeName = "reel";
        return info;
    }
    if (std::dynamic_pointer_cast<StringExpr>(expr)) {
        info.typeName = "chaine";
        return info;
    }
    if (std::dynamic_pointer_cast<BoolExpr>(expr)) {
        info.typeName = "booleen";
        return info;
    }
    if (auto v = std::dynamic_pointer_cast<VariableExpr>(expr)) {
        const Symbol* sym = findSymbol(v->name);
        if (sym) {
            info.typeName = sym->typeName;
            info.typeSpec = sym->typeSpec;
            if (info.typeSpec.kind != TypeKind::Array1D && info.typeSpec.kind != TypeKind::Array2D &&
                info.typeSpec.fields.empty() && !info.typeName.empty()) {
                auto it = typeMap_.find(normalize(info.typeName));
                if (it != typeMap_.end()) {
                    info.typeSpec = it->second.typeSpec;
                }
            }
        }
        return info;
    }
    if (auto mem = std::dynamic_pointer_cast<MemberAccessExpr>(expr)) {
        ExprTypeInfo objInfo = resolveExprTypeInfo(mem->object);
        const std::vector<std::shared_ptr<VarDecl>>* fields = nullptr;
        if (!objInfo.typeSpec.fields.empty()) {
            fields = &objInfo.typeSpec.fields;
        } else if (!objInfo.typeName.empty()) {
            auto it = typeMap_.find(normalize(objInfo.typeName));
            if (it != typeMap_.end() && !it->second.typeSpec.fields.empty()) {
                fields = &it->second.typeSpec.fields;
            }
        }
        if (fields) {
            for (const auto& fld : *fields) {
                for (const auto& fname : fld->names) {
                    if (normalize(fname) == normalize(mem->member)) {
                        info.typeName = fld->varType;
                        info.typeSpec = fld->typeSpec;
                        if (info.typeSpec.fields.empty() && !info.typeName.empty()) {
                            auto itF = typeMap_.find(normalize(info.typeName));
                            if (itF != typeMap_.end()) {
                                info.typeSpec = itF->second.typeSpec;
                            }
                        }
                        return info;
                    }
                }
            }
        }
        return info;
    }
    if (auto aa = std::dynamic_pointer_cast<ArrayAccessExpr>(expr)) {
        ExprTypeInfo arrInfo;
        if (aa->target) {
            arrInfo = resolveExprTypeInfo(aa->target);
        } else if (!aa->name.empty()) {
            const Symbol* sym = findSymbol(aa->name);
            if (sym) {
                arrInfo.typeName = sym->typeName;
                arrInfo.typeSpec = sym->typeSpec;
            }
        }
        if (arrInfo.typeSpec.kind != TypeKind::Array1D && arrInfo.typeSpec.kind != TypeKind::Array2D &&
            arrInfo.typeSpec.fields.empty() && !arrInfo.typeName.empty()) {
            auto it = typeMap_.find(normalize(arrInfo.typeName));
            if (it != typeMap_.end()) {
                arrInfo.typeSpec = it->second.typeSpec;
            }
        }
        std::string elemType = arrInfo.typeSpec.elemType;
        if (elemType.empty() && !arrInfo.typeName.empty()) {
            auto it = typeMap_.find(normalize(arrInfo.typeName));
            if (it != typeMap_.end()) {
                elemType = it->second.elemType.empty() ? it->second.typeSpec.elemType : it->second.elemType;
            }
        }
        if (elemType.empty() && (normalize(arrInfo.typeName) == "chaine" || normalize(arrInfo.typeName) == "caractere")) {
            elemType = "caractere";
        }
        info.typeName = elemType.empty() ? "inconnu" : elemType;
        auto itElem = typeMap_.find(normalize(info.typeName));
        if (itElem != typeMap_.end()) {
            info.typeSpec = itElem->second.typeSpec;
        }
        return info;
    }

    info.typeName = checkExpr(expr);
    return info;
}

static void collectReadVars(const std::shared_ptr<Expr>& expr, std::set<std::string>& out) {
    if (!expr) return;
    if (auto v = std::dynamic_pointer_cast<VariableExpr>(expr)) {
        out.insert(normalize(v->name));
        return;
    }
    if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
        collectReadVars(bin->left, out);
        collectReadVars(bin->right, out);
        return;
    }
    if (auto un = std::dynamic_pointer_cast<UnaryExpr>(expr)) {
        collectReadVars(un->operand, out);
        return;
    }
    if (auto aa = std::dynamic_pointer_cast<ArrayAccessExpr>(expr)) {
        if (!aa->name.empty()) out.insert(normalize(aa->name));
        if (aa->target) collectReadVars(aa->target, out);
        collectReadVars(aa->index, out);
        if (aa->index2) collectReadVars(aa->index2, out);
        return;
    }
    if (auto mem = std::dynamic_pointer_cast<MemberAccessExpr>(expr)) {
        collectReadVars(mem->object, out);
        return;
    }
    if (auto call = std::dynamic_pointer_cast<CallExpr>(expr)) {
        for (const auto& a : call->args) collectReadVars(a, out);
        return;
    }
    if (auto setLit = std::dynamic_pointer_cast<SetLiteralExpr>(expr)) {
        for (const auto& e : setLit->elements) collectReadVars(e, out);
        return;
    }
}

static void collectModifiedVars(const std::vector<std::shared_ptr<Stmt>>& stmts, std::set<std::string>& out) {
    for (const auto& stmt : stmts) {
        if (!stmt) continue;
        if (auto as = std::dynamic_pointer_cast<AssignStmt>(stmt)) {
            out.insert(normalize(as->target));
        } else if (auto aas = std::dynamic_pointer_cast<ArrayAssignStmt>(stmt)) {
            if (!aas->target.empty()) out.insert(normalize(aas->target));
            if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(aas->targetExpr)) {
                out.insert(normalize(varExpr->name));
            }
        } else if (auto ma = std::dynamic_pointer_cast<MemberAssignStmt>(stmt)) {
            if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(ma->target)) {
                out.insert(normalize(varExpr->name));
            }
        } else if (auto rd = std::dynamic_pointer_cast<ReadStmt>(stmt)) {
            for (const auto& tgt : rd->targets) {
                if (!tgt.name.empty()) out.insert(normalize(tgt.name));
                if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(tgt.targetExpr)) {
                    out.insert(normalize(varExpr->name));
                }
            }
        } else if (auto fors = std::dynamic_pointer_cast<ForStmt>(stmt)) {
            out.insert(normalize(fors->varName));
            collectModifiedVars(fors->body, out);
        } else if (auto ifs = std::dynamic_pointer_cast<IfStmt>(stmt)) {
            collectModifiedVars(ifs->thenBlock, out);
            for (const auto& eb : ifs->elseIfBlocks) collectModifiedVars(eb.body, out);
            collectModifiedVars(ifs->elseBlock, out);
        } else if (auto wh = std::dynamic_pointer_cast<WhileStmt>(stmt)) {
            collectModifiedVars(wh->body, out);
        } else if (auto rep = std::dynamic_pointer_cast<RepeatStmt>(stmt)) {
            collectModifiedVars(rep->body, out);
        } else if (auto call = std::dynamic_pointer_cast<CallStmt>(stmt)) {
            std::string n = normalize(call->name);
            if (n == "lire" || n == "lire_ligne" || n == "inserer") {
                for (const auto& a : call->args) {
                    if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(a)) {
                        out.insert(normalize(varExpr->name));
                    }
                }
            }
        }
    }
}

void SemanticAnalyzer::checkStmt(const std::shared_ptr<Stmt>& stmt) {
    if (!stmt) return;

    if (auto as = std::dynamic_pointer_cast<AssignStmt>(stmt)) {
        if (constNames_.find(normalize(as->target)) != constNames_.end()) {
            reportError(as->line, -1, as->target,
                        "Impossible de modifier la constante '" + as->target + "'.");
            return;
        }
        const Symbol* sym = findSymbol(as->target);
        std::string valType = checkExpr(as->value);
        if (!sym) {
            TypeSpec ts;
            declareSymbol(as->target, valType, ts, SymbolKind::Variable, as->line);
            sym = findSymbol(as->target);
        } else if (sym->typeName != "inconnu" && !areTypesCompatible(sym->typeName, valType)) {
            reportError(as->line, -1, as->target,
                        "Affectation de type incompatible pour l'objet '" + as->target + "' : attendu '" +
                        sym->typeName + "', mais obtenu '" + valType + "'.");
        }
        assignedVars_.insert(normalize(as->target));
        return;
    }

    if (auto aas = std::dynamic_pointer_cast<ArrayAssignStmt>(stmt)) {
        std::string elemType = "inconnu";
        std::string arrName = aas->target;
        if (aas->targetExpr) {
            if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(aas->targetExpr)) {
                arrName = varExpr->name;
            }
        }

        if (constNames_.find(normalize(arrName)) != constNames_.end()) {
            reportError(aas->line, -1, arrName,
                        "Impossible de modifier la constante '" + arrName + "'.");
            return;
        }

        ExprTypeInfo arrInfo;
        if (aas->targetExpr && !std::dynamic_pointer_cast<VariableExpr>(aas->targetExpr)) {
            arrInfo = resolveExprTypeInfo(aas->targetExpr);
            checkExpr(aas->targetExpr);
            elemType = arrInfo.typeSpec.elemType;
            if (elemType.empty() && !arrInfo.typeName.empty()) {
                auto it = typeMap_.find(normalize(arrInfo.typeName));
                if (it != typeMap_.end()) {
                    elemType = it->second.elemType.empty() ? it->second.typeSpec.elemType : it->second.elemType;
                }
            }
            if (elemType.empty() && (normalize(arrInfo.typeName) == "chaine" || normalize(arrInfo.typeName) == "caractere")) {
                elemType = "caractere";
            }
            if (arrName.empty()) arrName = "tableau";
        } else {
            const Symbol* sym = findSymbol(arrName);
            if (!sym) {
                std::string sugg = findClosestMatch(arrName, getVisibleSymbolNames());
                reportError(aas->line, -1, arrName,
                            "Le tableau '" + arrName + "' non défini dans le TDO/TDOL.", sugg);
                return;
            }
            arrInfo.typeName = sym->typeName;
            arrInfo.typeSpec = sym->typeSpec;
            elemType = resolveElemType(sym, typeMap_);
        }

        if (arrInfo.typeSpec.rows > 0) {
            if (auto num = std::dynamic_pointer_cast<NumberExpr>(aas->index)) {
                if (num->value < 0 || num->value >= arrInfo.typeSpec.rows) {
                    reportError(aas->line, -1, arrName,
                                "Indice " + std::to_string(num->value) + " hors bornes pour le tableau '" +
                                arrName + "' de taille " + std::to_string(arrInfo.typeSpec.rows) + ".");
                }
            }
        }

        std::string idxType = checkExpr(aas->index);
        if (!isNumericType(idxType) || idxType == "reel") {
            reportError(aas->line, -1, arrName,
                        "L'indice pour le tableau '" + arrName + "' doit être un entier (obtenu '" + idxType + "').");
        }
        if (aas->index2) {
            std::string idx2Type = checkExpr(aas->index2);
            if (!isNumericType(idx2Type) || idx2Type == "reel") {
                reportError(aas->line, -1, arrName,
                            "Le deuxième indice de la matrice '" + arrName + "' doit être un entier (obtenu '" + idx2Type + "').");
            }
        }
        std::string valType = checkExpr(aas->value);
        if (!areTypesCompatible(elemType, valType)) {
            reportError(aas->line, -1, arrName,
                        "Affectation de type incompatible dans le tableau '" + arrName + "' : attendu '" +
                        elemType + "', mais obtenu '" + valType + "'.");
        }
        assignedVars_.insert(normalize(arrName));
        return;
    }

    if (auto ma = std::dynamic_pointer_cast<MemberAssignStmt>(stmt)) {
        ExprTypeInfo objInfo = resolveExprTypeInfo(ma->target);
        checkExpr(ma->target);
        const std::vector<std::shared_ptr<VarDecl>>* fields = nullptr;
        if (!objInfo.typeSpec.fields.empty()) {
            fields = &objInfo.typeSpec.fields;
        } else if (!objInfo.typeName.empty()) {
            auto it = typeMap_.find(normalize(objInfo.typeName));
            if (it != typeMap_.end() && !it->second.typeSpec.fields.empty()) {
                fields = &it->second.typeSpec.fields;
            }
        }

        if (!fields) {
            reportError(ma->line, -1, ma->member,
                        "L'objet ciblé n'est pas de type enregistrement (type '" + objInfo.typeName + "').");
            return;
        }
        bool fieldFound = false;
        std::string expectedFieldType;
        std::vector<std::string> fieldCandidates;
        for (const auto& fld : *fields) {
            for (const auto& fname : fld->names) {
                fieldCandidates.push_back(fname);
                if (normalize(fname) == normalize(ma->member)) {
                    fieldFound = true;
                    expectedFieldType = fld->varType;
                    break;
                }
            }
            if (fieldFound) break;
        }
        if (!fieldFound) {
            std::string sugg = findClosestMatch(ma->member, fieldCandidates);
            reportError(ma->line, -1, ma->member,
                        "Le champ '" + ma->member + "' n'est pas défini dans l'enregistrement '" + objInfo.typeName + "'.", sugg);
            return;
        }
        std::string valType = checkExpr(ma->value);
        if (!areTypesCompatible(expectedFieldType, valType)) {
            reportError(ma->line, -1, ma->member,
                        "Affectation de type incompatible pour le champ '" + ma->member + "' : attendu '" +
                        expectedFieldType + "', mais obtenu '" + valType + "'.");
        }
        if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(ma->target)) {
            assignedVars_.insert(normalize(varExpr->name));
        }
        return;
    }

    if (auto r = std::dynamic_pointer_cast<ReturnStmt>(stmt)) {
        if (currentFunctionName_.empty()) {
            reportError(r->line, -1, "Retourner", "L'instruction 'Retourner' n'est autorisée qu'à l'intérieur d'une fonction.");
            return;
        }
        hasReturnInCurrentFunction_ = true;
        if (r->value) {
            std::string retType = checkExpr(r->value);
            if (!areTypesCompatible(currentFunctionReturnType_, retType)) {
                reportError(r->line, -1, currentFunctionName_,
                            "Valeur de retour incompatible pour la fonction '" + currentFunctionName_ + "' : attendu '" +
                            currentFunctionReturnType_ + "', mais obtenu '" + retType + "'.");
            }
        }
        return;
    }

    if (auto call = std::dynamic_pointer_cast<CallStmt>(stmt)) {
        std::string n = normalize(call->name);
        if (n == "inserer") {
            reportError(call->line, -1, call->name,
                        "'Inserer' est une fonction et ne peut pas être appelée en tant que procédure.");
            return;
        }

        if (n == "lire" || n == "ecrire" || n == "ecrire_nl" || n == "ouvrir" || n == "fermer" ||
            n == "lire_ligne" || n == "graine" || n == "effacer" ||
            n == "aller_a" || n == "pointer") {
            for (const auto& a : call->args) checkExpr(a);
            return;
        }

        auto itProc = procMap_.find(n);
        if (itProc == procMap_.end()) {
            std::string sugg = findClosestMatch(call->name, getVisibleSymbolNames());
            reportError(call->line, -1, call->name,
                        "La procédure '" + call->name + "' non définie dans le TDO/TDOG.", sugg);
            return;
        }

        const auto& proc = itProc->second;
        if (call->args.size() != proc.params.size()) {
            reportError(call->line, -1, call->name,
                        "Nombre d'arguments invalide pour l'appel de la procédure '" + call->name +
                        "' : attendu " + std::to_string(proc.params.size()) + ", mais obtenu " +
                        std::to_string(call->args.size()) + ".");
            return;
        }

        for (size_t i = 0; i < proc.params.size(); i++) {
            const auto& param = proc.params[i];
            const auto& arg = call->args[i];
            if (param.byRef && !isLValue(arg)) {
                reportError(call->line, -1, param.name,
                            "L'argument passé au paramètre par référence '@" + param.name +
                            "' de la procédure '" + call->name + "' doit être une variable modifiable (LValue).");
            }
            if (param.byRef) {
                if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(arg)) {
                    if (constNames_.find(normalize(varExpr->name)) != constNames_.end()) {
                        reportError(call->line, -1, varExpr->name,
                                    "Impossible de passer la constante '" + varExpr->name + "' à un paramètre modifiable.");
                    }
                    assignedVars_.insert(normalize(varExpr->name));
                }
            }
            std::string argType = checkExpr(arg);
            if (!areTypesCompatible(param.paramType, argType)) {
                reportError(call->line, -1, param.name,
                            "Type d'argument incompatible pour le paramètre '" + param.name + "' de la procédure '" +
                            call->name + "' : attendu '" + param.paramType + "', mais obtenu '" + argType + "'.");
            }
        }
        return;
    }

    if (auto ifs = std::dynamic_pointer_cast<IfStmt>(stmt)) {
        std::string condType = checkExpr(ifs->condition);
        if (!isBooleanType(condType)) {
            reportError(ifs->line, -1, "Si",
                        "La condition de la structure 'Si' doit être de type booléen (obtenu '" + condType + "').");
        }
        checkBlock(ifs->thenBlock);
        for (const auto& eb : ifs->elseIfBlocks) {
            std::string ebCondType = checkExpr(eb.condition);
            if (!isBooleanType(ebCondType)) {
                reportError(ifs->line, -1, "Sinon Si",
                            "La condition de 'Sinon Si' doit être de type booléen (obtenu '" + ebCondType + "').");
            }
            checkBlock(eb.body);
        }
        checkBlock(ifs->elseBlock);
        return;
    }

    if (auto fors = std::dynamic_pointer_cast<ForStmt>(stmt)) {
        const Symbol* sym = findSymbol(fors->varName);
        if (!sym) {
            // Implicit integer loop counter (standard convention in Tunisian algorithms)
            TypeSpec ts;
            declareSymbol(fors->varName, "entier", ts, SymbolKind::Variable, fors->line);
        } else if (!isNumericType(sym->typeName)) {
            reportError(fors->line, -1, fors->varName,
                        "La variable de boucle '" + fors->varName + "' doit être un nombre de type numérique (obtenu '" + sym->typeName + "').");
        }
        assignedVars_.insert(normalize(fors->varName));

        std::string startType = checkExpr(fors->start);
        std::string endType = checkExpr(fors->end);
        if (!isNumericType(startType) || !isNumericType(endType)) {
            reportError(fors->line, -1, "Pour",
                        "Les bornes de la boucle 'Pour' doivent être des nombres (types numériques attendus, obtenu '" +
                        startType + "' à '" + endType + "').");
        }
        if (fors->step) {
            std::string stepType = checkExpr(fors->step);
            if (!isNumericType(stepType)) {
                reportError(fors->line, -1, "Pour",
                            "Le pas de la boucle 'Pour' doit être parmi les nombres (type numérique attendu, obtenu '" + stepType + "').");
            }
        }
        checkBlock(fors->body);
        return;
    }

    if (auto wh = std::dynamic_pointer_cast<WhileStmt>(stmt)) {
        std::string condType = checkExpr(wh->condition);
        if (!isBooleanType(condType)) {
            reportError(wh->line, -1, "Tant Que",
                        "La condition de la boucle 'Tant Que' doit être de type booléen (obtenu '" + condType + "').");
        }

        std::set<std::string> condVars;
        collectReadVars(wh->condition, condVars);
        std::set<std::string> modVars;
        collectModifiedVars(wh->body, modVars);
        bool hasMod = false;
        for (const auto& cv : condVars) {
            if (modVars.find(cv) != modVars.end()) {
                hasMod = true;
                break;
            }
        }
        if (!condVars.empty() && !hasMod) {
            reportWarning(wh->line, -1, "Tant Que",
                          "Boucle potentiellement infinie : aucune variable de la condition n'est modifiée dans le corps de la boucle.");
        }

        checkBlock(wh->body);
        return;
    }

    if (auto rep = std::dynamic_pointer_cast<RepeatStmt>(stmt)) {
        checkBlock(rep->body);
        std::string condType = checkExpr(rep->condition);
        if (!isBooleanType(condType)) {
            reportError(rep->line, -1, "Jusqu'à",
                        "La condition de 'Jusqu'à' doit être de type booléen (obtenu '" + condType + "').");
        }
        return;
    }

    if (auto sw = std::dynamic_pointer_cast<SwitchStmt>(stmt)) {
        std::string selType = checkExpr(sw->expr);
        for (const auto& cs : sw->cases) {
            for (const auto& item : cs.items) {
                if (item.start) {
                    std::string t = checkExpr(item.start);
                    if (!areTypesCompatible(selType, t)) {
                        reportError(sw->line, -1, "Selon",
                                    "Valeur de cas incompatible dans 'Selon' : attendu '" + selType + "', obtenu '" + t + "'.");
                    }
                }
                if (item.end) {
                    std::string t = checkExpr(item.end);
                    if (!areTypesCompatible(selType, t)) {
                        reportError(sw->line, -1, "Selon",
                                    "Borne de cas incompatible dans 'Selon' : attendu '" + selType + "', obtenu '" + t + "'.");
                    }
                }
            }
            checkBlock(cs.body);
        }
        return;
    }

    if (auto w = std::dynamic_pointer_cast<WriteStmt>(stmt)) {
        for (const auto& a : w->args) {
            checkExpr(a);
        }
        return;
    }

    if (auto rd = std::dynamic_pointer_cast<ReadStmt>(stmt)) {
        for (const auto& tgt : rd->targets) {
            if (!tgt.name.empty()) {
                if (constNames_.find(normalize(tgt.name)) != constNames_.end()) {
                    reportError(rd->line, -1, tgt.name, "Impossible de modifier la constante '" + tgt.name + "'.");
                }
                assignedVars_.insert(normalize(tgt.name));
            }
            if (tgt.targetExpr) {
                if (!isLValue(tgt.targetExpr)) {
                    reportError(rd->line, -1, "Lire", "L'instruction 'Lire' attend une variable ou un emplacement modifiable.");
                }
                if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(tgt.targetExpr)) {
                    if (constNames_.find(normalize(varExpr->name)) != constNames_.end()) {
                        reportError(rd->line, -1, varExpr->name, "Impossible de modifier la constante '" + varExpr->name + "'.");
                    }
                    assignedVars_.insert(normalize(varExpr->name));
                }
                checkExpr(tgt.targetExpr);
            }
        }
        return;
    }
}

std::string SemanticAnalyzer::checkExpr(const std::shared_ptr<Expr>& expr) {
    if (!expr) return "inconnu";

    if (std::dynamic_pointer_cast<NumberExpr>(expr)) {
        return "entier";
    }
    if (std::dynamic_pointer_cast<RealExpr>(expr)) {
        return "reel";
    }
    if (std::dynamic_pointer_cast<StringExpr>(expr)) {
        return "chaine";
    }
    if (std::dynamic_pointer_cast<BoolExpr>(expr)) {
        return "booleen";
    }
    if (auto v = std::dynamic_pointer_cast<VariableExpr>(expr)) {
        const Symbol* sym = findSymbol(v->name);
        if (!sym) {
            std::string sugg = findClosestMatch(v->name, getVisibleSymbolNames());
            reportError(v->line, -1, v->name,
                        "La variable '" + v->name + "' non définie (objet non déclaré dans le TDO/TDOL).", sugg);
            return "inconnu";
        }
        if (sym->kind == SymbolKind::Variable) {
            std::string vNorm = normalize(v->name);
            if (assignedVars_.find(vNorm) == assignedVars_.end() &&
                warnedUnassignedVars_.find(vNorm) == warnedUnassignedVars_.end()) {
                warnedUnassignedVars_.insert(vNorm);
                reportWarning(v->line, -1, v->name,
                              "La variable '" + v->name + "' est utilisée sans avoir été préalablement initialisée.");
            }
        }
        return sym->typeName;
    }
    if (auto mem = std::dynamic_pointer_cast<MemberAccessExpr>(expr)) {
        ExprTypeInfo objInfo = resolveExprTypeInfo(mem->object);
        checkExpr(mem->object);
        const std::vector<std::shared_ptr<VarDecl>>* fields = nullptr;
        if (!objInfo.typeSpec.fields.empty()) {
            fields = &objInfo.typeSpec.fields;
        } else if (!objInfo.typeName.empty()) {
            auto it = typeMap_.find(normalize(objInfo.typeName));
            if (it != typeMap_.end() && !it->second.typeSpec.fields.empty()) {
                fields = &it->second.typeSpec.fields;
            }
        }
        if (fields) {
            std::vector<std::string> fieldCandidates;
            for (const auto& fld : *fields) {
                for (const auto& fname : fld->names) {
                    fieldCandidates.push_back(fname);
                    if (normalize(fname) == normalize(mem->member)) {
                        return fld->varType;
                    }
                }
            }
            std::string sugg = findClosestMatch(mem->member, fieldCandidates);
            reportError(mem->line, -1, mem->member,
                        "Le champ '" + mem->member + "' n'est pas défini dans l'enregistrement '" + objInfo.typeName + "'.", sugg);
            return "inconnu";
        }
        return "inconnu";
    }
    if (auto aa = std::dynamic_pointer_cast<ArrayAccessExpr>(expr)) {
        std::string elemType = "inconnu";
        std::string arrName = aa->name;
        if (aa->target) {
            if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(aa->target)) {
                arrName = varExpr->name;
            }
        }

        ExprTypeInfo arrInfo;
        if (aa->target && !std::dynamic_pointer_cast<VariableExpr>(aa->target)) {
            arrInfo = resolveExprTypeInfo(aa->target);
            checkExpr(aa->target);
            elemType = arrInfo.typeSpec.elemType;
            if (elemType.empty() && !arrInfo.typeName.empty()) {
                auto it = typeMap_.find(normalize(arrInfo.typeName));
                if (it != typeMap_.end()) {
                    elemType = it->second.elemType.empty() ? it->second.typeSpec.elemType : it->second.elemType;
                }
            }
            if (elemType.empty() && (normalize(arrInfo.typeName) == "chaine" || normalize(arrInfo.typeName) == "caractere")) {
                elemType = "caractere";
            }
            if (arrName.empty()) arrName = "tableau";
        } else {
            const Symbol* sym = findSymbol(arrName);
            if (!sym) {
                std::string sugg = findClosestMatch(arrName, getVisibleSymbolNames());
                reportError(aa->line, -1, arrName,
                            "Le tableau '" + arrName + "' non défini dans le TDO/TDOL.", sugg);
                return "inconnu";
            }
            arrInfo.typeName = sym->typeName;
            arrInfo.typeSpec = sym->typeSpec;
            elemType = resolveElemType(sym, typeMap_);
        }

        if (arrInfo.typeSpec.rows > 0) {
            if (auto num = std::dynamic_pointer_cast<NumberExpr>(aa->index)) {
                if (num->value < 0 || num->value >= arrInfo.typeSpec.rows) {
                    reportError(aa->line, -1, arrName,
                                "Indice " + std::to_string(num->value) + " hors bornes pour le tableau '" +
                                arrName + "' de taille " + std::to_string(arrInfo.typeSpec.rows) + ".");
                }
            }
        }

        std::string idxType = checkExpr(aa->index);
        if (!isNumericType(idxType) || idxType == "reel") {
            reportError(aa->line, -1, arrName,
                        "L'indice pour le tableau '" + arrName + "' doit être un entier (obtenu '" + idxType + "').");
        }
        if (aa->index2) {
            std::string idx2Type = checkExpr(aa->index2);
            if (!isNumericType(idx2Type) || idx2Type == "reel") {
                reportError(aa->line, -1, arrName,
                            "Le deuxième indice pour la matrice '" + arrName + "' doit être un entier (obtenu '" + idx2Type + "').");
            }
        }
        return elemType;
    }
    if (auto un = std::dynamic_pointer_cast<UnaryExpr>(expr)) {
        std::string op = normalize(un->op);
        std::string opType = checkExpr(un->operand);
        if (op == "non" || op == "not") {
            if (!isBooleanType(opType)) {
                reportError(un->line, -1, "Non", "L'opérateur 'Non' s'applique uniquement à une expression booléenne.");
            }
            return "booleen";
        }
        if (op == "-") {
            if (!isNumericType(opType)) {
                reportError(un->line, -1, "-", "L'opérateur '-' unaire s'applique uniquement à un type numérique.");
            }
            return opType;
        }
        return opType;
    }
    if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
        std::string lt = checkExpr(bin->left);
        std::string rt = checkExpr(bin->right);
        std::string op = normalize(bin->op);

        if (op == "et" || op == "ou" || op == "and" || op == "or") {
            if (!isBooleanType(lt) || !isBooleanType(rt)) {
                reportError(bin->line, -1, bin->op,
                            "Les opérandes de '" + bin->op + "' doivent être de type booléen.");
            }
            return "booleen";
        }
        if (op == "=" || op == "≠" || op == "!=" || op == "<" || op == "<=" || op == "≤" || op == ">" || op == ">=" || op == "≥") {
            return "booleen";
        }
        if (op == "∈" || op == "dans") {
            return "booleen";
        }
        if (op == "^" || op == "**") {
            if (!isNumericType(lt) || !isNumericType(rt)) {
                reportError(bin->line, -1, bin->op, "L'opérateur '" + bin->op + "' attend des opérandes numériques.");
            }
            if (lt == "reel" || rt == "reel") return "reel";
            return "entier";
        }
        if (op == "+") {
            if (isStringType(lt) || isStringType(rt)) {
                return "chaine";
            }
            if (lt == "reel" || rt == "reel") return "reel";
            return "entier";
        }
        if (op == "-" || op == "*") {
            if (lt == "reel" || rt == "reel") return "reel";
            return "entier";
        }
        if (op == "/") {
            return "reel";
        }
        if (op == "div" || op == "mod" || op == "%") {
            return "entier";
        }
        return lt;
    }
    if (auto setLit = std::dynamic_pointer_cast<SetLiteralExpr>(expr)) {
        for (const auto& e : setLit->elements) checkExpr(e);
        return "ensemble";
    }
    if (auto call = std::dynamic_pointer_cast<CallExpr>(expr)) {
        std::string n = normalize(call->name);

        auto itFunc = funcMap_.find(n);
        if (itFunc != funcMap_.end()) {
            const auto& func = itFunc->second;
            if (call->args.size() != func.params.size()) {
                reportError(call->line, -1, call->name,
                            "Nombre d'arguments invalide pour l'appel de la fonction '" + call->name +
                            "' : attendu " + std::to_string(func.params.size()) + ", mais obtenu " +
                            std::to_string(call->args.size()) + ".");
            }

            for (size_t i = 0; i < func.params.size() && i < call->args.size(); i++) {
                checkExpr(call->args[i]);
            }

            return func.returnType;
        }

        if (n == "sin" || n == "cos" || n == "tan" || n == "arctan" || n == "atan" || n == "frac") {
            for (const auto& a : call->args) checkExpr(a);
            return "reel";
        }
        if (n == "puissance") {
            for (const auto& a : call->args) checkExpr(a);
            return "reel";
        }
        if (n == "taille_fichier" || n == "taille" || n == "position_fichier" || n == "pointeur") {
            for (const auto& a : call->args) checkExpr(a);
            return "entier";
        }
        if (n == "inserer") {
            for (const auto& a : call->args) checkExpr(a);
            return "chaine";
        }

        if (n == "racine" || n == "exp" || n == "ln" || n == "log") {
            for (const auto& a : call->args) checkExpr(a);
            return "reel";
        }
        if (n == "carre" || n == "abs") {
            if (!call->args.empty()) return checkExpr(call->args[0]);
            return "reel";
        }
        if (n == "ent" || n == "arrondi" || n == "long" || n == "ord" || n == "pos" || n == "valeur" || n == "alea" || n == "graine") {
            for (const auto& a : call->args) checkExpr(a);
            return "entier";
        }
        if (n == "chr" || n == "convch" || n == "sous_chaine" || n == "souschaine" || n == "majus" || n == "minus" || n == "effacer") {
            for (const auto& a : call->args) checkExpr(a);
            return "chaine";
        }
        if (n == "fin_fichier" || n == "finfichier" || n == "estnum" || n == "est_num") {
            for (const auto& a : call->args) checkExpr(a);
            return "booleen";
        }

        std::string sugg = findClosestMatch(call->name, getVisibleSymbolNames());
        reportError(call->line, -1, call->name,
                    "La fonction '" + call->name + "' non définie dans le TDO/TDOG.", sugg);
        return "inconnu";
    }

    return "inconnu";
}

bool SemanticAnalyzer::areTypesCompatible(const std::string& targetType, const std::string& srcType) {
    std::string t = normalize(targetType);
    std::string s = normalize(srcType);
    if (t == "inconnu" || s == "inconnu") return true;

    if (enumValueToType_.count(s) && normalize(enumValueToType_[s]) == t) return true;
    if (enumValueToType_.count(t) && normalize(enumValueToType_[t]) == s) return true;

    // Resolve aliases and subranges
    auto itT = typeMap_.find(t);
    if (itT != typeMap_.end()) {
        if (itT->second.typeSpec.kind == TypeKind::Subrange) {
            t = "entier";
        } else if (itT->second.typeSpec.kind == TypeKind::Simple && !itT->second.typeSpec.name.empty()) {
            t = normalize(itT->second.typeSpec.name);
        }
    }
    auto itS = typeMap_.find(s);
    if (itS != typeMap_.end()) {
        if (itS->second.typeSpec.kind == TypeKind::Subrange) {
            s = "entier";
        } else if (itS->second.typeSpec.kind == TypeKind::Simple && !itS->second.typeSpec.name.empty()) {
            s = normalize(itS->second.typeSpec.name);
        }
    }

    if (t == s) return true;
    if (isNumericType(t) && isNumericType(s)) return true;
    if ((t == "chaine" || t == "caractere") && (s == "chaine" || s == "caractere")) return true;
    return false;
}

bool SemanticAnalyzer::isNumericType(const std::string& type) {
    std::string t = normalize(type);
    auto it = typeMap_.find(t);
    if (it != typeMap_.end()) {
        if (it->second.typeSpec.kind == TypeKind::Subrange) return true;
        if (it->second.typeSpec.kind == TypeKind::Simple) {
            t = normalize(it->second.typeSpec.name);
        }
    }
    return t == "entier" || t == "reel" || t == "inconnu";
}

bool SemanticAnalyzer::isBooleanType(const std::string& type) {
    std::string t = normalize(type);
    auto it = typeMap_.find(t);
    if (it != typeMap_.end() && it->second.typeSpec.kind == TypeKind::Simple) {
        t = normalize(it->second.typeSpec.name);
    }
    return t == "booleen" || t == "inconnu";
}

bool SemanticAnalyzer::isStringType(const std::string& type) {
    std::string t = normalize(type);
    auto it = typeMap_.find(t);
    if (it != typeMap_.end() && it->second.typeSpec.kind == TypeKind::Simple) {
        t = normalize(it->second.typeSpec.name);
    }
    return t == "chaine" || t == "caractere" || t == "inconnu";
}

bool SemanticAnalyzer::isLValue(const std::shared_ptr<Expr>& expr) {
    if (!expr) return false;
    if (std::dynamic_pointer_cast<VariableExpr>(expr)) return true;
    if (std::dynamic_pointer_cast<ArrayAccessExpr>(expr)) return true;
    if (std::dynamic_pointer_cast<MemberAccessExpr>(expr)) return true;
    return false;
}

} // namespace algo
