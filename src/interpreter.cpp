#include "../include/interpreter.hpp"
#include <regex>
#include <filesystem>
#include <sstream>

namespace algo {

InterpreterOptions InterpreterOptions::fromSource(const std::string& source) {
    return fromSource(source, InterpreterOptions{});
}

InterpreterOptions InterpreterOptions::fromSource(const std::string& source, const InterpreterOptions& base) {
    InterpreterOptions opts = base;
    std::istringstream iss(source);
    std::string line;
    while (std::getline(iss, line)) {
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        std::string trimmed = line.substr(start);

        if (trimmed.rfind("//", 0) != 0) {
            // Arrêter la lecture dès la première ligne qui n'est pas un commentaire
            break;
        }

        std::string comment = trimmed.substr(2);

        auto findDirective = [&](const char* const* keys, auto& target) {
            for (int i = 0; keys[i] != nullptr; ++i) {
                std::string k = keys[i];
                size_t p = comment.find(k);
                if (p != std::string::npos) {
                    size_t colon = comment.find(':', p + k.size());
                    if (colon != std::string::npos) {
                        std::string valStr = comment.substr(colon + 1);
                        size_t vStart = valStr.find_first_not_of(" \t\r\n");
                        if (vStart != std::string::npos) {
                            try {
                                target = std::stoull(valStr.substr(vStart));
                            } catch (...) {}
                        }
                    }
                    return;
                }
            }
        };

        static const char* const maxIterKeys[] = {"@max-iter", "@max_iter", "@max-iterations", "@max_iterations", nullptr};
        static const char* const timeoutKeys[] = {"@timeout", "@timeout-ms", "@timeout_ms", nullptr};
        findDirective(maxIterKeys, opts.maxIterations);
        findDirective(timeoutKeys, opts.timeoutMs);

        size_t sp = comment.find("@seed");
        if (sp != std::string::npos) {
            size_t colon = comment.find(':', sp + 5);
            if (colon != std::string::npos) {
                std::string valStr = comment.substr(colon + 1);
                size_t vStart = valStr.find_first_not_of(" \t\r\n");
                if (vStart != std::string::npos) {
                    try {
                        opts.seed = (uint32_t)std::stoul(valStr.substr(vStart));
                    } catch (...) {}
                }
            }
        }
    }
    return opts;
}

Interpreter::Interpreter(InterpreterOptions options)
    : options_(options), globalEnv_(std::make_shared<Environment>()), rng_(options.seed) {}

void Interpreter::checkIterationLimit(int line) {
    iterationCount_++;
    if (iterationCount_ > options_.maxIterations) {
        throw AlgoError("Limite d'itérations atteinte", line);
    }
}

void Interpreter::checkTimeout(int line) {
    if ((iterationCount_ & 255) != 0) return;
    auto now = std::chrono::steady_clock::now();
    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime_).count();
    if ((uint64_t)elapsedMs > options_.timeoutMs) {
        throw AlgoError("Timeout", line);
    }
}

void Interpreter::addOutput(const std::string& str) {
    output_.push_back(str);
    if (outputCallback_) {
        outputCallback_(str);
    }
}

