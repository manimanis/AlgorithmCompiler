#include "../include/interpreter.hpp"
#include <regex>
#include <filesystem>

namespace algo {

Interpreter::Interpreter(InterpreterOptions options)
    : options_(options), globalEnv_(std::make_shared<Environment>()), rng_(options.seed) {}

void Interpreter::checkIterationLimit(int line) {
    iterationCount_++;
    if (iterationCount_ > options_.maxIterations) {
        throw AlgoError("Limite d'itérations atteinte", line);
    }
}

void Interpreter::checkTimeout(int line) {
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
    if (norm == "entier") return Value((int64_t)0);
    if (norm == "reel") return Value((double)0.0);
    if (norm == "booleen") return Value(false);
    if (norm == "caractere") return Value("");
    if (norm == "chaine") return Value("");

    auto it = typeDefs_.find(norm);
    if (it != typeDefs_.end()) {
        const auto& td = it->second;
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
    openFiles_.clear();

    for (const auto& td : prog.typeDecls) {
        typeDefs_[normalize(td.name)] = td;
    }
    for (const auto& proc : prog.procedures) {
        procedures_[normalize(proc.name)] = proc;
    }
    for (const auto& func : prog.functions) {
        functions_[normalize(func.name)] = func;
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
    if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(lhs)) {
        env->set(varExpr->name, val, line > 0 ? line : varExpr->line);
        return;
    }
    if (auto memExpr = std::dynamic_pointer_cast<MemberAccessExpr>(lhs)) {
        Value obj = evaluateExpression(memExpr->object, env);
        if (!obj.isRecord()) {
            throw AlgoError("Accès à un champ sur un objet qui n'est pas un enregistrement", line > 0 ? line : memExpr->line);
        }
        obj.recVal[memExpr->member] = val;
        assignToLValue(memExpr->object, obj, env, line);
        return;
    }
    if (auto arrExpr = std::dynamic_pointer_cast<ArrayAccessExpr>(lhs)) {
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
    if (returnValue_.set) return;

    if (auto a = std::dynamic_pointer_cast<AssignStmt>(stmt)) {
        Value val = evaluateExpression(a->value, env);
        assignToLValue(std::make_shared<VariableExpr>(a->target, a->line), val, env, a->line);
    } else if (auto aa = std::dynamic_pointer_cast<ArrayAssignStmt>(stmt)) {
        Value val = evaluateExpression(aa->value, env);
        if (aa->targetExpr) {
            assignToLValue(std::make_shared<ArrayAccessExpr>(aa->targetExpr, aa->index, aa->index2, aa->line), val, env, aa->line);
        } else {
            assignToLValue(std::make_shared<ArrayAccessExpr>(aa->target, aa->index, aa->index2, aa->line), val, env, aa->line);
        }
    } else if (auto ma = std::dynamic_pointer_cast<MemberAssignStmt>(stmt)) {
        Value val = evaluateExpression(ma->value, env);
        assignToLValue(std::make_shared<MemberAccessExpr>(ma->target, ma->member, ma->line), val, env, ma->line);
    } else if (auto w = std::dynamic_pointer_cast<WriteStmt>(stmt)) {
        bool isFileWrite = false;
        std::string fileLogicalName;
        if (!w->args.empty()) {
            if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(w->args[0])) {
                if (env->has(varExpr->name)) {
                    Value v = env->get(varExpr->name);
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
                throw AlgoError("Fichier '" + fileLogicalName + "' non ouvert", w->line);
            }
            auto& f = *(it->second);
            if (f.isText) {
                for (size_t i = 1; i < w->args.size(); i++) {
                    Value v = evaluateExpression(w->args[i], env);
                    f.stream << v.asString();
                }
                if (w->isNl) {
                    f.stream << "\n";
                }
                f.stream.flush();
            } else {
                for (size_t i = 1; i < w->args.size(); i++) {
                    Value v = evaluateExpression(w->args[i], env);
                    writeFileData(f, v);
                }
                f.stream.flush();
            }
            return;
        }

        std::string lineStr;
        for (size_t i = 0; i < w->args.size(); i++) {
            Value val = evaluateExpression(w->args[i], env);
            if (val.isBool()) {
                lineStr += val.asBool() ? "Vrai" : "Faux";
            } else {
                lineStr += val.asString();
            }
            if (i + 1 < w->args.size()) {
                lineStr += w->sep;
            }
        }
        lineStr += w->fin;
        addOutput(lineStr);
    } else if (auto r = std::dynamic_pointer_cast<ReadStmt>(stmt)) {
        if (r->targets.size() == 2) {
            std::string t0Name = r->targets[0].name;
            if (env->has(t0Name)) {
                Value v = env->get(t0Name);
                if (v.isFile()) {
                    std::string fNorm = normalize(t0Name);
                    auto it = openFiles_.find(fNorm);
                    if (it == openFiles_.end() || !it->second->isOpen) {
                        throw AlgoError("Fichier '" + t0Name + "' non ouvert", r->line);
                    }
                    auto& f = *(it->second);
                    if (f.isText) {
                        std::ostringstream ss;
                        ss << f.stream.rdbuf();
                        std::string content = ss.str();
                        assignToLValue(r->targets[1].targetExpr, Value(content), env, r->line);
                    } else {
                        Value val = readFileData(f);
                        assignToLValue(r->targets[1].targetExpr, val, env, r->line);
                    }
                    return;
                }
            }
        }

        for (const auto& tgt : r->targets) {
            Value currentVal = evaluateExpression(tgt.targetExpr, env);
            std::string inputStr = requestInput("Saisir une valeur :");
            Value converted = convertInput(inputStr, currentVal);
            assignToLValue(tgt.targetExpr, converted, env, r->line);
        }
    } else if (auto ifs = std::dynamic_pointer_cast<IfStmt>(stmt)) {
        if (evaluateExpression(ifs->condition, env).asBool()) {
            executeBlock(ifs->thenBlock, env);
        } else {
            bool matched = false;
            for (const auto& eb : ifs->elseIfBlocks) {
                if (evaluateExpression(eb.condition, env).asBool()) {
                    executeBlock(eb.body, env);
                    matched = true;
                    break;
                }
            }
            if (!matched && !ifs->elseBlock.empty()) {
                executeBlock(ifs->elseBlock, env);
            }
        }
    } else if (auto fors = std::dynamic_pointer_cast<ForStmt>(stmt)) {
        Value startVal = evaluateExpression(fors->start, env);
        Value endVal = evaluateExpression(fors->end, env);
        Value stepVal = fors->step ? evaluateExpression(fors->step, env) : Value((int64_t)1);

        if (!startVal.isNumber() || !endVal.isNumber() || !stepVal.isNumber()) {
            throw AlgoError("Les bornes de la boucle Pour doivent être des nombres", fors->line);
        }
        int64_t step = stepVal.asInt();
        if (step == 0) {
            throw AlgoError("Le pas de la boucle Pour ne peut pas être nul", fors->line);
        }

        int64_t start = startVal.asInt();
        int64_t end = endVal.asInt();

        if (step > 0) {
            for (int64_t i = start; i <= end; i += step) {
                checkIterationLimit(fors->line);
                checkTimeout(fors->line);
                env->set(fors->varName, Value(i), fors->line);
                executeBlock(fors->body, env);
                if (returnValue_.set) return;
            }
        } else {
            for (int64_t i = start; i >= end; i += step) {
                checkIterationLimit(fors->line);
                checkTimeout(fors->line);
                env->set(fors->varName, Value(i), fors->line);
                executeBlock(fors->body, env);
                if (returnValue_.set) return;
            }
        }
    } else if (auto whiles = std::dynamic_pointer_cast<WhileStmt>(stmt)) {
        while (evaluateExpression(whiles->condition, env).asBool()) {
            checkIterationLimit(whiles->line);
            checkTimeout(whiles->line);
            executeBlock(whiles->body, env);
            if (returnValue_.set) return;
        }
    } else if (auto reps = std::dynamic_pointer_cast<RepeatStmt>(stmt)) {
        do {
            checkIterationLimit(reps->line);
            checkTimeout(reps->line);
            executeBlock(reps->body, env);
            if (returnValue_.set) return;
        } while (!evaluateExpression(reps->condition, env).asBool());
    } else if (auto calls = std::dynamic_pointer_cast<CallStmt>(stmt)) {
        std::string n = normalize(calls->name);
        if (n == "ouvrir" || n == "fermer" || n == "lire_ligne" || procedures_.find(n) != procedures_.end()) {
            executeProcedure(n, calls->args, env, calls->line);
        } else if (functions_.find(n) != functions_.end()) {
            executeFunction(n, calls->args, env, calls->line);
        } else if (n == "graine") {
            evaluateCall(n, calls->args, env, calls->line);
        } else {
            throw AlgoError("Procédure '" + calls->name + "' non définie", calls->line);
        }
        } else if (auto sws = std::dynamic_pointer_cast<SwitchStmt>(stmt)) {
        Value targetVal = evaluateExpression(sws->expr, env);
        bool matched = false;
        const SwitchCase* defaultCase = nullptr;

        for (const auto& sc : sws->cases) {
            if (sc.isDefault) {
                defaultCase = &sc;
                continue;
            }
            if (!sc.items.empty()) {
                for (const auto& item : sc.items) {
                    if (item.isRange()) {
                        Value startVal = evaluateExpression(item.start, env);
                        Value endVal = evaluateExpression(item.end, env);
                        bool inRange = false;
                        if (startVal.lessOrEqual(endVal)) {
                            inRange = targetVal.greaterOrEqual(startVal) && targetVal.lessOrEqual(endVal);
                        } else {
                            inRange = targetVal.greaterOrEqual(endVal) && targetVal.lessOrEqual(startVal);
                        }
                        if (inRange) {
                            matched = true;
                            executeBlock(sc.body, env);
                            break;
                        }
                    } else {
                        Value caseVal = evaluateExpression(item.start, env);
                        if (targetVal.equals(caseVal)) {
                            matched = true;
                            executeBlock(sc.body, env);
                            break;
                        }
                    }
                }
            } else {
                for (const auto& valExpr : sc.values) {
                    Value caseVal = evaluateExpression(valExpr, env);
                    if (targetVal.equals(caseVal)) {
                        matched = true;
                        executeBlock(sc.body, env);
                        break;
                    }
                }
            }
            if (matched) break;
        }

        if (!matched && defaultCase != nullptr) {
            executeBlock(defaultCase->body, env);
        }
    } else if (auto rets = std::dynamic_pointer_cast<ReturnStmt>(stmt)) {
        returnValue_ = {true, evaluateExpression(rets->value, env)};
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
        file->stream.open(std::filesystem::u8path(path), m);
#else
        file->stream.open(path, m);
#endif
        if (!file->stream.is_open()) {
            throw AlgoError("Impossible d'ouvrir le fichier '" + path + "'", line);
        }
        file->isOpen = true;
        openFiles_[fNorm] = file;

        Value fVal;
        fVal.type = ValueType::FileHandle;
        fVal.fileLogicalName = fVar;
        callerEnv->set(fVar, fVal);
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

    // Local variable declarations
    for (const auto& vd : proc.localVars) {
        std::string vtNorm = normalize(vd.varType);
        if (vtNorm == "procedure" || vtNorm == "fonction" || vd.typeSpec.name == "procedure" || vd.typeSpec.name == "fonction") continue;
        for (const auto& varName : vd.names) {
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

    // Local variable declarations
    for (const auto& vd : func.localVars) {
        std::string vtNorm = normalize(vd.varType);
        if (vtNorm == "procedure" || vtNorm == "fonction" || vd.typeSpec.name == "procedure" || vd.typeSpec.name == "fonction") continue;
        for (const auto& varName : vd.names) {
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

    Value result = returnValue_.value;
    returnValue_ = oldRet;
    return result;
}

Value Interpreter::evaluateExpression(const std::shared_ptr<Expr>& expr, const std::shared_ptr<Environment>& env) {
    if (!expr) return Value();

    if (auto num = std::dynamic_pointer_cast<NumberExpr>(expr)) {
        return Value(num->value);
    }
    if (auto r = std::dynamic_pointer_cast<RealExpr>(expr)) {
        return Value(r->value);
    }
    if (auto s = std::dynamic_pointer_cast<StringExpr>(expr)) {
        return Value(s->value);
    }
    if (auto b = std::dynamic_pointer_cast<BoolExpr>(expr)) {
        return Value(b->value);
    }
    if (auto v = std::dynamic_pointer_cast<VariableExpr>(expr)) {
        return env->get(v->name, v->line);
    }
    if (auto mem = std::dynamic_pointer_cast<MemberAccessExpr>(expr)) {
        Value obj = evaluateExpression(mem->object, env);
        if (!obj.isRecord()) {
            throw AlgoError("Accès à un membre sur un objet qui n'est pas un enregistrement", mem->line);
        }
        auto it = obj.recVal.find(mem->member);
        if (it != obj.recVal.end()) return it->second;
        std::string normMem = normalize(mem->member);
        for (const auto& kv : obj.recVal) {
            if (normalize(kv.first) == normMem) return kv.second;
        }
        throw AlgoError("Champ '" + mem->member + "' introuvable", mem->line);
    }
    if (auto aa = std::dynamic_pointer_cast<ArrayAccessExpr>(expr)) {
        Value arr;
        if (aa->target) {
            arr = evaluateExpression(aa->target, env);
        } else {
            arr = env->get(aa->name, aa->line);
        }
        int64_t idx = evaluateExpression(aa->index, env).asInt();

        if (arr.isString() && !aa->index2) {
            std::string s = arr.asString();
            size_t len = utf8Length(s);
            if (idx < 0 || (size_t)idx >= len) {
                throw AlgoError("Indice " + std::to_string(idx) + " hors bornes", aa->line);
            }
            return Value(utf8Substr(s, (size_t)idx, (size_t)(idx + 1)));
        }

        if (!arr.isArray()) {
            throw AlgoError((aa->target ? "L'expression" : aa->name) + " n'est pas un tableau ni une chaîne", aa->line);
        }
        if (idx < 0 || (size_t)idx >= arr.arrVal.size()) {
            throw AlgoError("Indice " + std::to_string(idx) + " hors bornes", aa->line);
        }

        if (aa->index2) {
            Value row = arr.arrVal[idx];
            int64_t idx2 = evaluateExpression(aa->index2, env).asInt();
            if (row.isString()) {
                size_t len = utf8Length(row.asString());
                if (idx2 < 0 || (size_t)idx2 >= len) {
                    throw AlgoError("Indice colonne " + std::to_string(idx2) + " hors bornes", aa->line);
                }
                return Value(utf8Substr(row.asString(), (size_t)idx2, (size_t)(idx2 + 1)));
            }
            if (!row.isArray()) {
                throw AlgoError("Ligne " + std::to_string(idx) + " de " + (aa->target ? "matrice" : aa->name) + " n'est pas un tableau", aa->line);
            }
            if (idx2 < 0 || (size_t)idx2 >= row.arrVal.size()) {
                throw AlgoError("Indice colonne " + std::to_string(idx2) + " hors bornes", aa->line);
            }
            return row.arrVal[idx2];
        }

        return arr.arrVal[idx];
    }
    if (auto u = std::dynamic_pointer_cast<UnaryExpr>(expr)) {
        Value operand = evaluateExpression(u->operand, env);
        if (u->op == "-") {
            if (operand.isInt()) return Value(-operand.asInt());
            return Value(-operand.asReal());
        }
        if (u->op == "Non" || normalize(u->op) == "non") {
            return Value(!operand.asBool());
        }
        throw AlgoError("Opérateur unaire inconnu: " + u->op, u->line);
    }
    if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
        // Short-circuit for logical operators
        if (bin->op == "Et") {
            Value left = evaluateExpression(bin->left, env);
            if (!left.asBool()) return Value(false);
            return Value(evaluateExpression(bin->right, env).asBool());
        }
        if (bin->op == "Ou") {
            Value left = evaluateExpression(bin->left, env);
            if (left.asBool()) return Value(true);
            return Value(evaluateExpression(bin->right, env).asBool());
        }

        Value left = evaluateExpression(bin->left, env);
        Value right = evaluateExpression(bin->right, env);

        const std::string& op = bin->op;
        if (op == "+") return left.add(right);
        if (op == "-") return left.sub(right);
        if (op == "*") return left.mul(right);
        if (op == "/") return left.divReal(right);
        if (op == "div") return left.divInt(right);
        if (op == "mod" || op == "%") return left.mod(right);

        if (op == "=") return Value(left.equals(right));
        if (op == "≠" || op == "!=") return Value(left.notEquals(right));
        if (op == "<") return Value(left.lessThan(right));
        if (op == "<=" || op == "≤") return Value(left.lessOrEqual(right));
        if (op == ">") return Value(left.greaterThan(right));
        if (op == ">=" || op == "≥") return Value(left.greaterOrEqual(right));

        if (op == "∈") return Value(right.contains(left));

        throw AlgoError("Opérateur binaire inconnu: " + op, bin->line);
    }
    if (auto c = std::dynamic_pointer_cast<CallExpr>(expr)) {
        return evaluateCall(c->name, c->args, env, c->line);
    }
    if (auto setLit = std::dynamic_pointer_cast<SetLiteralExpr>(expr)) {
        std::vector<Value> arr;
        for (const auto& elem : setLit->elements) {
            arr.push_back(evaluateExpression(elem, env));
        }
        return Value(std::move(arr));
    }

    throw AlgoError("Type d'expression non reconnu");
}

Value Interpreter::evaluateCall(const std::string& name, const std::vector<std::shared_ptr<Expr>>& args,
                                const std::shared_ptr<Environment>& env, int line) {
    std::string n = normalize(name);

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
        std::string s = evaluateExpression(args[0], env).asString();
        return Value((int64_t)(s.empty() ? 0 : (unsigned char)s[0]));
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
        return Value((int64_t)std::floor(x));
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
