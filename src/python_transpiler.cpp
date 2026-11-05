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
            varTypeMap_[normalize(name)] = vd.typeSpec.elemType.empty() ? (vd.typeSpec.name.empty() ? vd.varType : vd.typeSpec.name) : vd.typeSpec.elemType;
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
    if (auto setLit = std::dynamic_pointer_cast<SetLiteralExpr>(expr)) {
        for (const auto& e : setLit->elements) scanExpr(e);
        return;
    }
}

void PythonTranspiler::scanStmt(const std::shared_ptr<Stmt>& stmt) {
    if (!stmt) return;
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

void PythonTranspiler::transpileProceduresAndFunctions(std::ostringstream& ss) {
    for (const auto& proc : prog_.procedures) {
        transpileProcedure(proc, ss);
    }
    for (const auto& func : prog_.functions) {
        transpileFunction(func, ss);
    }
}

void PythonTranspiler::transpileProcedure(const ProcedureDecl& proc, std::ostringstream& ss) {
    ss << "def " << pySafeName(proc.name) << "(";
    for (size_t i = 0; i < proc.params.size(); i++) {
        if (i > 0) ss << ", ";
        ss << pySafeName(proc.params[i].name);
    }
    ss << "):\n";
    indentLevel_ = 1;

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
}

void PythonTranspiler::transpileFunction(const FunctionDecl& func, std::ostringstream& ss) {
    ss << "def " << pySafeName(func.name) << "(";
    for (size_t i = 0; i < func.params.size(); i++) {
        if (i > 0) ss << ", ";
        ss << pySafeName(func.params[i].name);
    }
    ss << "):\n";
    indentLevel_ = 1;

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
    if (ts.kind == TypeKind::FileText || ts.kind == TypeKind::FileData) {
        return "None";
    }

    std::string norm = normalize(typeName.empty() ? ts.name : typeName);
    if (norm == "entier") return "0";
    if (norm == "reel") return "0.0";
    if (norm == "booleen") return "False";
    if (norm == "caractere" || norm == "chaine") return "\"\"";

    auto it = typeMap_.find(norm);
    if (it != typeMap_.end()) {
        const auto& td = it->second;
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

void PythonTranspiler::transpileBlock(const std::vector<std::shared_ptr<Stmt>>& stmts, std::ostringstream& ss) {
    for (const auto& stmt : stmts) {
        transpileStmt(stmt, ss);
    }
}

void PythonTranspiler::transpileStmt(const std::shared_ptr<Stmt>& stmt, std::ostringstream& ss) {
    if (!stmt) return;

    if (auto as = std::dynamic_pointer_cast<AssignStmt>(stmt)) {
        std::string vt = normalize(varTypeMap_[normalize(as->target)]);
        auto it = typeMap_.find(vt);
        bool isRec = (it != typeMap_.end() && it->second.typeSpec.kind == TypeKind::Record);
        if (isRec && std::dynamic_pointer_cast<VariableExpr>(as->value)) {
            ss << indent() << pySafeName(as->target) << " = dict(" << transpileExpr(as->value) << ")\n";
        } else {
            ss << indent() << pySafeName(as->target) << " = " << transpileExpr(as->value) << "\n";
        }
        return;
    }
    if (auto aas = std::dynamic_pointer_cast<ArrayAssignStmt>(stmt)) {
        std::string target = aas->targetExpr ? transpileExpr(aas->targetExpr) : pySafeName(aas->target);
        if (aas->index2) {
            ss << indent() << target << "[" << transpileExpr(aas->index) << ", " << transpileExpr(aas->index2) << "] = " << transpileExpr(aas->value) << "\n";
        } else {
            ss << indent() << target << "[" << transpileExpr(aas->index) << "] = " << transpileExpr(aas->value) << "\n";
        }
        return;
    }
    if (auto ma = std::dynamic_pointer_cast<MemberAssignStmt>(stmt)) {
        ss << indent() << transpileExpr(ma->target) << "[\"" << ma->member << "\"] = " << transpileExpr(ma->value) << "\n";
        return;
    }
    if (auto r = std::dynamic_pointer_cast<ReturnStmt>(stmt)) {
        ss << indent() << "return " << (r->value ? transpileExpr(r->value) : "") << "\n";
        return;
    }
    if (auto w = std::dynamic_pointer_cast<WriteStmt>(stmt)) {
        // Check if first argument is a file
        if (!w->args.empty()) {
            if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(w->args[0])) {
                std::string vt = normalize(varTypeMap_[normalize(varExpr->name)]);
                if (vt.find("fichier") != std::string::npos) {
                    std::string fVar = pySafeName(varExpr->name);
                    if (vt.find("texte") != std::string::npos) {
                        ss << indent() << "print(";
                        for (size_t i = 1; i < w->args.size(); i++) {
                            if (i > 1) ss << ", ";
                            ss << transpileExpr(w->args[i]);
                        }
                        ss << ", file=" << fVar << ")\n";
                    } else {
                        // Binary file: dump
                        for (size_t i = 1; i < w->args.size(); i++) {
                            ss << indent() << "dump(" << transpileExpr(w->args[i]) << ", " << fVar << ")\n";
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
            ss << transpileExpr(w->args[i]);
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
            std::string vt = normalize(varTypeMap_[normalize(t0)]);
            if (vt.find("fichier") != std::string::npos) {
                std::string fVar = pySafeName(t0);
                std::string dest = transpileExpr(rd->targets[1].targetExpr);
                if (vt.find("texte") != std::string::npos) {
                    ss << indent() << dest << " = " << fVar << ".read()\n";
                } else {
                    ss << indent() << dest << " = load(" << fVar << ")\n";
                }
                return;
            }
        }

        for (const auto& tgt : rd->targets) {
            std::string dest = transpileExpr(tgt.targetExpr);
            std::string tName = getExprType(tgt.targetExpr);
            ss << indent() << dest << " = " << transpileReadInput(tName) << "\n";
        }
        return;
    }
    if (auto call = std::dynamic_pointer_cast<CallStmt>(stmt)) {
        std::string n = normalize(call->name);
        if (n == "ouvrir") {
            if (call->args.size() >= 3) {
                std::string path = transpileExpr(call->args[0]);
                std::string fVar = transpileExpr(call->args[1]);
                std::string mode = transpileExpr(call->args[2]);
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
                ss << indent() << transpileExpr(call->args[0]) << ".close()\n";
            }
            return;
        }
        if (n == "lire_ligne") {
            if (call->args.size() >= 2) {
                std::string fVar = transpileExpr(call->args[0]);
                std::string dest = transpileExpr(call->args[1]);
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
                    refArgs.push_back(transpileExpr(call->args[i]));
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
                    ss << transpileExpr(call->args[i]);
                }
                ss << ")\n";
                return;
            }
        }

        ss << indent() << pySafeName(call->name) << "(";
        for (size_t i = 0; i < call->args.size(); i++) {
            if (i > 0) ss << ", ";
            ss << transpileExpr(call->args[i]);
        }
        ss << ")\n";
        return;
    }
    if (auto ifs = std::dynamic_pointer_cast<IfStmt>(stmt)) {
        ss << indent() << "if " << transpileExpr(ifs->condition) << ":\n";
        indentLevel_++;
        if (ifs->thenBlock.empty()) ss << indent() << "pass\n";
        else transpileBlock(ifs->thenBlock, ss);
        indentLevel_--;

        for (const auto& eb : ifs->elseIfBlocks) {
            ss << indent() << "elif " << transpileExpr(eb.condition) << ":\n";
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
        std::string start = transpileExpr(fors->start);
        std::string end = transpileExpr(fors->end);
        if (fors->step) {
            if (auto num = std::dynamic_pointer_cast<NumberExpr>(fors->step)) {
                if (num->value > 0) {
                    ss << indent() << "for " << var << " in range(" << start << ", " << end << " + 1, " << num->value << "):\n";
                } else if (num->value < 0) {
                    ss << indent() << "for " << var << " in range(" << start << ", " << end << " - 1, " << num->value << "):\n";
                } else {
                    ss << indent() << "for " << var << " in range(" << start << ", " << end << " + 1):\n";
                }
            } else {
                std::string step = transpileExpr(fors->step);
                ss << indent() << "for " << var << " in range(" << start << ", " << end << " + (1 if (" << step << ") > 0 else -1), " << step << "):\n";
            }
        } else {
            ss << indent() << "for " << var << " in range(" << start << ", " << end << " + 1):\n";
        }
        indentLevel_++;
        if (fors->body.empty()) ss << indent() << "pass\n";
        else transpileBlock(fors->body, ss);
        indentLevel_--;
        return;
    }
    if (auto wh = std::dynamic_pointer_cast<WhileStmt>(stmt)) {
        ss << indent() << "while " << transpileExpr(wh->condition) << ":\n";
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
        ss << indent() << okVar << " = bool(" << transpileExpr(rep->condition) << ")\n";
        indentLevel_--;
        return;
    }
    if (auto sw = std::dynamic_pointer_cast<SwitchStmt>(stmt)) {
        std::string selExpr = transpileExpr(sw->expr);
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
            ss << indent() << (first ? "if " : "elif ");
            first = false;
            bool firstCond = true;
            for (const auto& item : cs.items) {
                if (!firstCond) ss << " or ";
                firstCond = false;
                if (item.isRange()) {
                    ss << "(" << transpileExpr(item.start) << " <= " << selExpr << " <= " << transpileExpr(item.end) << ")";
                } else {
                    ss << "(" << selExpr << " == " << transpileExpr(item.start) << ")";
                }
            }
            for (const auto& val : cs.values) {
                if (!firstCond) ss << " or ";
                firstCond = false;
                ss << "(" << selExpr << " == " << transpileExpr(val) << ")";
            }
            ss << ":\n";
            indentLevel_++;
            if (cs.body.empty()) ss << indent() << "pass\n";
            else transpileBlock(cs.body, ss);
            indentLevel_--;
        }
        return;
    }
}

std::string PythonTranspiler::transpileReadInput(const std::string& typeName) {
    std::string norm = normalize(typeName);
    if (norm == "entier") {
        return "int(input())";
    }
    if (norm == "reel") {
        return "float(input())";
    }
    if (norm == "booleen") {
        return "input().strip().lower() in ('vrai', 'true', '1')";
    }
    return "input()";
}

std::string PythonTranspiler::getExprType(const std::shared_ptr<Expr>& expr) {
    if (!expr) return "chaine";
    if (auto v = std::dynamic_pointer_cast<VariableExpr>(expr)) {
        auto it = varTypeMap_.find(normalize(v->name));
        if (it != varTypeMap_.end()) return it->second;
    }
    if (auto aa = std::dynamic_pointer_cast<ArrayAccessExpr>(expr)) {
        std::string n = aa->target ? "" : aa->name;
        if (!n.empty()) {
            auto it = varTypeMap_.find(normalize(n));
            if (it != varTypeMap_.end()) return it->second;
        }
    }
    if (auto mem = std::dynamic_pointer_cast<MemberAccessExpr>(expr)) {
        std::string objType = getExprType(mem->object);
        auto it = typeMap_.find(normalize(objType));
        if (it != typeMap_.end()) {
            for (const auto& fld : it->second.typeSpec.fields) {
                for (const auto& fname : fld->names) {
                    if (normalize(fname) == normalize(mem->member)) {
                        return fld->varType;
                    }
                }
            }
        }
    }
    return "chaine";
}

std::string PythonTranspiler::transpileExpr(const std::shared_ptr<Expr>& expr) {
    if (!expr) return "None";

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
        return transpileExpr(mem->object) + "[\"" + mem->member + "\"]";
    }
    if (auto aa = std::dynamic_pointer_cast<ArrayAccessExpr>(expr)) {
        std::string target = aa->target ? transpileExpr(aa->target) : pySafeName(aa->name);
        if (aa->index2) {
            return target + "[" + transpileExpr(aa->index) + ", " + transpileExpr(aa->index2) + "]";
        }
        return target + "[" + transpileExpr(aa->index) + "]";
    }
    if (auto un = std::dynamic_pointer_cast<UnaryExpr>(expr)) {
        std::string op = normalize(un->op);
        if (op == "non" || op == "not") {
            return "(not " + transpileExpr(un->operand) + ")";
        }
        return "(" + un->op + transpileExpr(un->operand) + ")";
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
        else if (op == "∈") {
            return "(" + transpileExpr(bin->left) + " in " + transpileExpr(bin->right) + ")";
        } else {
            pyOp = op;
        }
        return "(" + transpileExpr(bin->left) + " " + pyOp + " " + transpileExpr(bin->right) + ")";
    }
    if (auto setLit = std::dynamic_pointer_cast<SetLiteralExpr>(expr)) {
        std::string s = "[";
        for (size_t i = 0; i < setLit->elements.size(); i++) {
            if (i > 0) s += ", ";
            s += transpileExpr(setLit->elements[i]);
        }
        s += "]";
        return s;
    }
    if (auto call = std::dynamic_pointer_cast<CallExpr>(expr)) {
        std::string n = normalize(call->name);

        if (n == "racine") {
            return "sqrt(" + transpileExpr(call->args[0]) + ")";
        }
        if (n == "exp") {
            return "exp(" + transpileExpr(call->args[0]) + ")";
        }
        if (n == "ln" || n == "log") {
            return "log(" + transpileExpr(call->args[0]) + ")";
        }
        if (n == "abs") {
            return "abs(" + transpileExpr(call->args[0]) + ")";
        }
        if (n == "ent") {
            return "int(" + transpileExpr(call->args[0]) + ")";
        }
        if (n == "arrondi") {
            return "round(" + transpileExpr(call->args[0]) + ")";
        }
        if (n == "carre") {
            return "((" + transpileExpr(call->args[0]) + ") ** 2)";
        }
        if (n == "long") {
            return "len(" + transpileExpr(call->args[0]) + ")";
        }
        if (n == "chr") {
            return "chr(" + transpileExpr(call->args[0]) + ")";
        }
        if (n == "ord") {
            return "ord(" + transpileExpr(call->args[0]) + ")";
        }
        if (n == "convch") {
            return "str(" + transpileExpr(call->args[0]) + ")";
        }
        if (n == "valeur") {
            std::string a0 = transpileExpr(call->args[0]);
            return "(float(" + a0 + ") if ('.' in str(" + a0 + ") or 'e' in str(" + a0 + ").lower()) else int(" + a0 + "))";
        }
        if (n == "sous_chaine" || n == "souschaine") {
            std::string s = transpileExpr(call->args[0]);
            std::string d = transpileExpr(call->args[1]);
            std::string f = transpileExpr(call->args[2]);
            return "str(" + s + ")[" + d + ":" + f + "]";
        }
        if (n == "pos") {
            return transpileExpr(call->args[1]) + ".find(" + transpileExpr(call->args[0]) + ")";
        }
        if (n == "majus") {
            return "str(" + transpileExpr(call->args[0]) + ").upper()";
        }
        if (n == "minus") {
            return "str(" + transpileExpr(call->args[0]) + ").lower()";
        }
        if (n == "alea" || n == "aléa") {
            return "randint(" + transpileExpr(call->args[0]) + ", " + transpileExpr(call->args[1]) + ")";
        }
        if (n == "graine") {
            return "seed(" + transpileExpr(call->args[0]) + ")";
        }
        if (n == "fin_fichier") {
            std::string fVar = transpileExpr(call->args[0]);
            return "(" + fVar + ".tell() >= fstat(" + fVar + ".fileno()).st_size)";
        }
        if (n == "sin") {
            return "sin(" + transpileExpr(call->args[0]) + ")";
        }
        if (n == "cos") {
            return "cos(" + transpileExpr(call->args[0]) + ")";
        }
        if (n == "tan") {
            return "tan(" + transpileExpr(call->args[0]) + ")";
        }
        if (n == "arctan" || n == "atan") {
            return "atan(" + transpileExpr(call->args[0]) + ")";
        }
        if (n == "frac") {
            return "((" + transpileExpr(call->args[0]) + ") % 1)";
        }
        if (n == "puissance") {
            return "((" + transpileExpr(call->args[0]) + ") ** (" + transpileExpr(call->args[1]) + "))";
        }
        if (n == "inserer") {
            std::string c1 = transpileExpr(call->args[0]);
            std::string c2 = transpileExpr(call->args[1]);
            std::string p = transpileExpr(call->args[2]);
            return "(" + c2 + "[:" + p + "] + " + c1 + " + " + c2 + "[" + p + ":])";
        }

        std::string s = pySafeName(call->name) + "(";
        for (size_t i = 0; i < call->args.size(); i++) {
            if (i > 0) s += ", ";
            s += transpileExpr(call->args[i]);
        }
        s += ")";
        return s;
    }

    return "None";
}

} // namespace algo
