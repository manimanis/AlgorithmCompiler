#include "../include/python_transpiler.hpp"
#include <algorithm>
#include <iomanip>

namespace algo {

static std::string pySafeName(const std::string& name) {
    static const std::vector<std::string> pyKeywords = {
        "and", "as", "assert", "async", "await", "break", "class", "continue",
        "def", "del", "elif", "else", "except", "finally", "for", "from",
        "global", "if", "import", "in", "is", "lambda", "nonlocal", "not",
        "or", "pass", "raise", "return", "try", "while", "with", "yield"
    };
    std::string low = normalize(name);
    for (const auto& kw : pyKeywords) {
        if (low == kw) return name + "_";
    }
    return name;
}

PythonTranspiler::PythonTranspiler(const Program& prog) : prog_(prog) {
    collectTypes();
    collectNeeds();
}

void PythonTranspiler::collectTypes() {
    for (const auto& td : prog_.typeDecls) {
        typeMap_[normalize(td.name)] = td;
    }
    for (const auto& vd : prog_.varDecls) {
        for (const auto& name : vd.names) {
            if (vd.typeSpec.kind == TypeKind::Pointer) {
                varTypeMap_[normalize(name)] = vd.varType.empty() ? ("^" + vd.typeSpec.elemType) : vd.varType;
            } else {
                varTypeMap_[normalize(name)] = vd.typeSpec.elemType.empty() ? (vd.typeSpec.name.empty() ? vd.varType : vd.typeSpec.name) : vd.typeSpec.elemType;
            }
        }
    }
    for (const auto& proc : prog_.procedures) {
        procMap_[normalize(proc.name)] = proc;
    }
    for (const auto& func : prog_.functions) {
        funcMap_[normalize(func.name)] = func;
    }
}

void PythonTranspiler::scanTypeSpec(const TypeSpec& ts) {
    if (ts.kind == TypeKind::Array1D || ts.kind == TypeKind::Array2D) {
        needArray_ = true;
    }
}

void PythonTranspiler::scanExpr(const std::shared_ptr<Expr>& expr) {
    if (!expr) return;
    if (auto c = std::dynamic_pointer_cast<CallExpr>(expr)) {
        std::string n = normalize(c->name);
        if (n == "racine") mathImports_.insert("sqrt");
        else if (n == "exp") mathImports_.insert("exp");
        else if (n == "ln" || n == "log") mathImports_.insert("log");
        else if (n == "cos") mathImports_.insert("cos");
        else if (n == "sin") mathImports_.insert("sin");
        else if (n == "tan") mathImports_.insert("tan");
        else if (n == "arctan" || n == "atan") mathImports_.insert("atan");
        else if (n == "alea" || n == "alea") randomImports_.insert("randint");
        else if (n == "graine") randomImports_.insert("seed");
        else if (n == "fin_fichier" || n == "finfichier") osImports_.insert("fstat");
        for (const auto& a : c->args) scanExpr(a);
        return;
    }
    if (auto un = std::dynamic_pointer_cast<UnaryExpr>(expr)) {
        scanExpr(un->operand);
        return;
    }
    if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
        scanExpr(bin->left);
        scanExpr(bin->right);
        return;
    }
    if (auto aa = std::dynamic_pointer_cast<ArrayAccessExpr>(expr)) {
        if (aa->target) scanExpr(aa->target);
        scanExpr(aa->index);
        if (aa->index2) scanExpr(aa->index2);
        return;
    }
    if (auto mem = std::dynamic_pointer_cast<MemberAccessExpr>(expr)) {
        scanExpr(mem->object);
        return;
    }
    if (auto deref = std::dynamic_pointer_cast<DerefExpr>(expr)) {
        scanExpr(deref->target);
        return;
    }
    if (auto setLit = std::dynamic_pointer_cast<SetLiteralExpr>(expr)) {
        for (const auto& e : setLit->elements) scanExpr(e);
        return;
    }
}

void PythonTranspiler::scanStmt(const std::shared_ptr<Stmt>& stmt) {
    if (!stmt) return;
    if (auto alloc = std::dynamic_pointer_cast<AllocStmt>(stmt)) {
        scanExpr(alloc->target);
        return;
    }
    if (auto fr = std::dynamic_pointer_cast<FreeStmt>(stmt)) {
        scanExpr(fr->target);
        return;
    }
    if (auto da = std::dynamic_pointer_cast<DerefAssignStmt>(stmt)) {
        scanExpr(da->target);
        scanExpr(da->value);
        return;
    }
    if (auto as = std::dynamic_pointer_cast<AssignStmt>(stmt)) {
        scanExpr(as->value);
        return;
    }
    if (auto aas = std::dynamic_pointer_cast<ArrayAssignStmt>(stmt)) {
        if (aas->targetExpr) scanExpr(aas->targetExpr);
        scanExpr(aas->index);
        if (aas->index2) scanExpr(aas->index2);
        scanExpr(aas->value);
        return;
    }
    if (auto ma = std::dynamic_pointer_cast<MemberAssignStmt>(stmt)) {
        scanExpr(ma->target);
        scanExpr(ma->value);
        return;
    }
    if (auto r = std::dynamic_pointer_cast<ReturnStmt>(stmt)) {
        if (r->value) scanExpr(r->value);
        return;
    }
    if (auto w = std::dynamic_pointer_cast<WriteStmt>(stmt)) {
        if (!w->args.empty()) {
            if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(w->args[0])) {
                std::string vt = normalize(varTypeMap_[normalize(varExpr->name)]);
                if (vt.find("fichier") != std::string::npos && vt.find("texte") == std::string::npos) {
                    pickleImports_.insert("dump");
                }
            }
        }
        for (const auto& a : w->args) scanExpr(a);
        return;
    }
    if (auto rd = std::dynamic_pointer_cast<ReadStmt>(stmt)) {
        if (rd->targets.size() == 2) {
            std::string t0 = rd->targets[0].name;
            std::string vt = normalize(varTypeMap_[normalize(t0)]);
            if (vt.find("fichier") != std::string::npos && vt.find("texte") == std::string::npos) {
                pickleImports_.insert("load");
            }
        }
        for (const auto& t : rd->targets) {
            if (t.targetExpr) scanExpr(t.targetExpr);
        }
        return;
    }
    if (auto call = std::dynamic_pointer_cast<CallStmt>(stmt)) {
        std::string n = normalize(call->name);
        if (n == "fin_fichier" || n == "finfichier") osImports_.insert("fstat");
        for (const auto& a : call->args) scanExpr(a);
        return;
    }
    if (auto ifs = std::dynamic_pointer_cast<IfStmt>(stmt)) {
        scanExpr(ifs->condition);
        for (const auto& s : ifs->thenBlock) scanStmt(s);
        for (const auto& eb : ifs->elseIfBlocks) {
            scanExpr(eb.condition);
            for (const auto& s : eb.body) scanStmt(s);
        }
        for (const auto& s : ifs->elseBlock) scanStmt(s);
        return;
    }
    if (auto fors = std::dynamic_pointer_cast<ForStmt>(stmt)) {
        scanExpr(fors->start);
        scanExpr(fors->end);
        if (fors->step) scanExpr(fors->step);
        for (const auto& s : fors->body) scanStmt(s);
        return;
    }
    if (auto wh = std::dynamic_pointer_cast<WhileStmt>(stmt)) {
        scanExpr(wh->condition);
        for (const auto& s : wh->body) scanStmt(s);
        return;
    }
    if (auto rep = std::dynamic_pointer_cast<RepeatStmt>(stmt)) {
        for (const auto& s : rep->body) scanStmt(s);
        scanExpr(rep->condition);
        return;
    }
    if (auto sw = std::dynamic_pointer_cast<SwitchStmt>(stmt)) {
        scanExpr(sw->expr);
        for (const auto& cs : sw->cases) {
            for (const auto& item : cs.items) {
                if (item.start) scanExpr(item.start);
                if (item.end) scanExpr(item.end);
            }
            for (const auto& val : cs.values) scanExpr(val);
            for (const auto& s : cs.body) scanStmt(s);
        }
        return;
    }
}