std::string Interpreter::requestInput(const std::string& prompt) {
    if (inputBufferIdx_ < inputBuffer_.size()) {
        return inputBuffer_[inputBufferIdx_++];
    }
    if (inputCallback_) {
        return inputCallback_(prompt);
    }
    std::string line;
    if (std::getline(std::cin, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        return line;
    }
    return "";
}

Value Interpreter::convertInput(const std::string& inputStr, const Value& currentVal) {
    std::string trimmed = inputStr;
    size_t start = trimmed.find_first_not_of(" \t\r\n");
    size_t end = trimmed.find_last_not_of(" \t\r\n");
    if (start != std::string::npos && end != std::string::npos) {
        trimmed = trimmed.substr(start, end - start + 1);
    } else {
        trimmed = "";
    }

    if (currentVal.isInt()) {
        try {
            return Value((int64_t)std::stoll(trimmed));
        } catch (...) {
            return Value((int64_t)0);
        }
    }
    if (currentVal.isReal()) {
        try {
            return Value((double)std::stod(trimmed));
        } catch (...) {
            return Value((double)0.0);
        }
    }
    if (currentVal.isBool()) {
        std::string low = toLower(trimmed);
        return Value(low == "vrai" || low == "true" || low == "1");
    }
    std::string s = inputStr;
    if (!s.empty() && s.back() == '\r') s.pop_back();
    return Value(s);
}

Value Interpreter::getDefaultValue(const TypeSpec& ts, const std::string& varName) const {
    if (ts.kind == TypeKind::Pointer) {
        return Value::makeNil();
    }
    if (ts.kind == TypeKind::Array2D) {
        Value elemDef = getDefaultValue(ts.elemType);
        std::vector<Value> row(ts.cols, elemDef);
        std::vector<Value> mat(ts.rows, Value(row));
        return Value(std::move(mat));
    }
    if (ts.kind == TypeKind::Array1D) {
        Value elemDef = getDefaultValue(ts.elemType);
        std::vector<Value> arr(ts.rows, elemDef);
        return Value(std::move(arr));
    }
    if (ts.kind == TypeKind::Record) {
        std::map<std::string, Value> fields;
        for (const auto& fld : ts.fields) {
            for (const auto& name : fld->names) {
                fields[name] = getDefaultValue(fld->typeSpec, name);
            }
        }
        return Value(std::move(fields));
    }
    if (ts.kind == TypeKind::FileText || ts.kind == TypeKind::FileData) {
        Value v;
        v.type = ValueType::FileHandle;
        v.fileLogicalName = varName;
        return v;
    }
    // Simple
    return getDefaultValue(ts.name.empty() ? ts.elemType : ts.name);
}

Value Interpreter::getDefaultValue(const std::string& typeName) const {
    std::string norm = normalize(typeName);
    if (!norm.empty() && norm[0] == '^') return Value::makeNil();
    if (norm.find("pointeur") == 0) return Value::makeNil();
    if (norm == "entier") return Value((int64_t)0);
    if (norm == "reel") return Value((double)0.0);
    if (norm == "booleen") return Value(false);
    if (norm == "caractere") return Value("");
    if (norm == "chaine") return Value("");

    auto it = typeDefs_.find(norm);
    if (it != typeDefs_.end()) {
        const auto& td = it->second;
        if (td.typeSpec.kind == TypeKind::Pointer) {
            return Value::makeNil();
        }
        if (td.typeSpec.kind != TypeKind::Simple) {
            return getDefaultValue(td.typeSpec);
        }
        Value elemDef = getDefaultValue(td.elemType);
        std::vector<Value> arr(td.arraySize, elemDef);
        return Value(std::move(arr));
    }
    return Value();
}

void Interpreter::run(const Program& prog) {
    output_.clear();
    inputBufferIdx_ = 0;
    iterationCount_ = 0;
    returnValue_ = {false, Value()};
    startTime_ = std::chrono::steady_clock::now();
    globalEnv_ = std::make_shared<Environment>();
    typeDefs_.clear();
    procedures_.clear();
    functions_.clear();
    varDecls_ = prog.varDecls;
    currentSubroutine_.clear();
    openFiles_.clear();
    enumOrdinals_.clear();

    for (const auto& td : prog.typeDecls) {
        typeDefs_[normalize(td.name)] = td;
        if (td.typeSpec.kind == TypeKind::Enum) {
            for (size_t i = 0; i < td.typeSpec.enumValues.size(); i++) {
                const std::string& ev = td.typeSpec.enumValues[i];
                enumOrdinals_[normalize(ev)] = (int64_t)i;
                globalEnv_->define(ev, Value(ev));
            }
        }
    }
    for (const auto& proc : prog.procedures) {
        procedures_[normalize(proc.name)] = proc;
    }
    for (const auto& func : prog.functions) {
        functions_[normalize(func.name)] = func;
    }
    for (const auto& cd : prog.constDecls) {
        Value cVal = evaluateExpression(cd.valueExpr, globalEnv_);
        globalEnv_->define(cd.name, cVal);
    }
    for (const auto& vd : prog.varDecls) {
        std::string vtNorm = normalize(vd.varType);
        if (vtNorm == "procedure" || vtNorm == "fonction" || vd.typeSpec.name == "procedure" || vd.typeSpec.name == "fonction") continue;
        for (const auto& name : vd.names) {
            Value defVal;
            if (vd.typeSpec.kind != TypeKind::Simple || !vd.typeSpec.name.empty()) {
                defVal = getDefaultValue(vd.typeSpec, name);
            } else {
                defVal = getDefaultValue(vd.varType);
            }
            globalEnv_->define(name, defVal);
        }
    }

    if (!prog.mainBody.empty()) {
        executeBlock(prog.mainBody, globalEnv_);
        return;
    }

    // Check if procedure 'main' or 'principal' exists
    if (procedures_.find("main") != procedures_.end()) {
        executeProcedure("main", {}, globalEnv_);
    } else if (procedures_.find("principal") != procedures_.end()) {
        executeProcedure("principal", {}, globalEnv_);
    }
}

void Interpreter::executeBlock(const std::vector<std::shared_ptr<Stmt>>& stmts, const std::shared_ptr<Environment>& env) {
    for (const auto& stmt : stmts) {
        if (returnValue_.set) return;
        executeStatement(stmt, env);
    }
}

void Interpreter::assignToLValue(const std::shared_ptr<Expr>& lhs, const Value& val, const std::shared_ptr<Environment>& env, int line) {
    if (!lhs) throw AlgoError("Cible d'affectation non valide", line);
    if (lhs->kind == ExprKind::Variable) {
        auto* varExpr = static_cast<VariableExpr*>(lhs.get());
        env->set(varExpr->name, val, line > 0 ? line : varExpr->line);
        return;
    }
    if (lhs->kind == ExprKind::Deref) {
        auto* derefExpr = static_cast<DerefExpr*>(lhs.get());
        Value ptr = evaluateExpression(derefExpr->target, env);
        if (!ptr.isPointer() || !ptr.ptrVal) {
            throw AlgoError("Déréférencement d'un pointeur NIL ou non alloué", line > 0 ? line : derefExpr->line);
        }
        *(ptr.ptrVal) = val;
        return;
    }
    if (lhs->kind == ExprKind::MemberAccess) {
        auto* memExpr = static_cast<MemberAccessExpr*>(lhs.get());
        Value obj = evaluateExpression(memExpr->object, env);
        if (obj.isPointer()) {
            if (!obj.ptrVal) {
                throw AlgoError("Déréférencement d'un pointeur NIL ou non alloué", line > 0 ? line : memExpr->line);
            }
            if (!obj.ptrVal->isRecord()) {
                throw AlgoError("Accès à un champ sur un objet qui n'est pas un enregistrement", line > 0 ? line : memExpr->line);
            }
            obj.ptrVal->recVal[memExpr->member] = val;
            return;
        }
        if (!obj.isRecord()) {
            throw AlgoError("Accès à un champ sur un objet qui n'est pas un enregistrement", line > 0 ? line : memExpr->line);
        }
        obj.recVal[memExpr->member] = val;
        assignToLValue(memExpr->object, obj, env, line);
        return;
    }
    if (lhs->kind == ExprKind::ArrayAccess) {
        auto* arrExpr = static_cast<ArrayAccessExpr*>(lhs.get());
        Value arr;
        if (arrExpr->target) {
            arr = evaluateExpression(arrExpr->target, env);
        } else {
            arr = env->get(arrExpr->name, line > 0 ? line : arrExpr->line);
        }
        if (!arr.isArray()) {
            throw AlgoError((arrExpr->target ? "L'expression" : arrExpr->name) + " n'est pas un tableau", line > 0 ? line : arrExpr->line);
        }
        int64_t idx = evaluateExpression(arrExpr->index, env).asInt();
        if (idx < 0 || (size_t)idx >= arr.arrVal.size()) {
            throw AlgoError("Indice " + std::to_string(idx) + " hors bornes", line > 0 ? line : arrExpr->line);
        }

        if (arrExpr->index2) {
            Value row = arr.arrVal[idx];
            if (!row.isArray()) {
                throw AlgoError("Ligne " + std::to_string(idx) + " n'est pas un tableau", line > 0 ? line : arrExpr->line);
            }
            int64_t col = evaluateExpression(arrExpr->index2, env).asInt();
            if (col < 0 || (size_t)col >= row.arrVal.size()) {
                throw AlgoError("Colonne " + std::to_string(col) + " hors bornes", line > 0 ? line : arrExpr->line);
            }
            row.arrVal[col] = val;
            arr.arrVal[idx] = row;
        } else {
            arr.arrVal[idx] = val;
        }

        if (arrExpr->target) {
            assignToLValue(arrExpr->target, arr, env, line);
        } else {
            env->set(arrExpr->name, arr, line > 0 ? line : arrExpr->line);
        }
        return;
    }
    throw AlgoError("Cible d'affectation non valide", line);
}

void Interpreter::writeFileData(AlgoFile& f, const Value& val) {
    std::streampos curPos = f.stream.tellp();
    if (curPos != (std::streampos)-1) {
        if (f.currentRecordIndex >= (int64_t)f.recordOffsets.size()) {
            f.recordOffsets.push_back(curPos);
        } else {
            f.recordOffsets[f.currentRecordIndex] = curPos;
        }
        f.currentRecordIndex++;
    }
    if (val.isInt()) {
        f.stream << "I " << val.asInt() << "\n";
    } else if (val.isReal()) {
        f.stream << "R " << val.asReal() << "\n";
    } else if (val.isBool()) {
        f.stream << "B " << (val.asBool() ? "1" : "0") << "\n";
    } else if (val.isString()) {
        f.stream << "S " << val.asString() << "\n";
    } else if (val.isArray()) {
        f.stream << "ARR " << val.arrVal.size() << "\n";
        for (const auto& item : val.arrVal) {
            writeFileData(f, item);
        }
        f.stream << "ENDARR\n";
    } else if (val.isRecord()) {
        f.stream << "REC " << val.recVal.size() << "\n";
        for (const auto& kv : val.recVal) {
            f.stream << "FLD " << kv.first << " ";
            writeFileData(f, kv.second);
        }
        f.stream << "ENDREC\n";
    } else {
        f.stream << "N 0\n";
    }
}

Value Interpreter::readFileData(AlgoFile& f) {
    std::string tag;
    f.stream >> std::ws;
    if (!(f.stream >> tag)) {
        return Value();
    }
    if (tag == "I") {
        int64_t v;
        f.stream >> v;
        return Value(v);
    }
    if (tag == "R") {
        double v;
        f.stream >> v;
        return Value(v);
    }
    if (tag == "B") {
        int v;
        f.stream >> v;
        return Value(v != 0);
    }
    if (tag == "S") {
        f.stream.get(); // skip space
        std::string s;
        std::getline(f.stream, s);
        if (!s.empty() && s.back() == '\r') s.pop_back();
        return Value(s);
    }
    if (tag == "REC") {
        size_t count;
        f.stream >> count;
        std::map<std::string, Value> fields;
        while (f.stream >> tag) {
            if (tag == "ENDREC") break;
            if (tag == "FLD") {
                std::string fldName;
                f.stream >> fldName;
                Value fldVal = readFileData(f);
                fields[fldName] = fldVal;
            }
        }
        return Value(fields);
    }
    if (tag == "ARR") {
        size_t count;
        f.stream >> count;
        std::vector<Value> items;
        items.reserve(count);
        for (size_t i = 0; i < count; i++) {
            items.push_back(readFileData(f));
        }
        std::string endTag;
        f.stream >> endTag; // consume ENDARR
        return Value(items);
    }
    return Value();
}

void Interpreter::executeStatement(const std::shared_ptr<Stmt>& stmt, const std::shared_ptr<Environment>& env) {
    if (!stmt || returnValue_.set) return;
    auto prevEnv = currentEnv_;
    currentEnv_ = env;
    stmt->accept(*this);
    currentEnv_ = prevEnv;
}

void Interpreter::visit(AssignStmt& a) {
    Value val = evaluateExpression(a.value, currentEnv_);
    auto lhs = std::make_shared<VariableExpr>(a.target, a.line);
    assignToLValue(lhs, val, currentEnv_, a.line);
}

void Interpreter::visit(ArrayAssignStmt& aa) {
    Value val = evaluateExpression(aa.value, currentEnv_);
    if (aa.targetExpr) {
        auto lhs = std::make_shared<ArrayAccessExpr>(aa.targetExpr, aa.index, aa.index2, aa.line);
        assignToLValue(lhs, val, currentEnv_, aa.line);
    } else {
        auto lhs = std::make_shared<ArrayAccessExpr>(aa.target, aa.index, aa.index2, aa.line);
        assignToLValue(lhs, val, currentEnv_, aa.line);
    }
}

void Interpreter::visit(MemberAssignStmt& ma) {
    Value val = evaluateExpression(ma.value, currentEnv_);
    auto lhs = std::make_shared<MemberAccessExpr>(ma.target, ma.member, ma.line);
    assignToLValue(lhs, val, currentEnv_, ma.line);
}

std::string Interpreter::resolveExprType(const std::shared_ptr<Expr>& expr) const {
    if (!expr) return "";
    if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(expr)) {
        std::string vNorm = normalize(varExpr->name);
        if (!currentSubroutine_.empty()) {
            auto pit = procedures_.find(currentSubroutine_);
            if (pit != procedures_.end()) {
                for (const auto& p : pit->second.params) {
                    if (normalize(p.name) == vNorm) return p.paramType;
                }
                for (const auto& vd : pit->second.localVars) {
                    for (const auto& n : vd.names) {
                        if (normalize(n) == vNorm) return vd.varType;
                    }
                }
            }
            auto fit = functions_.find(currentSubroutine_);
            if (fit != functions_.end()) {
                for (const auto& p : fit->second.params) {
                    if (normalize(p.name) == vNorm) return p.paramType;
                }
                for (const auto& vd : fit->second.localVars) {
                    for (const auto& n : vd.names) {
                        if (normalize(n) == vNorm) return vd.varType;
                    }
                }
            }
        }
        for (const auto& vd : varDecls_) {
            for (const auto& n : vd.names) {
                if (normalize(n) == vNorm) return vd.varType;
            }
        }
        return "";
    }
    if (auto mem = std::dynamic_pointer_cast<MemberAccessExpr>(expr)) {
        std::string objType = resolveExprType(mem->object);
        if (!objType.empty() && objType[0] == '^') objType = objType.substr(1);
        auto it = typeDefs_.find(normalize(objType));
        if (it != typeDefs_.end()) {
            for (const auto& f : it->second.typeSpec.fields) {
                for (const auto& n : f->names) {
                    if (normalize(n) == normalize(mem->member)) return f->varType;
                }
            }
        }
        return "";
    }
    if (auto deref = std::dynamic_pointer_cast<DerefExpr>(expr)) {
        std::string t = resolveExprType(deref->target);
        if (!t.empty() && t[0] == '^') return t.substr(1);
        auto it = typeDefs_.find(normalize(t));
        if (it != typeDefs_.end() && it->second.typeSpec.kind == TypeKind::Pointer) {
            return it->second.typeSpec.elemType;
        }
        return "";
    }
    return "";
}