void PythonTranspiler::collectNeeds() {
    needArray_ = false;
    mathImports_.clear();
    randomImports_.clear();
    pickleImports_.clear();
    osImports_.clear();

    for (const auto& td : prog_.typeDecls) {
        scanTypeSpec(td.typeSpec);
        for (const auto& fld : td.typeSpec.fields) {
            scanTypeSpec(fld->typeSpec);
        }
    }
    for (const auto& vd : prog_.varDecls) {
        scanTypeSpec(vd.typeSpec);
    }
    for (const auto& proc : prog_.procedures) {
        for (const auto& vd : proc.localVars) scanTypeSpec(vd.typeSpec);
        for (const auto& s : proc.body) scanStmt(s);
    }
    for (const auto& func : prog_.functions) {
        for (const auto& vd : func.localVars) scanTypeSpec(vd.typeSpec);
        for (const auto& s : func.body) scanStmt(s);
    }
    for (const auto& s : prog_.mainBody) {
        scanStmt(s);
    }
}

std::string PythonTranspiler::transpile() {
    std::ostringstream ss;
    ss << "# ========================================================\n";
    ss << "# Programme transpilé depuis Algorithme : " << prog_.name << "\n";
    ss << "# Transpilateur Python avec NumPy et programmation structurée\n";
    ss << "# (Approche 1 pour Répéter..Jusqu'à : drapeau booléen)\n";
    ss << "# ========================================================\n\n";

    transpileImports(ss);
    transpileProceduresAndFunctions(ss);
    transpileMain(ss);

    return ss.str();
}

void PythonTranspiler::transpileImports(std::ostringstream& ss) {
    bool hasImports = false;
    if (needArray_) {
        ss << "from numpy import array\n";
        hasImports = true;
    }
    if (!mathImports_.empty()) {
        ss << "from math import ";
        bool first = true;
        for (const auto& fn : mathImports_) {
            if (!first) ss << ", ";
            first = false;
            ss << fn;
        }
        ss << "\n";
        hasImports = true;
    }
    if (!randomImports_.empty()) {
        ss << "from random import ";
        bool first = true;
        for (const auto& fn : randomImports_) {
            if (!first) ss << ", ";
            first = false;
            ss << fn;
        }
        ss << "\n";
        hasImports = true;
    }
    if (!pickleImports_.empty()) {
        ss << "from pickle import ";
        bool first = true;
        for (const auto& fn : pickleImports_) {
            if (!first) ss << ", ";
            first = false;
            ss << fn;
        }
        ss << "\n";
        hasImports = true;
    }
    if (!osImports_.empty()) {
        ss << "from os import ";
        bool first = true;
        for (const auto& fn : osImports_) {
            if (!first) ss << ", ";
            first = false;
            ss << fn;
        }
        ss << "\n";
        hasImports = true;
    }
    if (hasImports) {
        ss << "\n";
    }
}

static void collectAssignedVariables(const std::vector<std::shared_ptr<Stmt>>& stmts, std::set<std::string>& out) {
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
        } else if (auto alloc = std::dynamic_pointer_cast<AllocStmt>(stmt)) {
            if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(alloc->target)) {
                out.insert(normalize(varExpr->name));
            }
        } else if (auto fr = std::dynamic_pointer_cast<FreeStmt>(stmt)) {
            if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(fr->target)) {
                out.insert(normalize(varExpr->name));
            }
        } else if (auto da = std::dynamic_pointer_cast<DerefAssignStmt>(stmt)) {
            if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(da->target)) {
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
            collectAssignedVariables(fors->body, out);
        } else if (auto ifs = std::dynamic_pointer_cast<IfStmt>(stmt)) {
            collectAssignedVariables(ifs->thenBlock, out);
            for (const auto& eb : ifs->elseIfBlocks) collectAssignedVariables(eb.body, out);
            collectAssignedVariables(ifs->elseBlock, out);
        } else if (auto wh = std::dynamic_pointer_cast<WhileStmt>(stmt)) {
            collectAssignedVariables(wh->body, out);
        } else if (auto rep = std::dynamic_pointer_cast<RepeatStmt>(stmt)) {
            collectAssignedVariables(rep->body, out);
        } else if (auto sw = std::dynamic_pointer_cast<SwitchStmt>(stmt)) {
            for (const auto& sc : sw->cases) collectAssignedVariables(sc.body, out);
        }
    }
}

void PythonTranspiler::transpileProceduresAndFunctions(std::ostringstream& ss) {
    for (const auto& proc : prog_.procedures) {
        transpileProcedure(proc, ss);
    }
    for (const auto& func : prog_.functions) {
        transpileFunction(func, ss);
    }
}

void PythonTranspiler::transpileProcedure(const ProcedureDecl& proc, std::ostringstream& ss) {
    currentLocalTypes_.clear();
    for (const auto& p : proc.params) {
        currentLocalTypes_[normalize(p.name)] = p.paramType;
    }
    for (const auto& vd : proc.localVars) {
        for (const auto& name : vd.names) {
            if (vd.typeSpec.kind == TypeKind::Pointer) {
                currentLocalTypes_[normalize(name)] = vd.varType.empty() ? ("^" + vd.typeSpec.elemType) : vd.varType;
            } else {
                currentLocalTypes_[normalize(name)] = vd.typeSpec.elemType.empty() ? (vd.typeSpec.name.empty() ? vd.varType : vd.typeSpec.name) : vd.typeSpec.elemType;
            }
        }
    }

    ss << "def " << pySafeName(proc.name) << "(";
    for (size_t i = 0; i < proc.params.size(); i++) {
        if (i > 0) ss << ", ";
        ss << pySafeName(proc.params[i].name);
    }
    ss << "):\n";
    indentLevel_ = 1;

    std::set<std::string> modifiedVars;
    collectAssignedVariables(proc.body, modifiedVars);
    for (const auto& vd : prog_.varDecls) {
        for (const auto& gName : vd.names) {
            std::string norm = normalize(gName);
            if (modifiedVars.find(norm) != modifiedVars.end()) {
                bool isLocal = false;
                for (const auto& p : proc.params) {
                    if (normalize(p.name) == norm) { isLocal = true; break; }
                }
                if (!isLocal) {
                    for (const auto& lvd : proc.localVars) {
                        for (const auto& lName : lvd.names) {
                            if (normalize(lName) == norm) { isLocal = true; break; }
                        }
                        if (isLocal) break;
                    }
                }
                if (!isLocal) {
                    ss << indent() << "global " << pySafeName(gName) << "\n";
                }
            }
        }
    }

    std::set<std::string> initialized;
    for (const auto& p : proc.params) {
        initialized.insert(normalize(p.name));
    }
    for (const auto& vd : proc.localVars) {
        std::string vt = normalize(vd.varType);
        if (vt == "procedure" || vt == "fonction") continue;
        for (const auto& name : vd.names) {
            if (initialized.insert(normalize(name)).second) {
                std::string init = getInitValuePy(vd.typeSpec, vd.varType);
                ss << indent() << pySafeName(name) << " = " << init << "\n";
            }
        }
    }

    if (proc.body.empty()) {
        ss << indent() << "pass\n";
    } else {
        transpileBlock(proc.body, ss);
    }

    std::vector<std::string> refParams;
    for (const auto& p : proc.params) {
        if (p.byRef) {
            refParams.push_back(pySafeName(p.name));
        }
    }
    if (!refParams.empty()) {
        ss << indent() << "return ";
        for (size_t i = 0; i < refParams.size(); i++) {
            if (i > 0) ss << ", ";
            ss << refParams[i];
        }
        ss << "\n";
    }

    indentLevel_ = 0;
    ss << "\n";
    currentLocalTypes_.clear();
}