std::string Interpreter::resolvePointeeType(const std::shared_ptr<Expr>& expr) const {
    std::string t = resolveExprType(expr);
    if (!t.empty() && t[0] == '^') return t.substr(1);
    std::string tNorm = normalize(t);
    if (tNorm.find("pointeur sur ") == 0) return t.substr(13);
    if (tNorm.find("pointeur de ") == 0) return t.substr(12);
    auto it = typeDefs_.find(tNorm);
    if (it != typeDefs_.end() && it->second.typeSpec.kind == TypeKind::Pointer) {
        return it->second.typeSpec.elemType;
    }
    return t;
}

void Interpreter::visit(DerefAssignStmt& da) {
    checkIterationLimit(da.line);
    checkTimeout(da.line);
    Value val = evaluateExpression(da.value, currentEnv_);
    auto lhs = std::make_shared<DerefExpr>(da.target, da.line);
    assignToLValue(lhs, val, currentEnv_, da.line);
}

void Interpreter::visit(AllocStmt& node) {
    checkIterationLimit(node.line);
    checkTimeout(node.line);
    std::string pointeeType = resolvePointeeType(node.target);
    Value initialVal = getDefaultValue(pointeeType);
    Value newPtr = Value::makePointer(std::make_shared<Value>(std::move(initialVal)));
    assignToLValue(node.target, newPtr, currentEnv_, node.line);
}

void Interpreter::visit(FreeStmt& node) {
    checkIterationLimit(node.line);
    checkTimeout(node.line);
    assignToLValue(node.target, Value::makeNil(), currentEnv_, node.line);
}