void PythonTranspiler::transpileFunction(const FunctionDecl& func, std::ostringstream& ss) {
    currentLocalTypes_.clear();
    for (const auto& p : func.params) {
        currentLocalTypes_[normalize(p.name)] = p.paramType;
    }
    for (const auto& vd : func.localVars) {
        for (const auto& name : vd.names) {
            if (vd.typeSpec.kind == TypeKind::Pointer) {
                currentLocalTypes_[normalize(name)] = vd.varType.empty() ? ("^" + vd.typeSpec.elemType) : vd.varType;
            } else {
                currentLocalTypes_[normalize(name)] = vd.typeSpec.elemType.empty() ? (vd.typeSpec.name.empty() ? vd.varType : vd.typeSpec.name) : vd.typeSpec.elemType;
            }
        }
    }

    ss << "def " << pySafeName(func.name) << "(";
    for (size_t i = 0; i < func.params.size(); i++) {
        if (i > 0) ss << ", ";
        ss << pySafeName(func.params[i].name);
    }
    ss << "):\n";
    indentLevel_ = 1;

    std::set<std::string> modifiedVars;
    collectAssignedVariables(func.body, modifiedVars);
    for (const auto& vd : prog_.varDecls) {
        for (const auto& gName : vd.names) {
            std::string norm = normalize(gName);
            if (modifiedVars.find(norm) != modifiedVars.end()) {
                bool isLocal = false;
                for (const auto& p : func.params) {
                    if (normalize(p.name) == norm) { isLocal = true; break; }
                }
                if (!isLocal) {
                    for (const auto& lvd : func.localVars) {
                        for (const auto& lName : lvd.names) {
                            if (normalize(lName) == norm) { isLocal = true; break; }
                        }
                        if (isLocal) break;
                    }
                }
                if (!isLocal) {
                    ss << indent() << "global " << pySafeName(gName) << "\n";
                }
            }
        }
    }

    std::set<std::string> initialized;
    for (const auto& p : func.params) {
        initialized.insert(normalize(p.name));
    }
    for (const auto& vd : func.localVars) {
        std::string vt = normalize(vd.varType);
        if (vt == "procedure" || vt == "fonction") continue;
        for (const auto& name : vd.names) {
            if (initialized.insert(normalize(name)).second) {
                std::string init = getInitValuePy(vd.typeSpec, vd.varType);
                ss << indent() << pySafeName(name) << " = " << init << "\n";
            }
        }
    }

    if (func.body.empty()) {
        ss << indent() << "return " << getInitValuePy(TypeSpec(), func.returnType) << "\n";
    } else {
        transpileBlock(func.body, ss);
    }

    indentLevel_ = 0;
    ss << "\n";
    currentLocalTypes_.clear();
}

void PythonTranspiler::transpileMain(std::ostringstream& ss) {
    std::set<std::string> initialized;
    for (const auto& vd : prog_.varDecls) {
        std::string vt = normalize(vd.varType);
        if (vt == "procedure" || vt == "fonction") continue;
        for (const auto& name : vd.names) {
            if (initialized.insert(normalize(name)).second) {
                std::string init = getInitValuePy(vd.typeSpec, vd.varType);
                ss << pySafeName(name) << " = " << init << "\n";
            }
        }
    }

    if (!prog_.mainBody.empty()) {
        if (!initialized.empty()) {
            ss << "\n";
        }
        indentLevel_ = 0;
        transpileBlock(prog_.mainBody, ss);
    }
}

std::string PythonTranspiler::getInitValuePy(const TypeSpec& ts, const std::string& typeName) {
    if (ts.kind == TypeKind::Array2D) {
        return getMatrixInitPy(ts.rows, ts.cols, ts.elemType);
    }
    if (ts.kind == TypeKind::Array1D) {
        return getArrayInitPy(ts.rows, ts.elemType);
    }
    if (ts.kind == TypeKind::Record) {
        std::ostringstream ss;
        ss << "{";
        bool first = true;
        for (const auto& fld : ts.fields) {
            for (const auto& fname : fld->names) {
                if (!first) ss << ", ";
                first = false;
                ss << "\"" << fname << "\": " << getInitValuePy(fld->typeSpec, fld->varType);
            }
        }
        ss << "}";
        return ss.str();
    }
    if (ts.kind == TypeKind::FileText || ts.kind == TypeKind::FileData || ts.kind == TypeKind::Pointer) {
        return "None";
    }

    std::string norm = normalize(typeName.empty() ? ts.name : typeName);
    if (norm == "entier") return "0";
    if (norm == "reel") return "0.0";
    if (norm == "booleen") return "False";
    if (norm == "caractere" || norm == "chaine") return "\"\"";
    if (norm == "nil" || norm == "null" || norm == "nul" || norm == "pointeur" || (!norm.empty() && norm[0] == '^')) return "None";

    auto it = typeMap_.find(norm);
    if (it != typeMap_.end()) {
        const auto& td = it->second;
        if (td.typeSpec.kind == TypeKind::Pointer) {
            return "None";
        }
        if (td.typeSpec.kind == TypeKind::Record) {
            return getInitValuePy(td.typeSpec, "");
        }
        if (td.typeSpec.kind == TypeKind::Array2D) {
            return getMatrixInitPy(td.typeSpec.rows, td.typeSpec.cols, td.typeSpec.elemType);
        }
        if (td.typeSpec.kind == TypeKind::Array1D) {
            return getArrayInitPy(td.typeSpec.rows, td.typeSpec.elemType);
        }
    }

    return "0";
}

std::string PythonTranspiler::getArrayInitPy(int size, const std::string& elemType) {
    std::string norm = normalize(elemType);
    if (norm == "entier") {
        return "array([0] * " + std::to_string(size) + ")";
    }
    if (norm == "reel") {
        return "array([0.0] * " + std::to_string(size) + ")";
    }
    if (norm == "booleen") {
        return "array([False] * " + std::to_string(size) + ")";
    }
    if (norm == "caractere" || norm == "chaine") {
        return "array([\"\"] * " + std::to_string(size) + ")";
    }
    auto it = typeMap_.find(norm);
    if (it != typeMap_.end() && it->second.typeSpec.kind == TypeKind::Record) {
        std::string dictInit = getInitValuePy(it->second.typeSpec, "");
        return "array([" + dictInit + " for _ in range(" + std::to_string(size) + ")])";
    }
    return "array([0] * " + std::to_string(size) + ")";
}

std::string PythonTranspiler::getMatrixInitPy(int rows, int cols, const std::string& elemType) {
    std::string norm = normalize(elemType);
    if (norm == "entier") {
        return "array([[0] * " + std::to_string(cols) + " for _ in range(" + std::to_string(rows) + ")])";
    }
    if (norm == "reel") {
        return "array([[0.0] * " + std::to_string(cols) + " for _ in range(" + std::to_string(rows) + ")])";
    }
    if (norm == "booleen") {
        return "array([[False] * " + std::to_string(cols) + " for _ in range(" + std::to_string(rows) + ")])";
    }
    if (norm == "caractere" || norm == "chaine") {
        return "array([[\"\"] * " + std::to_string(cols) + " for _ in range(" + std::to_string(rows) + ")])";
    }
    auto it = typeMap_.find(norm);
    if (it != typeMap_.end() && it->second.typeSpec.kind == TypeKind::Record) {
        std::string dictInit = getInitValuePy(it->second.typeSpec, "");
        return "array([[" + dictInit + " for _ in range(" + std::to_string(cols) + ")] for _ in range(" + std::to_string(rows) + ")])";
    }
    return "array([[0] * " + std::to_string(cols) + " for _ in range(" + std::to_string(rows) + ")])";
}

bool PythonTranspiler::isWriteToFile(const std::shared_ptr<WriteStmt>& w) {
    if (!w || w->args.empty()) return false;
    if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(w->args[0])) {
        std::string vt = normalize(getExprType(varExpr));
        if (vt.find("fichier") != std::string::npos) return true;
    }
    return false;
}

bool PythonTranspiler::isReadFromFile(const std::shared_ptr<ReadStmt>& rd) {
    if (!rd || rd->targets.empty()) return false;
    if (rd->targets.size() == 2) {
        std::string t0 = rd->targets[0].name;
        std::string vt;
        auto itLoc = currentLocalTypes_.find(normalize(t0));
        if (itLoc != currentLocalTypes_.end()) vt = normalize(itLoc->second);
        else vt = normalize(varTypeMap_[normalize(t0)]);
        if (vt.find("fichier") != std::string::npos) return true;
    }
    return false;
}