void Interpreter::visit(WriteStmt& w) {
    bool isFileWrite = false;
    std::string fileLogicalName;
    if (!w.args.empty()) {
        if (w.args[0]->kind == ExprKind::Variable) {
            auto* varExpr = static_cast<VariableExpr*>(w.args[0].get());
            if (currentEnv_->has(varExpr->name)) {
                Value v = currentEnv_->get(varExpr->name);
                if (v.isFile()) {
                    isFileWrite = true;
                    fileLogicalName = normalize(varExpr->name);
                }
            }
        }
    }

    if (isFileWrite) {
        auto it = openFiles_.find(fileLogicalName);
        if (it == openFiles_.end() || !it->second->isOpen) {
            throw AlgoError("Fichier '" + fileLogicalName + "' non ouvert", w.line);
        }
        auto& f = *(it->second);
        if (f.isText) {
            for (size_t i = 1; i < w.args.size(); i++) {
                Value v = evaluateExpression(w.args[i], currentEnv_);
                f.stream << v.asString();
            }
            if (w.isNl) {
                f.stream << "\n";
            }
            f.stream.flush();
        } else {
            for (size_t i = 1; i < w.args.size(); i++) {
                Value v = evaluateExpression(w.args[i], currentEnv_);
                writeFileData(f, v);
            }
            f.stream.flush();
        }
        return;
    }

    std::string lineStr;
    for (size_t i = 0; i < w.args.size(); i++) {
        Value val = evaluateExpression(w.args[i], currentEnv_);
        if (val.isBool()) {
            lineStr += val.asBool() ? "Vrai" : "Faux";
        } else {
            lineStr += val.asString();
        }
        if (i + 1 < w.args.size()) {
            lineStr += w.sep;
        }
    }
    lineStr += w.fin;
    addOutput(lineStr);
}

void Interpreter::visit(ReadStmt& r) {
    if (r.targets.size() == 2) {
        std::string t0Name = r.targets[0].name;
        if (currentEnv_->has(t0Name)) {
            Value v = currentEnv_->get(t0Name);
            if (v.isFile()) {
                std::string fNorm = normalize(t0Name);
                auto it = openFiles_.find(fNorm);
                if (it == openFiles_.end() || !it->second->isOpen) {
                    throw AlgoError("Fichier '" + t0Name + "' non ouvert", r.line);
                }
                auto& f = *(it->second);
                if (f.isText) {
                    std::ostringstream ss;
                    ss << f.stream.rdbuf();
                    std::string content = ss.str();
                    assignToLValue(r.targets[1].targetExpr, Value(content), currentEnv_, r.line);
                } else {
                    Value val = readFileData(f);
                    f.currentRecordIndex++;
                    assignToLValue(r.targets[1].targetExpr, val, currentEnv_, r.line);
                }
                return;
            }
        }
    }

    for (const auto& tgt : r.targets) {
        Value currentVal = evaluateExpression(tgt.targetExpr, currentEnv_);
        std::string inputStr = requestInput("Saisir une valeur :");
        Value converted = convertInput(inputStr, currentVal);
        assignToLValue(tgt.targetExpr, converted, currentEnv_, r.line);
    }
}

void Interpreter::visit(IfStmt& ifs) {
    if (evaluateExpression(ifs.condition, currentEnv_).asBool()) {
        executeBlock(ifs.thenBlock, currentEnv_);
    } else {
        bool matched = false;
        for (const auto& eb : ifs.elseIfBlocks) {
            if (evaluateExpression(eb.condition, currentEnv_).asBool()) {
                executeBlock(eb.body, currentEnv_);
                matched = true;
                break;
            }
        }
        if (!matched && !ifs.elseBlock.empty()) {
            executeBlock(ifs.elseBlock, currentEnv_);
        }
    }
}

void Interpreter::visit(ForStmt& fors) {
    Value startVal = evaluateExpression(fors.start, currentEnv_);
    Value endVal = evaluateExpression(fors.end, currentEnv_);
    Value stepVal = fors.step ? evaluateExpression(fors.step, currentEnv_) : Value((int64_t)1);

    if (!startVal.isNumber() || !endVal.isNumber() || !stepVal.isNumber()) {
        throw AlgoError("Les bornes de la boucle Pour doivent être des nombres", fors.line);
    }
    int64_t step = stepVal.asInt();
    if (step == 0) {
        throw AlgoError("Le pas de la boucle Pour ne peut pas être nul", fors.line);
    }

    int64_t start = startVal.asInt();
    int64_t end = endVal.asInt();

    if (step > 0) {
        for (int64_t i = start; i <= end; i += step) {
            checkIterationLimit(fors.line);
            checkTimeout(fors.line);
            currentEnv_->set(fors.varName, Value(i), fors.line);
            executeBlock(fors.body, currentEnv_);
            if (returnValue_.set) return;
        }
    } else {
        for (int64_t i = start; i >= end; i += step) {
            checkIterationLimit(fors.line);
            checkTimeout(fors.line);
            currentEnv_->set(fors.varName, Value(i), fors.line);
            executeBlock(fors.body, currentEnv_);
            if (returnValue_.set) return;
        }
    }
}

void Interpreter::visit(WhileStmt& whiles) {
    while (evaluateExpression(whiles.condition, currentEnv_).asBool()) {
        checkIterationLimit(whiles.line);
        checkTimeout(whiles.line);
        executeBlock(whiles.body, currentEnv_);
        if (returnValue_.set) return;
    }
}

void Interpreter::visit(RepeatStmt& reps) {
    do {
        checkIterationLimit(reps.line);
        checkTimeout(reps.line);
        executeBlock(reps.body, currentEnv_);
        if (returnValue_.set) return;
    } while (!evaluateExpression(reps.condition, currentEnv_).asBool());
}

void Interpreter::visit(CallStmt& calls) {
    std::string n = normalize(calls.name);
    if (n == "ouvrir" || n == "fermer" || n == "lire_ligne" || n == "aller_a" || n == "pointer" || procedures_.find(n) != procedures_.end()) {
        executeProcedure(n, calls.args, currentEnv_, calls.line);
    } else if (functions_.find(n) != functions_.end()) {
        executeFunction(n, calls.args, currentEnv_, calls.line);
    } else if (n == "graine") {
        evaluateCall(n, calls.args, currentEnv_, calls.line);
    } else {
        throw AlgoError("Procédure '" + calls.name + "' non définie", calls.line);
    }
}

void Interpreter::visit(SwitchStmt& sws) {
    Value targetVal = evaluateExpression(sws.expr, currentEnv_);
    bool matched = false;
    const SwitchCase* defaultCase = nullptr;

    for (const auto& sc : sws.cases) {
        if (sc.isDefault) {
            defaultCase = &sc;
            continue;
        }
        if (!sc.items.empty()) {
            for (const auto& item : sc.items) {
                if (item.isRange()) {
                    Value startVal = evaluateExpression(item.start, currentEnv_);
                    Value endVal = evaluateExpression(item.end, currentEnv_);
                    bool inRange = false;
                    if (startVal.lessOrEqual(endVal)) {
                        inRange = targetVal.greaterOrEqual(startVal) && targetVal.lessOrEqual(endVal);
                    } else {
                        inRange = targetVal.greaterOrEqual(endVal) && targetVal.lessOrEqual(startVal);
                    }
                    if (inRange) {
                        matched = true;
                        executeBlock(sc.body, currentEnv_);
                        break;
                    }
                } else {
                    Value caseVal = evaluateExpression(item.start, currentEnv_);
                    if (targetVal.equals(caseVal)) {
                        matched = true;
                        executeBlock(sc.body, currentEnv_);
                        break;
                    }
                }
            }
        } else {
            for (const auto& valExpr : sc.values) {
                Value caseVal = evaluateExpression(valExpr, currentEnv_);
                if (targetVal.equals(caseVal)) {
                    matched = true;
                    executeBlock(sc.body, currentEnv_);
                    break;
                }
            }
        }
        if (matched) break;
    }

    if (!matched && defaultCase != nullptr) {
        executeBlock(defaultCase->body, currentEnv_);
    }
}