std::string PythonTranspiler::buildPromptString(const std::vector<std::shared_ptr<Expr>>& args) {
    if (args.empty()) return "";
    if (args.size() == 1) {
        if (auto str = std::dynamic_pointer_cast<StringExpr>(args[0])) {
            return transpileExpr(str, 0);
        }
        std::string t = resolveBaseType(getExprType(args[0]));
        if (t == "chaine" || t == "caractere") {
            return transpileExpr(args[0], 0);
        }
        return "str(" + transpileExpr(args[0], 0) + ")";
    }
    std::string res;
    for (size_t i = 0; i < args.size(); i++) {
        if (i > 0) res += " + ";
        if (auto str = std::dynamic_pointer_cast<StringExpr>(args[i])) {
            res += transpileExpr(str, 0);
        } else {
            std::string t = resolveBaseType(getExprType(args[i]));
            if (t == "chaine" || t == "caractere") {
                res += transpileExpr(args[i], 0);
            } else {
                res += "str(" + transpileExpr(args[i], 0) + ")";
            }
        }
    }
    return res;
}

void PythonTranspiler::transpilePromptRead(const std::shared_ptr<WriteStmt>& w, const std::shared_ptr<ReadStmt>& rd, std::ostringstream& ss) {
    std::string prompt = buildPromptString(w->args);
    for (size_t i = 0; i < rd->targets.size(); i++) {
        const auto& tgt = rd->targets[i];
        std::string dest = transpileExpr(tgt.targetExpr, 0);
        std::string tName = getExprType(tgt.targetExpr);
        std::string p = (i == 0) ? prompt : "";
        ss << indent() << dest << " = " << transpileReadInput(tName, p) << "\n";
    }
}

void PythonTranspiler::transpileBlock(const std::vector<std::shared_ptr<Stmt>>& stmts, std::ostringstream& ss) {
    for (size_t i = 0; i < stmts.size(); i++) {
        if (!stmts[i]) continue;
        auto w = std::dynamic_pointer_cast<WriteStmt>(stmts[i]);
        if (w && !isWriteToFile(w) && !w->args.empty() && i + 1 < stmts.size()) {
            auto rd = std::dynamic_pointer_cast<ReadStmt>(stmts[i + 1]);
            if (rd && !isReadFromFile(rd)) {
                transpilePromptRead(w, rd, ss);
                i++;
                continue;
            }
        }
        transpileStmt(stmts[i], ss);
    }
}

std::string PythonTranspiler::formatRangeEnd(const std::shared_ptr<Expr>& endExpr, int delta) {
    if (auto num = std::dynamic_pointer_cast<NumberExpr>(endExpr)) {
        return std::to_string(num->value + delta);
    }
    std::string s = transpileExpr(endExpr, 0);
    if (getExprPrecedence(endExpr) <= 9) {
        s = "(" + s + ")";
    }
    if (delta > 0) {
        return s + " + " + std::to_string(delta);
    } else {
        return s + " - " + std::to_string(-delta);
    }
}