void Interpreter::visit(ReturnStmt& rets) {
    returnValue_ = {true, evaluateExpression(rets.value, currentEnv_)};
}

void Interpreter::visit(LocalVarDeclStmt& lvd) {
    for (const auto& name : lvd.names) {
        Value defVal = getDefaultValue(lvd.typeSpec, name);
        currentEnv_->define(name, defVal);
    }
}

void Interpreter::executeProcedure(const std::string& name, const std::vector<std::shared_ptr<Expr>>& args,
                                   const std::shared_ptr<Environment>& callerEnv, int line) {
    std::string n = normalize(name);

    if (n == "ouvrir") {
        if (args.size() < 3) throw AlgoError("Ouvrir attend 3 arguments: chemin, fichier, mode", line);
        std::string path = evaluateExpression(args[0], callerEnv).asString();
        std::string fVar;
        if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(args[1])) {
            fVar = varExpr->name;
        } else {
            throw AlgoError("Le second argument d'Ouvrir doit être une variable fichier", line);
        }
        std::string mode = evaluateExpression(args[2], callerEnv).asString();
        std::string fNorm = normalize(fVar);

        auto fileIt = openFiles_.find(fNorm);
        if (fileIt != openFiles_.end() && fileIt->second->isOpen) {
            fileIt->second->stream.close();
            fileIt->second->isOpen = false;
        }

        auto file = std::make_shared<AlgoFile>();
        file->path = path;
        file->mode = mode;
        file->isText = (mode == "r" || mode == "w" || mode == "a");

        std::ios_base::openmode m;
        if (mode == "r") m = std::ios_base::in;
        else if (mode == "w") m = std::ios_base::out | std::ios_base::trunc;
        else if (mode == "a") m = std::ios_base::out | std::ios_base::app;
        else if (mode == "rb") m = std::ios_base::in | std::ios_base::binary;
        else if (mode == "wb") m = std::ios_base::out | std::ios_base::trunc | std::ios_base::binary;
        else if (mode == "ab") m = std::ios_base::out | std::ios_base::app | std::ios_base::binary;
        else throw AlgoError("Mode de fichier invalide: '" + mode + "'", line);

#ifdef _WIN32
        file->stream.open(utf8Path(path), m);
#else
        file->stream.open(path, m);
#endif
        if (!file->stream.is_open()) {
            throw AlgoError("Impossible d'ouvrir le fichier '" + path + "'", line);
        }
        file->isOpen = true;
        file->recordOffsets.clear();
        file->currentRecordIndex = 0;
        if (mode == "rb") {
            while (true) {
                file->stream >> std::ws;
                std::streampos p = file->stream.tellg();
                if (p == (std::streampos)-1 || file->stream.peek() == std::char_traits<char>::eof()) break;
                file->recordOffsets.push_back(p);
                readFileData(*file);
                if (file->stream.fail()) break;
            }
            file->stream.clear();
            file->stream.seekg(0, std::ios::beg);
            file->currentRecordIndex = 0;
        }
        openFiles_[fNorm] = file;

        Value fVal;
        fVal.type = ValueType::FileHandle;
        fVal.fileLogicalName = fVar;
        callerEnv->set(fVar, fVal);
        return;
    }

    if (n == "aller_a" || n == "pointer") {
        if (args.size() < 2) throw AlgoError("Aller_A attend 2 arguments: fichier, position", line);
        std::string fVar;
        if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(args[0])) fVar = varExpr->name;
        else fVar = evaluateExpression(args[0], callerEnv).asString();
        std::string fNorm = normalize(fVar);
        auto it = openFiles_.find(fNorm);
        if (it == openFiles_.end() || !it->second->isOpen) {
            throw AlgoError("Fichier '" + fVar + "' non ouvert", line);
        }
        auto& f = *(it->second);
        int64_t targetPos = evaluateExpression(args[1], callerEnv).asInt();
        if (targetPos < 0 || targetPos > (int64_t)f.recordOffsets.size()) {
            throw AlgoError("Position fichier hors bornes", line);
        }
        f.currentRecordIndex = targetPos;
        f.stream.clear();
        if (targetPos < (int64_t)f.recordOffsets.size()) {
            f.stream.seekg(f.recordOffsets[targetPos]);
            f.stream.seekp(f.recordOffsets[targetPos]);
        } else {
            f.stream.seekg(0, std::ios::end);
            f.stream.seekp(0, std::ios::end);
        }
        return;
    }

    if (n == "fermer") {
        if (args.empty()) throw AlgoError("Fermer attend 1 argument: fichier", line);
        std::string fVar;
        if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(args[0])) {
            fVar = varExpr->name;
        } else {
            fVar = evaluateExpression(args[0], callerEnv).asString();
        }
        std::string fNorm = normalize(fVar);
        auto fileIt = openFiles_.find(fNorm);
        if (fileIt != openFiles_.end() && fileIt->second->isOpen) {
            fileIt->second->stream.close();
            fileIt->second->isOpen = false;
        }
        return;
    }

    if (n == "lire_ligne") {
        if (args.size() < 2) throw AlgoError("Lire_ligne attend 2 arguments: fichier, variable", line);
        std::string fVar;
        if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(args[0])) {
            fVar = varExpr->name;
        } else {
            throw AlgoError("Le premier argument de Lire_ligne doit être une variable fichier", line);
        }
        std::string fNorm = normalize(fVar);
        auto fileIt = openFiles_.find(fNorm);
        if (fileIt == openFiles_.end() || !fileIt->second->isOpen) {
            throw AlgoError("Fichier '" + fVar + "' non ouvert", line);
        }
        auto& f = *(fileIt->second);
        std::string lineStr;
        if (std::getline(f.stream, lineStr)) {
            if (!lineStr.empty() && lineStr.back() == '\r') {
                lineStr.pop_back();
            }
        }
        assignToLValue(args[1], Value(lineStr), callerEnv, line);
        return;
    }

    auto it = procedures_.find(n);
    if (it == procedures_.end()) {
        throw AlgoError("Procédure '" + name + "' non définie", line);
    }
    const auto& proc = it->second;
    auto localEnv = std::make_shared<Environment>(globalEnv_);
    std::string oldSub = currentSubroutine_;
    currentSubroutine_ = n;

    // Bind parameters
    for (size_t i = 0; i < proc.params.size(); i++) {
        const auto& p = proc.params[i];
        std::string pName = p.name;
        if (p.byRef && i < args.size()) {
            // Reference parameter
            if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(args[i])) {
                auto binding = callerEnv->getBinding(varExpr->name, varExpr->line);
                localEnv->defineRef(pName, binding.get, binding.set);
                continue;
            }
            auto argExpr = args[i];
            auto envRef = callerEnv;
            localEnv->defineRef(pName,
                [envRef, argExpr, this]() -> Value {
                    return this->evaluateExpression(argExpr, envRef);
                },
                [envRef, argExpr, this](const Value& v) {
                    this->assignToLValue(argExpr, v, envRef);
                }
            );
            continue;
        }

        // Value parameter
        Value val = (i < args.size()) ? evaluateExpression(args[i], callerEnv) : getDefaultValue(p.paramType);
        localEnv->define(pName, val);
    }

    // Local constants
    for (const auto& cd : proc.constDecls) {
        Value val = evaluateExpression(cd.valueExpr, localEnv);
        localEnv->define(cd.name, val);
    }

    // Local variable declarations
    for (const auto& vd : proc.localVars) {
        std::string vtNorm = normalize(vd.varType);
        if (vtNorm == "procedure" || vtNorm == "fonction" || vd.typeSpec.name == "procedure" || vd.typeSpec.name == "fonction") continue;
        for (const auto& varName : vd.names) {
            bool isParam = false;
            for (const auto& p : proc.params) {
                if (normalize(p.name) == normalize(varName)) {
                    isParam = true;
                    break;
                }
            }
            if (isParam) continue;

            Value defVal;
            if (vd.typeSpec.kind != TypeKind::Simple || !vd.typeSpec.name.empty()) {
                defVal = getDefaultValue(vd.typeSpec, varName);
            } else {
                defVal = getDefaultValue(vd.varType);
            }
            localEnv->define(varName, defVal);
        }
    }

    executeBlock(proc.body, localEnv);
    currentSubroutine_ = oldSub;
}

Value Interpreter::executeFunction(const std::string& name, const std::vector<std::shared_ptr<Expr>>& args,
                                   const std::shared_ptr<Environment>& callerEnv, int line) {
    std::string n = normalize(name);
    auto it = functions_.find(n);
    if (it == functions_.end()) {
        throw AlgoError("Fonction '" + name + "' non définie", line);
    }
    const auto& func = it->second;
    auto localEnv = std::make_shared<Environment>(globalEnv_);
    std::string oldSub = currentSubroutine_;
    currentSubroutine_ = n;

    // Bind parameters
    for (size_t i = 0; i < func.params.size(); i++) {
        const auto& p = func.params[i];
        std::string pName = p.name;
        if (p.byRef && i < args.size()) {
            if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(args[i])) {
                auto binding = callerEnv->getBinding(varExpr->name, varExpr->line);
                localEnv->defineRef(pName, binding.get, binding.set);
                continue;
            }
            auto argExpr = args[i];
            auto envRef = callerEnv;
            localEnv->defineRef(pName,
                [envRef, argExpr, this]() -> Value {
                    return this->evaluateExpression(argExpr, envRef);
                },
                [envRef, argExpr, this](const Value& v) {
                    this->assignToLValue(argExpr, v, envRef);
                }
            );
            continue;
        }

        Value val = (i < args.size()) ? evaluateExpression(args[i], callerEnv) : getDefaultValue(p.paramType);
        localEnv->define(pName, val);
    }

    // Local constants
    for (const auto& cd : func.constDecls) {
        Value val = evaluateExpression(cd.valueExpr, localEnv);
        localEnv->define(cd.name, val);
    }

    // Local variable declarations
    for (const auto& vd : func.localVars) {
        std::string vtNorm = normalize(vd.varType);
        if (vtNorm == "procedure" || vtNorm == "fonction" || vd.typeSpec.name == "procedure" || vd.typeSpec.name == "fonction") continue;
        for (const auto& varName : vd.names) {
            bool isParam = false;
            for (const auto& p : func.params) {
                if (normalize(p.name) == normalize(varName)) {
                    isParam = true;
                    break;
                }
            }
            if (isParam) continue;

            Value defVal;
            if (vd.typeSpec.kind != TypeKind::Simple || !vd.typeSpec.name.empty()) {
                defVal = getDefaultValue(vd.typeSpec, varName);
            } else {
                defVal = getDefaultValue(vd.varType);
            }
            localEnv->define(varName, defVal);
        }
    }

    auto oldRet = returnValue_;
    returnValue_ = {false, getDefaultValue(func.returnType)};

    executeBlock(func.body, localEnv);
    currentSubroutine_ = oldSub;

    Value result = returnValue_.value;
    returnValue_ = oldRet;
    return result;
}

Value Interpreter::evaluateExpression(const std::shared_ptr<Expr>& expr, const std::shared_ptr<Environment>& env) {
    if (!expr) return Value();
    auto prevEnv = currentEnv_;
    currentEnv_ = env;
    expr->accept(*this);
    currentEnv_ = prevEnv;
    return evalResult_;
}

void Interpreter::visit(NumberExpr& node) {
    evalResult_ = Value(node.value);
}

void Interpreter::visit(RealExpr& node) {
    evalResult_ = Value(node.value);
}

void Interpreter::visit(StringExpr& node) {
    evalResult_ = Value(node.value);
}

void Interpreter::visit(BoolExpr& node) {
    evalResult_ = Value(node.value);
}

void Interpreter::visit(VariableExpr& node) {
    evalResult_ = currentEnv_->get(node.name, node.line);
}

void Interpreter::visit(ArrayAccessExpr& node) {
    Value arr;
    if (node.target) {
        arr = evaluateExpression(node.target, currentEnv_);
    } else {
        arr = currentEnv_->get(node.name, node.line);
    }
    int64_t idx = evaluateExpression(node.index, currentEnv_).asInt();

    if (arr.isString() && !node.index2) {
        std::string s = arr.asString();
        size_t len = utf8Length(s);
        if (idx < 0 || (size_t)idx >= len) {
            throw AlgoError("Indice " + std::to_string(idx) + " hors bornes", node.line);
        }
        evalResult_ = Value(utf8Substr(s, (size_t)idx, (size_t)(idx + 1)));
        return;
    }

    if (!arr.isArray()) {
        throw AlgoError((node.target ? "L'expression" : node.name) + " n'est pas un tableau ni une chaîne", node.line);
    }
    if (idx < 0 || (size_t)idx >= arr.arrVal.size()) {
        throw AlgoError("Indice " + std::to_string(idx) + " hors bornes", node.line);
    }

    if (node.index2) {
        Value row = arr.arrVal[idx];
        int64_t idx2 = evaluateExpression(node.index2, currentEnv_).asInt();
        if (row.isString()) {
            size_t len = utf8Length(row.asString());
            if (idx2 < 0 || (size_t)idx2 >= len) {
                throw AlgoError("Indice colonne " + std::to_string(idx2) + " hors bornes", node.line);
            }
            evalResult_ = Value(utf8Substr(row.asString(), (size_t)idx2, (size_t)(idx2 + 1)));
            return;
        }
        if (!row.isArray()) {
            throw AlgoError("Ligne " + std::to_string(idx) + " de " + (node.target ? "matrice" : node.name) + " n'est pas un tableau", node.line);
        }
        if (idx2 < 0 || (size_t)idx2 >= row.arrVal.size()) {
            throw AlgoError("Indice colonne " + std::to_string(idx2) + " hors bornes", node.line);
        }
        evalResult_ = row.arrVal[idx2];
        return;
    }

    evalResult_ = arr.arrVal[idx];
}

void Interpreter::visit(MemberAccessExpr& node) {
    Value obj = evaluateExpression(node.object, currentEnv_);
    if (obj.isPointer()) {
        if (!obj.ptrVal) {
            throw AlgoError("Déréférencement d'un pointeur NIL ou non alloué", node.line);
        }
        obj = *(obj.ptrVal);
    }
    if (!obj.isRecord()) {
        throw AlgoError("Accès à un membre sur un objet qui n'est pas un enregistrement", node.line);
    }
    auto it = obj.recVal.find(node.member);
    if (it != obj.recVal.end()) {
        evalResult_ = it->second;
        return;
    }
    std::string normMem = normalize(node.member);
    for (const auto& kv : obj.recVal) {
        if (normalize(kv.first) == normMem) {
            evalResult_ = kv.second;
            return;
        }
    }
    throw AlgoError("Champ '" + node.member + "' introuvable", node.line);
}