void PythonTranspiler::transpileStmt(const std::shared_ptr<Stmt>& stmt, std::ostringstream& ss) {
    if (!stmt) return;

    if (auto alloc = std::dynamic_pointer_cast<AllocStmt>(stmt)) {
        std::string target = transpileExpr(alloc->target, 0);
        std::string ptrType = getExprType(alloc->target);
        std::string pointeeType = (!ptrType.empty() && ptrType[0] == '^') ? ptrType.substr(1) : "";
        auto it = typeMap_.find(normalize(ptrType));
        if (it != typeMap_.end() && it->second.typeSpec.kind == TypeKind::Pointer) {
            pointeeType = it->second.typeSpec.elemType;
        }
        std::string init = "{}";
        if (!pointeeType.empty()) {
            auto itP = typeMap_.find(normalize(pointeeType));
            if (itP != typeMap_.end()) {
                init = getInitValuePy(itP->second.typeSpec, pointeeType);
            } else {
                std::string normP = normalize(pointeeType);
                if (normP == "entier") init = "[0]";
                else if (normP == "reel") init = "[0.0]";
                else if (normP == "booleen") init = "[False]";
                else if (normP == "chaine" || normP == "caractere") init = "[\"\"]";
                else init = "{}";
            }
        }
        ss << indent() << target << " = " << init << "\n";
        return;
    }
    if (auto fr = std::dynamic_pointer_cast<FreeStmt>(stmt)) {
        std::string target = transpileExpr(fr->target, 0);
        ss << indent() << target << " = None\n";
        return;
    }
    if (auto da = std::dynamic_pointer_cast<DerefAssignStmt>(stmt)) {
        std::string target = transpileExpr(da->target, 13);
        std::string ptrType = getExprType(da->target);
        std::string pointeeType = (!ptrType.empty() && ptrType[0] == '^') ? ptrType.substr(1) : "";
        auto it = typeMap_.find(normalize(ptrType));
        if (it != typeMap_.end() && it->second.typeSpec.kind == TypeKind::Pointer) {
            pointeeType = it->second.typeSpec.elemType;
        }
        std::string normP = normalize(pointeeType);
        bool isPrimitive = (normP == "entier" || normP == "reel" || normP == "booleen" || normP == "chaine" || normP == "caractere");
        if (isPrimitive) {
            ss << indent() << target << "[0] = " << transpileExpr(da->value, 0) << "\n";
        } else {
            ss << indent() << target << ".clear()\n";
            ss << indent() << target << ".update(" << transpileExpr(da->value, 0) << ")\n";
        }
        return;
    }
    if (auto as = std::dynamic_pointer_cast<AssignStmt>(stmt)) {
        std::string vt = normalize(varTypeMap_[normalize(as->target)]);
        auto it = typeMap_.find(vt);
        bool isRec = (it != typeMap_.end() && it->second.typeSpec.kind == TypeKind::Record);
        if (isRec && std::dynamic_pointer_cast<VariableExpr>(as->value)) {
            ss << indent() << pySafeName(as->target) << " = dict(" << transpileExpr(as->value, 0) << ")\n";
        } else {
            ss << indent() << pySafeName(as->target) << " = " << transpileExpr(as->value, 0) << "\n";
        }
        return;
    }
    if (auto aas = std::dynamic_pointer_cast<ArrayAssignStmt>(stmt)) {
        std::string target = aas->targetExpr ? transpileExpr(aas->targetExpr, 13) : pySafeName(aas->target);
        if (aas->index2) {
            ss << indent() << target << "[" << transpileExpr(aas->index, 0) << ", " << transpileExpr(aas->index2, 0) << "] = " << transpileExpr(aas->value, 0) << "\n";
        } else {
            ss << indent() << target << "[" << transpileExpr(aas->index, 0) << "] = " << transpileExpr(aas->value, 0) << "\n";
        }
        return;
    }
    if (auto ma = std::dynamic_pointer_cast<MemberAssignStmt>(stmt)) {
        ss << indent() << transpileExpr(ma->target, 13) << "[\"" << ma->member << "\"] = " << transpileExpr(ma->value, 0) << "\n";
        return;
    }
    if (auto r = std::dynamic_pointer_cast<ReturnStmt>(stmt)) {
        ss << indent() << "return " << (r->value ? transpileExpr(r->value, 0) : "") << "\n";
        return;
    }
    if (auto w = std::dynamic_pointer_cast<WriteStmt>(stmt)) {
        // Check if first argument is a file
        if (!w->args.empty()) {
            if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(w->args[0])) {
                std::string vt = normalize(getExprType(varExpr));
                if (vt.find("fichier") != std::string::npos) {
                    std::string fVar = pySafeName(varExpr->name);
                    if (vt.find("texte") != std::string::npos) {
                        ss << indent() << "print(";
                        for (size_t i = 1; i < w->args.size(); i++) {
                            if (i > 1) ss << ", ";
                            ss << transpileExpr(w->args[i], 0);
                        }
                        ss << ", file=" << fVar << ")\n";
                    } else {
                        // Binary file: dump
                        for (size_t i = 1; i < w->args.size(); i++) {
                            ss << indent() << "dump(" << transpileExpr(w->args[i], 0) << ", " << fVar << ")\n";
                        }
                    }
                    return;
                }
            }
        }

        // Standard console print
        ss << indent() << "print(";
        for (size_t i = 0; i < w->args.size(); i++) {
            if (i > 0) ss << ", ";
            if (std::dynamic_pointer_cast<NilExpr>(w->args[i])) {
                ss << "\"NIL\"";
                continue;
            }
            std::string t = getExprType(w->args[i]);
            bool isPtr = (!t.empty() && t[0] == '^');
            auto it = typeMap_.find(normalize(t));
            if (it != typeMap_.end() && it->second.typeSpec.kind == TypeKind::Pointer) isPtr = true;
            if (isPtr) {
                std::string pointee = (!t.empty() && t[0] == '^') ? t.substr(1) : (it != typeMap_.end() ? it->second.typeSpec.elemType : "");
                std::string normP = normalize(pointee);
                bool isPrim = (normP == "entier" || normP == "reel" || normP == "booleen" || normP == "chaine" || normP == "caractere");
                std::string exprStr = transpileExpr(w->args[i], 0);
                if (isPrim) {
                    ss << "(\"NIL\" if " << exprStr << " is None else " << exprStr << "[0])";
                } else {
                    ss << "(\"NIL\" if " << exprStr << " is None else " << exprStr << ")";
                }
            } else {
                ss << transpileExpr(w->args[i], 0);
            }
        }
        if (w->sep != " ") {
            ss << ", sep=" << "\"" << w->sep << "\"";
        }
        if (w->fin != "\n") {
            ss << ", end=" << "\"" << w->fin << "\"";
        }
        ss << ")\n";
        return;
    }
    if (auto rd = std::dynamic_pointer_cast<ReadStmt>(stmt)) {
        // Check if file read: Lire(f, var)
        if (rd->targets.size() == 2) {
            std::string t0 = rd->targets[0].name;
            std::string vt;
            auto itLoc = currentLocalTypes_.find(normalize(t0));
            if (itLoc != currentLocalTypes_.end()) vt = normalize(itLoc->second);
            else vt = normalize(varTypeMap_[normalize(t0)]);
            if (vt.find("fichier") != std::string::npos) {
                std::string fVar = pySafeName(t0);
                std::string dest = transpileExpr(rd->targets[1].targetExpr, 0);
                if (vt.find("texte") != std::string::npos) {
                    ss << indent() << dest << " = " << fVar << ".read()\n";
                } else {
                    ss << indent() << dest << " = load(" << fVar << ")\n";
                }
                return;
            }
        }

        for (const auto& tgt : rd->targets) {
            std::string dest = transpileExpr(tgt.targetExpr, 0);
            std::string tName = getExprType(tgt.targetExpr);
            ss << indent() << dest << " = " << transpileReadInput(tName) << "\n";
        }
        return;
    }
    if (auto call = std::dynamic_pointer_cast<CallStmt>(stmt)) {
        std::string n = normalize(call->name);
        if (n == "ouvrir") {
            if (call->args.size() >= 3) {
                std::string path = transpileExpr(call->args[0], 0);
                std::string fVar = transpileExpr(call->args[1], 0);
                std::string mode = transpileExpr(call->args[2], 0);
                ss << indent() << fVar << " = open(" << path << ", " << mode;
                if (mode.find('b') == std::string::npos) {
                    ss << ", encoding='utf-8'";
                }
                ss << ")\n";
            }
            return;
        }
        if (n == "fermer") {
            if (!call->args.empty()) {
                ss << indent() << transpileExpr(call->args[0], 0) << ".close()\n";
            }
            return;
        }
        if (n == "lire_ligne") {
            if (call->args.size() >= 2) {
                std::string fVar = transpileExpr(call->args[0], 0);
                std::string dest = transpileExpr(call->args[1], 0);
                ss << indent() << dest << " = " << fVar << ".readline().rstrip('\\r\\n')\n";
            }
            return;
        }

        // Check if procedure has byRef parameters
        auto it = procMap_.find(n);
        if (it != procMap_.end()) {
            const auto& proc = it->second;
            std::vector<std::string> refArgs;
            for (size_t i = 0; i < proc.params.size() && i < call->args.size(); i++) {
                if (proc.params[i].byRef) {
                    refArgs.push_back(transpileExpr(call->args[i], 0));
                }
            }
            if (!refArgs.empty()) {
                ss << indent();
                for (size_t i = 0; i < refArgs.size(); i++) {
                    if (i > 0) ss << ", ";
                    ss << refArgs[i];
                }
                ss << " = " << pySafeName(call->name) << "(";
                for (size_t i = 0; i < call->args.size(); i++) {
                    if (i > 0) ss << ", ";
                    ss << transpileExpr(call->args[i], 0);
                }
                ss << ")\n";
                return;
            }
        }

        ss << indent() << pySafeName(call->name) << "(";
        for (size_t i = 0; i < call->args.size(); i++) {
            if (i > 0) ss << ", ";
            ss << transpileExpr(call->args[i], 0);
        }
        ss << ")\n";
        return;
    }
    if (auto ifs = std::dynamic_pointer_cast<IfStmt>(stmt)) {
        ss << indent() << "if " << transpileExpr(ifs->condition, 0) << ":\n";
        indentLevel_++;
        if (ifs->thenBlock.empty()) ss << indent() << "pass\n";
        else transpileBlock(ifs->thenBlock, ss);
        indentLevel_--;

        for (const auto& eb : ifs->elseIfBlocks) {
            ss << indent() << "elif " << transpileExpr(eb.condition, 0) << ":\n";
            indentLevel_++;
            if (eb.body.empty()) ss << indent() << "pass\n";
            else transpileBlock(eb.body, ss);
            indentLevel_--;
        }

        if (!ifs->elseBlock.empty()) {
            ss << indent() << "else:\n";
            indentLevel_++;
            transpileBlock(ifs->elseBlock, ss);
            indentLevel_--;
        }
        return;
    }
    if (auto fors = std::dynamic_pointer_cast<ForStmt>(stmt)) {
        std::string var = pySafeName(fors->varName);
        std::string varNorm = normalize(fors->varName);
        std::string varType = "";
        auto itLoc = currentLocalTypes_.find(varNorm);
        if (itLoc != currentLocalTypes_.end()) varType = normalize(resolveBaseType(itLoc->second));
        else {
            auto it = varTypeMap_.find(varNorm);
            if (it != varTypeMap_.end()) varType = normalize(resolveBaseType(it->second));
        }

        std::string startType = normalize(getExprType(fors->start));
        bool isCharLoop = (varType == "caractere" || varType == "caractère");
        if (!isCharLoop) {
            if (auto s = std::dynamic_pointer_cast<StringExpr>(fors->start)) {
                if (s->value.size() == 1) isCharLoop = true;
            }
        }

        bool isBoolLoop = (varType == "booleen" || varType == "booléen" || startType == "booleen");
        if (!isBoolLoop) {
            if (std::dynamic_pointer_cast<BoolExpr>(fors->start)) {
                isBoolLoop = true;
            }
        }

        if (isCharLoop) {
            std::string start = transpileExpr(fors->start, 0);
            std::string end = transpileExpr(fors->end, 0);
            std::string startOrd = "ord(" + start + ")";
            std::string endOrd = "ord(" + end + ")";

            int64_t stepVal = 1;
            bool isConstStep = true;
            if (fors->step) {
                if (auto num = std::dynamic_pointer_cast<NumberExpr>(fors->step)) {
                    stepVal = num->value;
                } else if (auto un = std::dynamic_pointer_cast<UnaryExpr>(fors->step)) {
                    if (un->op == "-" || un->op == "+") {
                        if (auto num = std::dynamic_pointer_cast<NumberExpr>(un->operand)) {
                            stepVal = (un->op == "-") ? -num->value : num->value;
                        } else isConstStep = false;
                    } else isConstStep = false;
                } else isConstStep = false;
            }

            if (isConstStep) {
                if (stepVal > 0) {
                    if (stepVal == 1) {
                        ss << indent() << "for " << var << " in (chr(_c) for _c in range(" << startOrd << ", " << endOrd << " + 1)):\n";
                    } else {
                        ss << indent() << "for " << var << " in (chr(_c) for _c in range(" << startOrd << ", " << endOrd << " + 1, " << stepVal << ")):\n";
                    }
                } else if (stepVal < 0) {
                    ss << indent() << "for " << var << " in (chr(_c) for _c in range(" << startOrd << ", " << endOrd << " - 1, " << stepVal << ")):\n";
                }
            } else {
                std::string step = transpileExpr(fors->step, 0);
                ss << indent() << "for " << var << " in (chr(_c) for _c in range(" << startOrd << ", " << endOrd << " + (1 if (" << step << ") > 0 else -1), " << step << ")):\n";
            }
        } else if (isBoolLoop) {
            std::string start = transpileExpr(fors->start, 0);
            std::string end = transpileExpr(fors->end, 0);
            int64_t stepVal = 1;
            bool isConstStep = true;
            if (fors->step) {
                if (auto num = std::dynamic_pointer_cast<NumberExpr>(fors->step)) {
                    stepVal = num->value;
                } else if (auto un = std::dynamic_pointer_cast<UnaryExpr>(fors->step)) {
                    if (un->op == "-" || un->op == "+") {
                        if (auto num = std::dynamic_pointer_cast<NumberExpr>(un->operand)) {
                            stepVal = (un->op == "-") ? -num->value : num->value;
                        } else isConstStep = false;
                    } else isConstStep = false;
                } else isConstStep = false;
            }
            if (isConstStep && stepVal == 1 && start == "False" && end == "True") {
                ss << indent() << "for " << var << " in [False, True]:\n";
            } else if (isConstStep && stepVal == -1 && start == "True" && end == "False") {
                ss << indent() << "for " << var << " in [True, False]:\n";
            } else {
                std::string stepStr = fors->step ? transpileExpr(fors->step, 0) : "1";
                ss << indent() << "for " << var << " in (bool(_b) for _b in range(int(" << start << "), int(" << end << ") + (1 if (" << stepStr << ") > 0 else -1), " << stepStr << ")):\n";
            }
        } else {
            std::string start = transpileExpr(fors->start, 0);
            if (fors->step) {
                int64_t stepVal = 0;
                bool isConstStep = false;
                if (auto num = std::dynamic_pointer_cast<NumberExpr>(fors->step)) {
                    stepVal = num->value;
                    isConstStep = true;
                } else if (auto un = std::dynamic_pointer_cast<UnaryExpr>(fors->step)) {
                    if (un->op == "-" || un->op == "+") {
                        if (auto num = std::dynamic_pointer_cast<NumberExpr>(un->operand)) {
                            stepVal = (un->op == "-") ? -num->value : num->value;
                            isConstStep = true;
                        }
                    }
                }
                if (isConstStep) {
                    if (stepVal > 0) {
                        if (stepVal == 1) {
                            ss << indent() << "for " << var << " in range(" << start << ", " << formatRangeEnd(fors->end, 1) << "):\n";
                        } else {
                            ss << indent() << "for " << var << " in range(" << start << ", " << formatRangeEnd(fors->end, 1) << ", " << stepVal << "):\n";
                        }
                    } else if (stepVal < 0) {
                        ss << indent() << "for " << var << " in range(" << start << ", " << formatRangeEnd(fors->end, -1) << ", " << stepVal << "):\n";
                    } else {
                        ss << indent() << "for " << var << " in range(" << start << ", " << formatRangeEnd(fors->end, 1) << "):\n";
                    }
                } else {
                    std::string step = transpileExpr(fors->step, 0);
                    std::string end = transpileExpr(fors->end, 0);
                    ss << indent() << "for " << var << " in range(" << start << ", " << end << " + (1 if (" << step << ") > 0 else -1), " << step << "):\n";
                }
            } else {
                ss << indent() << "for " << var << " in range(" << start << ", " << formatRangeEnd(fors->end, 1) << "):\n";
            }
        }
        indentLevel_++;
        if (fors->body.empty()) ss << indent() << "pass\n";
        else transpileBlock(fors->body, ss);
        indentLevel_--;
        return;
    }
    if (auto wh = std::dynamic_pointer_cast<WhileStmt>(stmt)) {
        ss << indent() << "while " << transpileExpr(wh->condition, 0) << ":\n";
        indentLevel_++;
        if (wh->body.empty()) ss << indent() << "pass\n";
        else transpileBlock(wh->body, ss);
        indentLevel_--;
        return;
    }
    if (auto rep = std::dynamic_pointer_cast<RepeatStmt>(stmt)) {
        // Approche 1 : drapeau booléen
        repeatCounter_++;
        std::string okVar = "_ok_" + std::to_string(repeatCounter_);
        ss << indent() << okVar << " = False\n";
        ss << indent() << "while not " << okVar << ":\n";
        indentLevel_++;
        if (rep->body.empty()) {
            ss << indent() << "pass\n";
        } else {
            transpileBlock(rep->body, ss);
        }
        ss << indent() << okVar << " = " << transpileExpr(rep->condition, 0) << "\n";
        indentLevel_--;
        return;
    }
    if (auto sw = std::dynamic_pointer_cast<SwitchStmt>(stmt)) {
        std::string selExpr = transpileExpr(sw->expr, 0);
        bool first = true;
        for (const auto& cs : sw->cases) {
            if (cs.isDefault) {
                ss << indent() << (first ? "if True:\n" : "else:\n");
                indentLevel_++;
                if (cs.body.empty()) ss << indent() << "pass\n";
                else transpileBlock(cs.body, ss);
                indentLevel_--;
                first = false;
                continue;
            }

            // Partition cs.items into discrete values and ranges
            std::vector<std::shared_ptr<Expr>> discreteValues;
            std::vector<SwitchCaseItem> ranges;
            for (const auto& item : cs.items) {
                if (item.isRange()) {
                    ranges.push_back(item);
                } else {
                    discreteValues.push_back(item.start);
                }
            }
            if (cs.items.empty() && !cs.values.empty()) {
                discreteValues = cs.values;
            }

            std::vector<std::string> condParts;
            if (discreteValues.size() == 1) {
                condParts.push_back(selExpr + " == " + transpileExpr(discreteValues[0], 0));
            } else if (discreteValues.size() > 1) {
                std::string setStr = "{";
                for (size_t k = 0; k < discreteValues.size(); k++) {
                    if (k > 0) setStr += ", ";
                    setStr += transpileExpr(discreteValues[k], 0);
                }
                setStr += "}";
                condParts.push_back(selExpr + " in " + setStr);
            }

            for (const auto& r : ranges) {
                condParts.push_back(transpileExpr(r.start, 0) + " <= " + selExpr + " <= " + transpileExpr(r.end, 0));
            }

            std::string fullCond;
            for (size_t k = 0; k < condParts.size(); k++) {
                if (k > 0) fullCond += " or ";
                fullCond += condParts[k];
            }

            ss << indent() << (first ? "if " : "elif ") << fullCond << ":\n";
            first = false;
            indentLevel_++;
            if (cs.body.empty()) ss << indent() << "pass\n";
            else transpileBlock(cs.body, ss);
            indentLevel_--;
        }
        return;
    }
}