void Interpreter::visit(UnaryExpr& node) {
    Value operand = evaluateExpression(node.operand, currentEnv_);
    if (node.op == "-") {
        if (operand.isInt()) evalResult_ = Value(-operand.asInt());
        else evalResult_ = Value(-operand.asReal());
        return;
    }
    if (node.op == "Non" || normalize(node.op) == "non") {
        evalResult_ = Value(!operand.asBool());
        return;
    }
    throw AlgoError("Opérateur unaire inconnu: " + node.op, node.line);
}

void Interpreter::visit(BinaryExpr& node) {
    // Short-circuit for logical operators
    if (node.op == "Et") {
        Value left = evaluateExpression(node.left, currentEnv_);
        if (!left.asBool()) {
            evalResult_ = Value(false);
            return;
        }
        evalResult_ = Value(evaluateExpression(node.right, currentEnv_).asBool());
        return;
    }
    if (node.op == "Ou") {
        Value left = evaluateExpression(node.left, currentEnv_);
        if (left.asBool()) {
            evalResult_ = Value(true);
            return;
        }
        evalResult_ = Value(evaluateExpression(node.right, currentEnv_).asBool());
        return;
    }

    Value left = evaluateExpression(node.left, currentEnv_);
    Value right = evaluateExpression(node.right, currentEnv_);

    const std::string& op = node.op;
    if (op == "+") { evalResult_ = left.add(right); return; }
    if (op == "-") { evalResult_ = left.sub(right); return; }
    if (op == "*") { evalResult_ = left.mul(right); return; }
    if (op == "^" || op == "**") {
        double base = left.asReal();
        double exp = right.asReal();
        double res = std::pow(base, exp);
        if (left.isInt() && right.isInt() && right.asInt() >= 0) {
            evalResult_ = Value((int64_t)std::round(res));
        } else {
            evalResult_ = Value(res);
        }
        return;
    }
    if (op == "/") { evalResult_ = left.divReal(right); return; }
    if (op == "div") { evalResult_ = left.divInt(right); return; }
    if (op == "mod" || op == "%") { evalResult_ = left.mod(right); return; }

    if (op == "=") { evalResult_ = Value(left.equals(right)); return; }
    if (op == "≠" || op == "!=") { evalResult_ = Value(left.notEquals(right)); return; }
    if (op == "<") { evalResult_ = Value(left.lessThan(right)); return; }
    if (op == "<=" || op == "≤") { evalResult_ = Value(left.lessOrEqual(right)); return; }
    if (op == ">") { evalResult_ = Value(left.greaterThan(right)); return; }
    if (op == ">=" || op == "≥") { evalResult_ = Value(left.greaterOrEqual(right)); return; }

    if (op == "∈") { evalResult_ = Value(right.contains(left)); return; }

    throw AlgoError("Opérateur binaire inconnu: " + op, node.line);
}

void Interpreter::visit(CallExpr& node) {
    evalResult_ = evaluateCall(node.name, node.args, currentEnv_, node.line);
}

void Interpreter::visit(SetLiteralExpr& node) {
    std::vector<Value> arr;
    arr.reserve(node.elements.size());
    for (const auto& elem : node.elements) {
        arr.push_back(evaluateExpression(elem, currentEnv_));
    }
    evalResult_ = Value(std::move(arr));
}

void Interpreter::visit(DerefExpr& node) {
    Value ptr = evaluateExpression(node.target, currentEnv_);
    if (!ptr.isPointer() || !ptr.ptrVal) {
        throw AlgoError("Déréférencement d'un pointeur NIL ou non alloué", node.line);
    }
    evalResult_ = *(ptr.ptrVal);
}

void Interpreter::visit([[maybe_unused]] NilExpr& node) {
    evalResult_ = Value::makeNil();
}

Value Interpreter::evaluateCall(const std::string& name, const std::vector<std::shared_ptr<Expr>>& args,
                                const std::shared_ptr<Environment>& env, int line) {
    std::string n = normalize(name);

    if (functions_.find(n) != functions_.end()) {
        return executeFunction(n, args, env, line);
    }

    if (n == "fin_fichier" || n == "finfichier") {
        if (args.empty()) throw AlgoError("Fin_fichier attend 1 argument: fichier", line);
        std::string fVar;
        if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(args[0])) {
            fVar = varExpr->name;
        } else {
            fVar = evaluateExpression(args[0], env).asString();
        }
        std::string fNorm = normalize(fVar);
        auto it = openFiles_.find(fNorm);
        if (it == openFiles_.end() || !it->second->isOpen) {
            return Value(true);
        }
        auto& stream = it->second->stream;
        if (!stream.good()) {
            return Value(true);
        }
        if (!it->second->isText) {
            stream >> std::ws;
        }
        int c = stream.peek();
        return Value(c == std::char_traits<char>::eof());
    }

    if (n == "chr") {
        if (args.empty()) throw AlgoError("chr attend 1 argument", line);
        int64_t code = evaluateExpression(args[0], env).asInt();
        return Value(std::string(1, (char)code));
    }
    if (n == "ord") {
        if (args.empty()) throw AlgoError("ord attend 1 argument", line);
        Value v = evaluateExpression(args[0], env);
        if (v.isInt()) return v;
        std::string s = v.asString();
        auto itEnum = enumOrdinals_.find(normalize(s));
        if (itEnum != enumOrdinals_.end()) {
            return Value(itEnum->second);
        }
        return Value((int64_t)(s.empty() ? 0 : (unsigned char)s[0]));
    }
    if (n == "sin") {
        if (args.empty()) throw AlgoError("sin attend 1 argument", line);
        return Value(std::sin(evaluateExpression(args[0], env).asReal()));
    }
    if (n == "cos") {
        if (args.empty()) throw AlgoError("cos attend 1 argument", line);
        return Value(std::cos(evaluateExpression(args[0], env).asReal()));
    }
    if (n == "tan") {
        if (args.empty()) throw AlgoError("tan attend 1 argument", line);
        return Value(std::tan(evaluateExpression(args[0], env).asReal()));
    }
    if (n == "arctan" || n == "atan") {
        if (args.empty()) throw AlgoError("arctan attend 1 argument", line);
        return Value(std::atan(evaluateExpression(args[0], env).asReal()));
    }
    if (n == "frac") {
        if (args.empty()) throw AlgoError("frac attend 1 argument", line);
        double val = evaluateExpression(args[0], env).asReal();
        double intpart;
        return Value(std::modf(val, &intpart));
    }
    if (n == "puissance") {
        if (args.size() < 2) throw AlgoError("puissance attend 2 arguments", line);
        Value b = evaluateExpression(args[0], env);
        Value e = evaluateExpression(args[1], env);
        double res = std::pow(b.asReal(), e.asReal());
        if (b.isInt() && e.isInt() && e.asInt() >= 0) {
            return Value((int64_t)std::round(res));
        }
        return Value(res);
    }
    if (n == "inserer") {
        if (args.size() < 3) throw AlgoError("inserer attend 3 arguments", line);
        std::string ch1 = evaluateExpression(args[0], env).asString();
        std::string ch2 = evaluateExpression(args[1], env).asString();
        int64_t pos = evaluateExpression(args[2], env).asInt();
        size_t len = utf8Length(ch2);
        if (pos < 0) pos = 0;
        if (pos > (int64_t)len) pos = len;
        std::string prefix = utf8Substr(ch2, 0, (size_t)pos);
        std::string suffix = utf8Substr(ch2, (size_t)pos, len);
        return Value(prefix + ch1 + suffix);
    }
    if (n == "taille_fichier" || n == "taille") {
        if (args.empty()) throw AlgoError("taille_fichier attend 1 argument", line);
        std::string fVar;
        if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(args[0])) fVar = varExpr->name;
        else fVar = evaluateExpression(args[0], env).asString();
        auto it = openFiles_.find(normalize(fVar));
        if (it == openFiles_.end() || !it->second->isOpen) return Value((int64_t)0);
        return Value((int64_t)it->second->recordOffsets.size());
    }
    if (n == "position_fichier" || n == "pointeur") {
        if (args.empty()) throw AlgoError("position_fichier attend 1 argument", line);
        std::string fVar;
        if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(args[0])) fVar = varExpr->name;
        else fVar = evaluateExpression(args[0], env).asString();
        auto it = openFiles_.find(normalize(fVar));
        if (it == openFiles_.end() || !it->second->isOpen) return Value((int64_t)0);
        return Value(it->second->currentRecordIndex);
    }
    if (n == "long") {
        if (args.empty()) throw AlgoError("long attend 1 argument", line);
        Value val = evaluateExpression(args[0], env);
        if (val.isArray()) return Value((int64_t)val.arrVal.size());
        return Value((int64_t)utf8Length(val.asString()));
    }
    if (n == "majus") {
        if (args.empty()) throw AlgoError("majus attend 1 argument", line);
        std::string s = evaluateExpression(args[0], env).asString();
        return Value(toUpper(s));
    }
    if (n == "minus") {
        if (args.empty()) throw AlgoError("minus attend 1 argument", line);
        std::string s = evaluateExpression(args[0], env).asString();
        return Value(toLower(s));
    }
    if (n == "sous_chaine" || n == "souschaine") {
        if (args.size() < 3) throw AlgoError("sous_chaine attend 3 arguments", line);
        std::string s = evaluateExpression(args[0], env).asString();
        int64_t d = evaluateExpression(args[1], env).asInt();
        int64_t p3 = evaluateExpression(args[2], env).asInt();
        size_t len = utf8Length(s);
        if (d < 0) d = 0;
        if (d >= (int64_t)len) return Value("");

        int64_t end;
        if (p3 < d) {
            end = d + p3;
        } else if (d + p3 == (int64_t)len && p3 != (int64_t)len) {
            end = len;
        } else {
            end = p3;
        }
        if (end < d) return Value("");
        if (end > (int64_t)len) end = len;
        return Value(utf8Substr(s, (size_t)d, (size_t)end));
    }
    if (n == "effacer") {
        if (args.size() < 3) throw AlgoError("effacer attend 3 arguments", line);
        std::string s = evaluateExpression(args[0], env).asString();
        int64_t d = evaluateExpression(args[1], env).asInt();
        int64_t f = evaluateExpression(args[2], env).asInt();
        size_t len = utf8Length(s);
        int64_t end = (f < d) ? (d + f) : f;
        std::string part1 = utf8Substr(s, 0, (size_t)std::max((int64_t)0, d));
        std::string part2 = (end < (int64_t)len) ? utf8Substr(s, (size_t)end, len) : "";
        return Value(part1 + part2);
    }
    if (n == "pos") {
        if (args.size() < 2) throw AlgoError("pos attend 2 arguments", line);
        std::string s1 = evaluateExpression(args[0], env).asString();
        std::string s2 = evaluateExpression(args[1], env).asString();
        size_t p = s2.find(s1);
        if (p == std::string::npos) return Value((int64_t)-1);
        return Value((int64_t)utf8Length(s2.substr(0, p)));
    }
    if (n == "convch") {
        if (args.empty()) throw AlgoError("convch attend 1 argument", line);
        return Value(evaluateExpression(args[0], env).asString());
    }
    if (n == "valeur") {
        if (args.empty()) throw AlgoError("valeur attend 1 argument", line);
        std::string s = evaluateExpression(args[0], env).asString();
        try {
            if (s.find('.') != std::string::npos || s.find('e') != std::string::npos || s.find('E') != std::string::npos) return Value((double)std::stod(s)); else return Value((int64_t)std::stoll(s));
        } catch (...) {
            return Value((double)0.0);
        }
    }
    if (n == "exp") {
        if (args.empty()) throw AlgoError("exp attend 1 argument", line);
        double val = evaluateExpression(args[0], env).asReal();
        return Value(std::exp(val));
    }
    if (n == "ln" || n == "log") {
        if (args.empty()) throw AlgoError("ln attend 1 argument", line);
        double val = evaluateExpression(args[0], env).asReal();
        return Value(std::log(val));
    }
    if (n == "racine") {
        if (args.empty()) throw AlgoError("racine attend 1 argument", line);
        double val = evaluateExpression(args[0], env).asReal();
        return Value(std::sqrt(val));
    }
    if (n == "abs") {
        if (args.empty()) throw AlgoError("abs attend 1 argument", line);
        Value v = evaluateExpression(args[0], env);
        if (v.isInt()) return Value(std::abs(v.asInt()));
        return Value(std::abs(v.asReal()));
    }
    if (n == "arrondi") {
        if (args.empty()) throw AlgoError("arrondi attend 1 argument", line);
        double x = evaluateExpression(args[0], env).asReal();
        double ix = std::floor(x);
        if (x - ix == 0.5) {
            return Value((int64_t)ix % 2 == 0 ? (int64_t)ix : (int64_t)ix + 1);
        }
        return Value((int64_t)std::round(x));
    }
    if (n == "ent") {
        if (args.empty()) throw AlgoError("ent attend 1 argument", line);
        double x = evaluateExpression(args[0], env).asReal();
        return Value((int64_t)std::trunc(x));
    }
    if (n == "estnum" || n == "est_num") {
        if (args.empty()) throw AlgoError("estnum attend 1 argument", line);
        std::string s = evaluateExpression(args[0], env).asString();
        static const std::regex numRegex("^-?\\d+(\\.\\d+)?$");
        return Value(std::regex_match(s, numRegex));
    }
    if (n == "graine") {
        if (args.empty()) throw AlgoError("graine attend 1 argument", line);
        int64_t seed = evaluateExpression(args[0], env).asInt();
        rng_.reseed((uint32_t)seed);
        return Value(seed);
    }
    if (n == "alea" || n == "alea") {
        if (args.size() < 2) throw AlgoError("alea attend 2 arguments", line);
        int64_t minVal = evaluateExpression(args[0], env).asInt();
        int64_t maxVal = evaluateExpression(args[1], env).asInt();
        return Value(rng_.randomInt(minVal, maxVal));
    }

    if (functions_.find(n) != functions_.end()) {
        return executeFunction(n, args, env, line);
    }

    throw AlgoError("Fonction '" + name + "' non définie", line);
}

} // namespace algo