std::string PythonTranspiler::resolveBaseType(const std::string& typeName) {
    std::string norm = normalize(typeName);
    auto it = typeMap_.find(norm);
    if (it != typeMap_.end()) {
        if (!it->second.typeSpec.elemType.empty()) {
            return resolveBaseType(it->second.typeSpec.elemType);
        }
        if (!it->second.typeSpec.name.empty()) {
            return resolveBaseType(it->second.typeSpec.name);
        }
    }
    return norm;
}

std::string PythonTranspiler::transpileReadInput(const std::string& typeName, const std::string& prompt) {
    std::string norm = resolveBaseType(typeName);
    std::string inCall = prompt.empty() ? "input()" : "input(" + prompt + ")";
    if (norm == "entier") {
        return "int(" + inCall + ")";
    }
    if (norm == "reel") {
        return "float(" + inCall + ")";
    }
    if (norm == "booleen") {
        return inCall + ".strip().lower() in ('vrai', 'true', '1')";
    }
    return inCall;
}

std::string PythonTranspiler::getExprType(const std::shared_ptr<Expr>& expr) {
    if (!expr) return "chaine";
    if (std::dynamic_pointer_cast<NilExpr>(expr)) return "nil";
    if (std::dynamic_pointer_cast<StringExpr>(expr)) return "chaine";
    if (std::dynamic_pointer_cast<NumberExpr>(expr)) return "entier";
    if (std::dynamic_pointer_cast<RealExpr>(expr)) return "reel";
    if (std::dynamic_pointer_cast<BoolExpr>(expr)) return "booleen";
    if (auto deref = std::dynamic_pointer_cast<DerefExpr>(expr)) {
        std::string tgtType = getExprType(deref->target);
        if (!tgtType.empty() && tgtType[0] == '^') return resolveBaseType(tgtType.substr(1));
        auto it = typeMap_.find(normalize(tgtType));
        if (it != typeMap_.end() && it->second.typeSpec.kind == TypeKind::Pointer) {
            return resolveBaseType(it->second.typeSpec.elemType);
        }
        return resolveBaseType(tgtType);
    }

    if (auto v = std::dynamic_pointer_cast<VariableExpr>(expr)) {
        std::string n = normalize(v->name);
        auto itLoc = currentLocalTypes_.find(n);
        if (itLoc != currentLocalTypes_.end()) return resolveBaseType(itLoc->second);
        auto it = varTypeMap_.find(n);
        if (it != varTypeMap_.end()) return resolveBaseType(it->second);
    }
    if (auto aa = std::dynamic_pointer_cast<ArrayAccessExpr>(expr)) {
        std::string n = aa->target ? "" : normalize(aa->name);
        if (!n.empty()) {
            auto itLoc = currentLocalTypes_.find(n);
            if (itLoc != currentLocalTypes_.end()) return resolveBaseType(itLoc->second);
            auto it = varTypeMap_.find(n);
            if (it != varTypeMap_.end()) return resolveBaseType(it->second);
        } else if (aa->target) {
            return resolveBaseType(getExprType(aa->target));
        }
    }
    if (auto mem = std::dynamic_pointer_cast<MemberAccessExpr>(expr)) {
        std::string objType = getExprType(mem->object);
        if (!objType.empty() && objType[0] == '^') objType = objType.substr(1);
        auto it = typeMap_.find(normalize(objType));
        if (it != typeMap_.end() && it->second.typeSpec.kind == TypeKind::Pointer) {
            objType = it->second.typeSpec.elemType;
            it = typeMap_.find(normalize(objType));
        }
        if (it != typeMap_.end()) {
            for (const auto& fld : it->second.typeSpec.fields) {
                for (const auto& fname : fld->names) {
                    if (normalize(fname) == normalize(mem->member)) {
                        return resolveBaseType(fld->varType.empty() ? fld->typeSpec.name : fld->varType);
                    }
                }
            }
        }
    }
    if (auto call = std::dynamic_pointer_cast<CallExpr>(expr)) {
        std::string n = normalize(call->name);
        if (n == "sous_chaine" || n == "souschaine" || n == "convch" || n == "majus" || n == "minus" || n == "chr") {
            return "chaine";
        }
        if (n == "long" || n == "pos" || n == "ord" || n == "ent" || n == "alea" || n == "aléa") {
            return "entier";
        }
        if (n == "racine" || n == "sin" || n == "cos" || n == "tan" || n == "atan" || n == "arctan" || n == "exp" || n == "log" || n == "ln" || n == "arrondi" || n == "frac") {
            return "reel";
        }
        if (n == "fin_fichier") {
            return "booleen";
        }
        auto itF = funcMap_.find(n);
        if (itF != funcMap_.end()) {
            return resolveBaseType(itF->second.returnType);
        }
    }
    return "chaine";
}

static bool isComparisonOp(const std::string& op) {
    return (op == "==" || op == "!=" || op == "<" || op == "<=" || op == ">" || op == ">=" || op == "in");
}

static int getBinaryPrecedence(const std::string& op) {
    std::string n = normalize(op);
    if (n == "ou" || n == "or") return 1;
    if (n == "et" || n == "and") return 2;
    if (n == "==" || n == "=" || n == "!=" || n == "≠" ||
        n == "<" || n == "<=" || n == "≤" ||
        n == ">" || n == ">=" || n == "≥" ||
        n == "in" || n == "∈") return 4;
    if (n == "+" || n == "-") return 9;
    if (n == "*" || n == "/" || n == "//" || n == "div" || n == "%" || n == "mod") return 10;
    if (n == "**" || n == "^") return 12;
    return 0;
}

static int getUnaryPrecedence(const std::string& op) {
    std::string n = normalize(op);
    if (n == "non" || n == "not") return 3;
    if (n == "-" || n == "+") return 11;
    return 11;
}

int PythonTranspiler::getExprPrecedence(const std::shared_ptr<Expr>& expr) {
    if (!expr) return 0;
    if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
        return getBinaryPrecedence(bin->op);
    }
    if (auto un = std::dynamic_pointer_cast<UnaryExpr>(expr)) {
        return getUnaryPrecedence(un->op);
    }
    return 13;
}

std::string PythonTranspiler::transpileExpr(const std::shared_ptr<Expr>& expr, int parentPrec, bool isRight) {
    if (!expr) return "None";

    if (std::dynamic_pointer_cast<NilExpr>(expr)) {
        return "None";
    }
    if (auto deref = std::dynamic_pointer_cast<DerefExpr>(expr)) {
        std::string ptrType = getExprType(deref->target);
        std::string pointeeType = (!ptrType.empty() && ptrType[0] == '^') ? ptrType.substr(1) : "";
        auto it = typeMap_.find(normalize(ptrType));
        if (it != typeMap_.end() && it->second.typeSpec.kind == TypeKind::Pointer) {
            pointeeType = it->second.typeSpec.elemType;
        }
        std::string normP = normalize(pointeeType);
        bool isPrimitive = (normP == "entier" || normP == "reel" || normP == "booleen" || normP == "chaine" || normP == "caractere");
        if (isPrimitive) {
            return transpileExpr(deref->target, 13) + "[0]";
        }
        return transpileExpr(deref->target, parentPrec, isRight);
    }
    if (auto num = std::dynamic_pointer_cast<NumberExpr>(expr)) {
        return std::to_string(num->value);
    }
    if (auto r = std::dynamic_pointer_cast<RealExpr>(expr)) {
        std::ostringstream ss;
        ss.precision(10);
        ss << r->value;
        std::string s = ss.str();
        if (s.find('.') == std::string::npos && s.find('e') == std::string::npos && s.find('E') == std::string::npos) {
            s += ".0";
        }
        return s;
    }
    if (auto str = std::dynamic_pointer_cast<StringExpr>(expr)) {
        std::string res = "\"";
        for (char c : str->value) {
            if (c == '"') res += "\\\"";
            else if (c == '\\') res += "\\\\";
            else if (c == '\n') res += "\\n";
            else if (c == '\t') res += "\\t";
            else res += c;
        }
        res += "\"";
        return res;
    }
    if (auto b = std::dynamic_pointer_cast<BoolExpr>(expr)) {
        return b->value ? "True" : "False";
    }
    if (auto v = std::dynamic_pointer_cast<VariableExpr>(expr)) {
        return pySafeName(v->name);
    }
    if (auto mem = std::dynamic_pointer_cast<MemberAccessExpr>(expr)) {
        return transpileExpr(mem->object, 13) + "[\"" + mem->member + "\"]";
    }
    if (auto aa = std::dynamic_pointer_cast<ArrayAccessExpr>(expr)) {
        std::string target = aa->target ? transpileExpr(aa->target, 13) : pySafeName(aa->name);
        if (aa->index2) {
            return target + "[" + transpileExpr(aa->index, 0) + ", " + transpileExpr(aa->index2, 0) + "]";
        }
        return target + "[" + transpileExpr(aa->index, 0) + "]";
    }
    if (auto un = std::dynamic_pointer_cast<UnaryExpr>(expr)) {
        std::string op = normalize(un->op);
        int unPrec = getUnaryPrecedence(un->op);
        std::string inner;
        if (op == "non" || op == "not") {
            inner = "not " + transpileExpr(un->operand, unPrec);
        } else {
            inner = un->op + transpileExpr(un->operand, unPrec);
        }
        if (unPrec < parentPrec) {
            return "(" + inner + ")";
        }
        return inner;
    }
    if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
        std::string op = bin->op;
        std::string normOp = normalize(op);
        std::string pyOp;
        if (normOp == "div") pyOp = "//";
        else if (normOp == "mod") pyOp = "%";
        else if (normOp == "et" || normOp == "and") pyOp = "and";
        else if (normOp == "ou" || normOp == "or") pyOp = "or";
        else if (op == "=") pyOp = "==";
        else if (op == "≠" || op == "!=") pyOp = "!=";
        else if (op == "≤") pyOp = "<=";
        else if (op == "≥") pyOp = ">=";
        else if (op == "^" || op == "**") pyOp = "**";
        else if (op == "∈") pyOp = "in";
        else pyOp = op;

        int myPrec = getBinaryPrecedence(pyOp);

        std::string leftStr;
        int leftPrec = getExprPrecedence(bin->left);
        bool leftNeedsParen = false;
        if (leftPrec < myPrec) {
            leftNeedsParen = true;
        } else if (leftPrec == myPrec) {
            if (pyOp == "**" || isComparisonOp(pyOp)) {
                leftNeedsParen = true;
            }
        }
        leftStr = transpileExpr(bin->left, leftNeedsParen ? 0 : myPrec, false);
        if (leftNeedsParen) {
            leftStr = "(" + leftStr + ")";
        }

        std::string rightStr;
        int rightPrec = getExprPrecedence(bin->right);
        bool rightNeedsParen = false;
        if (rightPrec < myPrec) {
            rightNeedsParen = true;
        } else if (rightPrec == myPrec) {
            if (pyOp == "-") {
                rightNeedsParen = true;
            } else if (pyOp == "/" || pyOp == "//" || pyOp == "%") {
                rightNeedsParen = true;
            } else if (pyOp == "*") {
                if (auto rBin = std::dynamic_pointer_cast<BinaryExpr>(bin->right)) {
                    std::string rOp = normalize(rBin->op);
                    if (rOp == "/" || rOp == "//" || rOp == "div" || rOp == "%" || rOp == "mod") {
                        rightNeedsParen = true;
                    }
                }
            } else if (isComparisonOp(pyOp)) {
                rightNeedsParen = true;
            }
        }
        rightStr = transpileExpr(bin->right, rightNeedsParen ? 0 : myPrec, true);
        if (rightNeedsParen) {
            rightStr = "(" + rightStr + ")";
        }

        std::string res = leftStr + " " + pyOp + " " + rightStr;

        if (myPrec < parentPrec) {
            return "(" + res + ")";
        }
        return res;
    }
    if (auto setLit = std::dynamic_pointer_cast<SetLiteralExpr>(expr)) {
        std::string s = "[";
        for (size_t i = 0; i < setLit->elements.size(); i++) {
            if (i > 0) s += ", ";
            s += transpileExpr(setLit->elements[i], 0);
        }
        s += "]";
        return s;
    }
    if (auto call = std::dynamic_pointer_cast<CallExpr>(expr)) {
        std::string n = normalize(call->name);

        if (n == "racine") {
            return "sqrt(" + transpileExpr(call->args[0], 0) + ")";
        }
        if (n == "exp") {
            return "exp(" + transpileExpr(call->args[0], 0) + ")";
        }
        if (n == "ln" || n == "log") {
            return "log(" + transpileExpr(call->args[0], 0) + ")";
        }
        if (n == "abs") {
            return "abs(" + transpileExpr(call->args[0], 0) + ")";
        }
        if (n == "ent") {
            return "int(" + transpileExpr(call->args[0], 0) + ")";
        }
        if (n == "arrondi") {
            return "round(" + transpileExpr(call->args[0], 0) + ")";
        }
        if (n == "carre") {
            std::string argStr = transpileExpr(call->args[0], 12);
            int argPrec = getExprPrecedence(call->args[0]);
            if (argPrec < 12) argStr = "(" + argStr + ")";
            std::string res = argStr + " ** 2";
            if (12 < parentPrec) res = "(" + res + ")";
            return res;
        }
        if (n == "long") {
            return "len(" + transpileExpr(call->args[0], 0) + ")";
        }
        if (n == "chr") {
            return "chr(" + transpileExpr(call->args[0], 0) + ")";
        }
        if (n == "ord") {
            return "ord(" + transpileExpr(call->args[0], 0) + ")";
        }
        if (n == "convch") {
            std::string t = resolveBaseType(getExprType(call->args[0]));
            if (t == "chaine" || t == "caractere") {
                return transpileExpr(call->args[0], 0);
            }
            return "str(" + transpileExpr(call->args[0], 0) + ")";
        }
        if (n == "valeur") {
            std::string a0 = transpileExpr(call->args[0], 0);
            std::string t = resolveBaseType(getExprType(call->args[0]));
            std::string a0Str = (t == "chaine" || t == "caractere") ? a0 : "str(" + a0 + ")";
            return "(float(" + a0 + ") if ('.' in " + a0Str + " or 'e' in " + a0Str + ".lower()) else int(" + a0 + "))";
        }
        if (n == "sous_chaine" || n == "souschaine") {
            std::string t = resolveBaseType(getExprType(call->args[0]));
            bool isStr = (t == "chaine" || t == "caractere");
            std::string s = transpileExpr(call->args[0], 13);
            if (!isStr) {
                s = "str(" + transpileExpr(call->args[0], 0) + ")";
            } else if (getExprPrecedence(call->args[0]) < 13) {
                s = "(" + s + ")";
            }
            std::string d = transpileExpr(call->args[1], 0);
            std::string f = transpileExpr(call->args[2], 0);
            return s + "[" + d + ":" + f + "]";
        }
        if (n == "pos") {
            std::string sub = transpileExpr(call->args[0], 0);
            std::string strObj = transpileExpr(call->args[1], 13);
            if (getExprPrecedence(call->args[1]) < 13) strObj = "(" + strObj + ")";
            return strObj + ".find(" + sub + ")";
        }
        if (n == "majus") {
            std::string t = resolveBaseType(getExprType(call->args[0]));
            std::string s = transpileExpr(call->args[0], 13);
            if (t != "chaine" && t != "caractere") s = "str(" + transpileExpr(call->args[0], 0) + ")";
            else if (getExprPrecedence(call->args[0]) < 13) s = "(" + s + ")";
            return s + ".upper()";
        }
        if (n == "minus") {
            std::string t = resolveBaseType(getExprType(call->args[0]));
            std::string s = transpileExpr(call->args[0], 13);
            if (t != "chaine" && t != "caractere") s = "str(" + transpileExpr(call->args[0], 0) + ")";
            else if (getExprPrecedence(call->args[0]) < 13) s = "(" + s + ")";
            return s + ".lower()";
        }
        if (n == "alea" || n == "aléa") {
            return "randint(" + transpileExpr(call->args[0], 0) + ", " + transpileExpr(call->args[1], 0) + ")";
        }
        if (n == "graine") {
            return "seed(" + transpileExpr(call->args[0], 0) + ")";
        }
        if (n == "fin_fichier") {
            std::string fVar = transpileExpr(call->args[0], 0);
            return "(" + fVar + ".tell() >= fstat(" + fVar + ".fileno()).st_size)";
        }
        if (n == "sin") {
            return "sin(" + transpileExpr(call->args[0], 0) + ")";
        }
        if (n == "cos") {
            return "cos(" + transpileExpr(call->args[0], 0) + ")";
        }
        if (n == "tan") {
            return "tan(" + transpileExpr(call->args[0], 0) + ")";
        }
        if (n == "arctan" || n == "atan") {
            return "atan(" + transpileExpr(call->args[0], 0) + ")";
        }
        if (n == "frac") {
            std::string argStr = transpileExpr(call->args[0], 10);
            int argPrec = getExprPrecedence(call->args[0]);
            if (argPrec < 10) argStr = "(" + argStr + ")";
            std::string res = argStr + " % 1";
            if (10 < parentPrec) res = "(" + res + ")";
            return res;
        }
        if (n == "puissance") {
            std::string baseStr = transpileExpr(call->args[0], 12);
            if (getExprPrecedence(call->args[0]) < 12) baseStr = "(" + baseStr + ")";
            std::string expStr = transpileExpr(call->args[1], 12);
            if (getExprPrecedence(call->args[1]) < 12) expStr = "(" + expStr + ")";
            std::string res = baseStr + " ** " + expStr;
            if (12 < parentPrec) res = "(" + res + ")";
            return res;
        }
        if (n == "inserer") {
            std::string c1 = transpileExpr(call->args[0], 0);
            std::string c2 = transpileExpr(call->args[1], 0);
            std::string p = transpileExpr(call->args[2], 0);
            return "(" + c2 + "[:" + p + "] + " + c1 + " + " + c2 + "[" + p + ":])";
        }

        std::string s = pySafeName(call->name) + "(";
        for (size_t i = 0; i < call->args.size(); i++) {
            if (i > 0) s += ", ";
            s += transpileExpr(call->args[i], 0);
        }
        s += ")";
        return s;
    }

    return "None";
}

} // namespace algo
