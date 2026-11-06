#include "../include/bytecode_compiler.hpp"

namespace algo {

BytecodeCompiler::BytecodeCompiler() {
    initBuiltins();
}

void BytecodeCompiler::initBuiltins() {
    builtinIds_["racine"] = BI_RACINE;
    builtinIds_["carre"] = BI_CARRE;
    builtinIds_["abs"] = BI_ABS;
    builtinIds_["exp"] = BI_EXP;
    builtinIds_["ln"] = BI_LN;
    builtinIds_["log"] = BI_LOG;
    builtinIds_["ent"] = BI_ENT;
    builtinIds_["arrondi"] = BI_ARRONDI;
    builtinIds_["long"] = BI_LONG;
    builtinIds_["chr"] = BI_CHR;
    builtinIds_["ord"] = BI_ORD;
    builtinIds_["convch"] = BI_CONVCH;
    builtinIds_["valeur"] = BI_VALEUR;
    builtinIds_["sous_chaine"] = BI_SOUS_CHAINE;
    builtinIds_["souschaine"] = BI_SOUS_CHAINE;
    builtinIds_["pos"] = BI_POS;
    builtinIds_["majus"] = BI_MAJUS;
    builtinIds_["minus"] = BI_MINUS;
    builtinIds_["effacer"] = BI_EFFACER;
    builtinIds_["estnum"] = BI_ESTNUM;
    builtinIds_["est_num"] = BI_ESTNUM;
    builtinIds_["alea"] = BI_ALEA;
    builtinIds_["graine"] = BI_GRAINE;
    builtinIds_["ouvrir"] = BI_OUVRIR;
    builtinIds_["fermer"] = BI_FERMER;
    builtinIds_["lire_ligne"] = BI_LIRE_LIGNE;
    builtinIds_["fin_fichier"] = BI_FIN_FICHIER;
    builtinIds_["finfichier"] = BI_FIN_FICHIER;
    builtinIds_["sin"] = BI_SIN;
    builtinIds_["cos"] = BI_COS;
    builtinIds_["tan"] = BI_TAN;
    builtinIds_["arctan"] = BI_ARCTAN;
    builtinIds_["atan"] = BI_ARCTAN;
    builtinIds_["frac"] = BI_FRAC;
    builtinIds_["puissance"] = BI_PUISSANCE;
    builtinIds_["inserer"] = BI_INSERER;
    builtinIds_["taille_fichier"] = BI_TAILLE_FICHIER;
    builtinIds_["taille"] = BI_TAILLE_FICHIER;
    builtinIds_["position_fichier"] = BI_POSITION_FICHIER;
    builtinIds_["pointeur"] = BI_POSITION_FICHIER;
    builtinIds_["aller_a"] = BI_ALLER_A;
    builtinIds_["pointer"] = BI_ALLER_A;
}

void BytecodeCompiler::emitByte(uint8_t byte) {
    currentChunk_->writeByte(byte, currentLine_);
}

void BytecodeCompiler::emitOp(OpCode op) {
    currentChunk_->writeOp(op, currentLine_);
}

void BytecodeCompiler::emitUint32(uint32_t val) {
    currentChunk_->writeUint32(val, currentLine_);
}

void BytecodeCompiler::emitInt32(int32_t val) {
    currentChunk_->writeInt32(val, currentLine_);
}

size_t BytecodeCompiler::emitJump(OpCode jumpOp) {
    emitOp(jumpOp);
    size_t offset = currentChunk_->code.size();
    emitInt32(0); // placeholder
    return offset;
}

void BytecodeCompiler::patchJump(size_t jumpOffset) {
    // Jump offset relative to instruction immediately following the 4-byte offset
    int32_t jump = static_cast<int32_t>(currentChunk_->code.size() - (jumpOffset + 4));
    currentChunk_->patchInt32(jumpOffset, jump);
}

void BytecodeCompiler::emitLoop(size_t loopStart) {
    emitOp(OpCode::OP_LOOP);
    // Relative backward jump from end of loop operand
    uint32_t offset = static_cast<uint32_t>(currentChunk_->code.size() + 4 - loopStart);
    emitUint32(offset);
}

uint32_t BytecodeCompiler::addConstant(const Value& val) {
    return currentChunk_->addConstant(val);
}

uint32_t BytecodeCompiler::addName(const std::string& name) {
    return currentChunk_->addName(name);
}

int BytecodeCompiler::findLocal(const std::string& name) const {
    std::string norm = normalize(name);
    for (int i = static_cast<int>(locals_.size()) - 1; i >= 0; --i) {
        if (normalize(locals_[i].name) == norm) {
            return i;
        }
    }
    return -1;
}

void BytecodeCompiler::addLocal(const std::string& name, bool byRef) {
    LocalVarInfo info;
    info.name = name;
    info.slot = static_cast<uint32_t>(locals_.size());
    info.byRef = byRef;
    locals_.push_back(info);
}

static bool tryEvalConst(const std::shared_ptr<Expr>& expr, Value& out) {
    if (!expr) return false;
    if (auto num = std::dynamic_pointer_cast<NumberExpr>(expr)) {
        out = Value(num->value);
        return true;
    }
    if (auto realE = std::dynamic_pointer_cast<RealExpr>(expr)) {
        out = Value(realE->value);
        return true;
    }
    if (auto str = std::dynamic_pointer_cast<StringExpr>(expr)) {
        out = Value(str->value);
        return true;
    }
    if (auto b = std::dynamic_pointer_cast<BoolExpr>(expr)) {
        out = Value(b->value);
        return true;
    }
    if (auto un = std::dynamic_pointer_cast<UnaryExpr>(expr)) {
        Value sub;
        if (!tryEvalConst(un->operand, sub)) return false;
        if (un->op == "-") {
            if (sub.isInt()) out = Value(-sub.asInt());
            else out = Value(-sub.asReal());
            return true;
        }
        if (un->op == "Non" || normalize(un->op) == "non") {
            out = Value(!sub.asBool());
            return true;
        }
        return false;
    }
    if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
        Value l, r;
        if (!tryEvalConst(bin->left, l) || !tryEvalConst(bin->right, r)) return false;
        const std::string& op = bin->op;
        try {
            if (op == "+") { out = l.add(r); return true; }
            if (op == "-") { out = l.sub(r); return true; }
            if (op == "*") { out = l.mul(r); return true; }
            if (op == "/" && r.asReal() != 0.0) { out = l.divReal(r); return true; }
            if (op == "div" && r.asInt() != 0) { out = l.divInt(r); return true; }
            if ((op == "mod" || op == "%") && r.asInt() != 0) { out = l.mod(r); return true; }
            if (op == "=") { out = Value(l.equals(r)); return true; }
            if (op == "≠" || op == "!=") { out = Value(l.notEquals(r)); return true; }
            if (op == "<") { out = Value(l.lessThan(r)); return true; }
            if (op == "<=" || op == "≤") { out = Value(l.lessOrEqual(r)); return true; }
            if (op == ">") { out = Value(l.greaterThan(r)); return true; }
            if (op == ">=" || op == "≥") { out = Value(l.greaterOrEqual(r)); return true; }
            if (op == "Et") { out = Value(l.asBool() && r.asBool()); return true; }
            if (op == "Ou") { out = Value(l.asBool() || r.asBool()); return true; }
        } catch (...) {
            return false;
        }
        return false;
    }
    return false;
}

static void optimizePeephole(Chunk& chunk) {
    if (chunk.code.empty()) return;

    size_t ip = 0;
    while (ip < chunk.code.size()) {
        OpCode op = static_cast<OpCode>(chunk.code[ip]);
        if (op == OpCode::OP_JUMP || op == OpCode::OP_JUMP_IF_FALSE || op == OpCode::OP_JUMP_IF_TRUE ||
            op == OpCode::OP_JUMP_IF_FALSE_PEEK || op == OpCode::OP_JUMP_IF_TRUE_PEEK) {
            if (ip + 5 <= chunk.code.size()) {
                size_t operandOffset = ip + 1;
                int32_t jmp = chunk.readInt32(operandOffset);
                size_t target = ip + 5 + jmp;

                // Si c'est un saut inconditionnel vers l'instruction suivante, on le NOP
                if (op == OpCode::OP_JUMP && jmp == 0) {
                    for (size_t k = 0; k < 5; ++k) chunk.code[ip + k] = static_cast<uint8_t>(OpCode::OP_NOP);
                } else if (target < chunk.code.size()) {
                    // Jump to jump chaining
                    int chainCount = 0;
                    while (target + 5 <= chunk.code.size() &&
                           static_cast<OpCode>(chunk.code[target]) == OpCode::OP_JUMP &&
                           chainCount < 10) {
                        size_t nextOpOffset = target + 1;
                        int32_t nextJmp = chunk.readInt32(nextOpOffset);
                        size_t nextTarget = target + 5 + nextJmp;
                        if (nextTarget >= chunk.code.size()) break;
                        target = nextTarget;
                        chainCount++;
                    }
                    if (chainCount > 0) {
                        int32_t newJmp = static_cast<int32_t>(target) - static_cast<int32_t>(ip + 5);
                        chunk.patchInt32(ip + 1, newJmp);
                    }
                }
            }
            ip += 5;
        } else {
            // Avancer selon la taille de l'instruction
            switch (op) {
                case OpCode::OP_CONST:
                case OpCode::OP_GET_GLOBAL:
                case OpCode::OP_SET_GLOBAL:
                case OpCode::OP_POP_GLOBAL:
                case OpCode::OP_GET_LOCAL:
                case OpCode::OP_SET_LOCAL:
                case OpCode::OP_POP_LOCAL:
                case OpCode::OP_GET_REF:
                case OpCode::OP_SET_REF:
                case OpCode::OP_POP_REF:
                case OpCode::OP_PUSH_REF_GLOBAL:
                case OpCode::OP_PUSH_REF_LOCAL:
                case OpCode::OP_PUSH_REF_REF:
                case OpCode::OP_REF_MEMBER:
                case OpCode::OP_RECORD_GET:
                case OpCode::OP_RECORD_SET:
                case OpCode::OP_READ_VAR:
                case OpCode::OP_READ_LOCAL:
                case OpCode::OP_READ_REF:
                case OpCode::OP_READ_ARRAY:
                case OpCode::OP_READ_ARRAY2D:
                case OpCode::OP_READ_FILE:
                case OpCode::OP_LOOP:
                case OpCode::OP_MAKE_SET:
                case OpCode::OP_ARRAY_GET_GLOBAL:
                case OpCode::OP_ARRAY_SET_GLOBAL:
                case OpCode::OP_ARRAY_GET_LOCAL:
                case OpCode::OP_ARRAY_SET_LOCAL:
                case OpCode::OP_ARRAY_GET_REF:
                case OpCode::OP_ARRAY_SET_REF:
                case OpCode::OP_ALLOC_PTR:
                    ip += 5;
                    break;
                case OpCode::OP_READ_RECORD:
                    ip += 9;
                    break;
                case OpCode::OP_CALL:
                case OpCode::OP_CALL_BUILTIN:
                    ip += 9;
                    break;
                case OpCode::OP_WRITE:
                    ip += 14;
                    break;
                default:
                    ip += 1;
                    break;
            }
        }
    }
}

void BytecodeCompiler::compileExpr(const std::shared_ptr<Expr>& expr) {
    if (!expr) return;
    currentLine_ = expr->line > 0 ? expr->line : currentLine_;

    // Constant Folding
    if (std::dynamic_pointer_cast<BinaryExpr>(expr) || std::dynamic_pointer_cast<UnaryExpr>(expr)) {
        Value constVal;
        if (tryEvalConst(expr, constVal)) {
            emitOp(OpCode::OP_CONST);
            emitUint32(addConstant(constVal));
            return;
        }
    }

    expr->accept(*this);
}

void BytecodeCompiler::compileStmt(const std::shared_ptr<Stmt>& stmt) {
    if (!stmt) return;
    currentLine_ = stmt->line > 0 ? stmt->line : currentLine_;
    stmt->accept(*this);
}

void BytecodeCompiler::compileBlock(const std::vector<std::shared_ptr<Stmt>>& stmts) {
    for (const auto& stmt : stmts) {
        compileStmt(stmt);
    }
}

BytecodeProgram BytecodeCompiler::compile(const Program& prog) {
    currentProg_ = &prog;
    BytecodeProgram result;
    result.name = prog.name;
    result.typeDecls = prog.typeDecls;
    result.constDecls = prog.constDecls;
    result.varDecls = prog.varDecls;
    varDecls_ = prog.varDecls;

    typeMap_.clear();
    for (const auto& td : prog.typeDecls) {
        typeMap_[normalize(td.name)] = td.typeSpec;
    }

    paramByRefMap_.clear();
    for (const auto& proc : prog.procedures) {
        isProcedureMap_[normalize(proc.name)] = true;
        std::vector<bool> refs;
        for (const auto& p : proc.params) refs.push_back(p.byRef);
        paramByRefMap_[normalize(proc.name)] = std::move(refs);
    }
    for (const auto& func : prog.functions) {
        isProcedureMap_[normalize(func.name)] = false;
        std::vector<bool> refs;
        for (const auto& p : func.params) refs.push_back(p.byRef);
        paramByRefMap_[normalize(func.name)] = std::move(refs);
    }

    // Compile procedures
    for (const auto& proc : prog.procedures) {
        currentSubroutineName_ = normalize(proc.name);
        BytecodeFunction bf;
        bf.name = proc.name;
        bf.isProcedure = true;
        bf.localVars = proc.localVars;

        locals_.clear();
        isInsideSubroutine_ = true;
        currentLocalVars_ = &proc.localVars;

        for (const auto& p : proc.params) {
            BytecodeParam bp;
            bp.name = p.name;
            bp.byRef = p.byRef;
            bf.params.push_back(bp);
            addLocal(p.name, p.byRef);
        }

        // Add local variables
        for (const auto& vd : proc.localVars) {
            std::string vtNorm = normalize(vd.varType);
            if (vtNorm == "procedure" || vtNorm == "fonction") continue;
            for (const auto& vName : vd.names) {
                if (findLocal(vName) == -1) {
                    addLocal(vName, false);
                }
            }
        }

        currentChunk_ = &bf.chunk;
        for (const auto& cd : proc.constDecls) {
            addLocal(cd.name, false);
            compileExpr(cd.valueExpr);
            int localIdx = findLocal(cd.name);
            emitOp(OpCode::OP_POP_LOCAL);
            emitUint32(static_cast<uint32_t>(localIdx));
        }
        compileBlock(proc.body);
        emitOp(OpCode::OP_RETURN_VOID);
        optimizePeephole(bf.chunk);

        result.subroutines[normalize(proc.name)] = std::move(bf);
    }

    // Compile functions
    for (const auto& func : prog.functions) {
        currentSubroutineName_ = normalize(func.name);
        BytecodeFunction bf;
        bf.name = func.name;
        bf.isProcedure = false;
        bf.returnType = func.returnType;
        bf.localVars = func.localVars;

        locals_.clear();
        isInsideSubroutine_ = true;
        currentLocalVars_ = &func.localVars;

        for (const auto& p : func.params) {
            BytecodeParam bp;
            bp.name = p.name;
            bp.byRef = p.byRef;
            bf.params.push_back(bp);
            addLocal(p.name, p.byRef);
        }

        for (const auto& vd : func.localVars) {
            std::string vtNorm = normalize(vd.varType);
            if (vtNorm == "procedure" || vtNorm == "fonction") continue;
            for (const auto& vName : vd.names) {
                if (findLocal(vName) == -1) {
                    addLocal(vName, false);
                }
            }
        }

        currentChunk_ = &bf.chunk;
        for (const auto& cd : func.constDecls) {
            addLocal(cd.name, false);
            compileExpr(cd.valueExpr);
            int localIdx = findLocal(cd.name);
            emitOp(OpCode::OP_POP_LOCAL);
            emitUint32(static_cast<uint32_t>(localIdx));
        }
        compileBlock(func.body);
        // Default return null if no explicit return
        emitOp(OpCode::OP_CONST);
        emitUint32(addConstant(Value()));
        emitOp(OpCode::OP_RETURN);
        optimizePeephole(bf.chunk);

        result.subroutines[normalize(func.name)] = std::move(bf);
    }

    // Compile main body
    locals_.clear();
    isInsideSubroutine_ = false;
    currentSubroutineName_.clear();
    currentLocalVars_ = nullptr;
    currentChunk_ = &result.mainChunk;

    for (const auto& cd : prog.constDecls) {
        compileExpr(cd.valueExpr);
        emitOp(OpCode::OP_POP_GLOBAL);
        emitUint32(addName(cd.name));
    }

    compileBlock(prog.mainBody);
    emitOp(OpCode::OP_HALT);
    optimizePeephole(result.mainChunk);

    return result;
}

// ---------------- Statements ----------------

void BytecodeCompiler::visit(AssignStmt& node) {
    compileExpr(node.value);
    int localIdx = isInsideSubroutine_ ? findLocal(node.target) : -1;
    if (localIdx != -1) {
        if (locals_[localIdx].byRef) {
            emitOp(OpCode::OP_POP_REF);
        } else {
            emitOp(OpCode::OP_POP_LOCAL);
        }
        emitUint32(static_cast<uint32_t>(localIdx));
    } else {
        emitOp(OpCode::OP_POP_GLOBAL);
        emitUint32(addName(node.target));
    }
}

void BytecodeCompiler::visit(ArrayAssignStmt& node) {
    if (!node.targetExpr && !node.index2) {
        compileExpr(node.index);
        compileExpr(node.value);
        int localIdx = isInsideSubroutine_ ? findLocal(node.target) : -1;
        if (localIdx != -1) {
            if (locals_[localIdx].byRef) {
                emitOp(OpCode::OP_ARRAY_SET_REF);
            } else {
                emitOp(OpCode::OP_ARRAY_SET_LOCAL);
            }
            emitUint32(static_cast<uint32_t>(localIdx));
        } else {
            emitOp(OpCode::OP_ARRAY_SET_GLOBAL);
            emitUint32(addName(node.target));
        }
        return;
    }

    if (node.targetExpr) {
        auto arrExpr = std::make_shared<ArrayAccessExpr>(node.targetExpr, node.index, node.index2, node.line);
        if (compileRefExpr(arrExpr)) {
            compileExpr(node.value);
            emitOp(OpCode::OP_SET_LVALUE);
            return;
        }
    }

    int localIdx = -1;
    // Evaluate target (array container)
    if (node.targetExpr) {
        compileExpr(node.targetExpr);
    } else {
        localIdx = isInsideSubroutine_ ? findLocal(node.target) : -1;
        if (localIdx != -1) {
            emitOp(locals_[localIdx].byRef ? OpCode::OP_GET_REF : OpCode::OP_GET_LOCAL);
            emitUint32(static_cast<uint32_t>(localIdx));
        } else {
            emitOp(OpCode::OP_GET_GLOBAL);
            emitUint32(addName(node.target));
        }
    }

    compileExpr(node.index);
    if (node.index2) {
        compileExpr(node.index2);
        compileExpr(node.value);
        emitOp(OpCode::OP_ARRAY2D_SET);
    } else {
        compileExpr(node.value);
        emitOp(OpCode::OP_ARRAY_SET);
    }

    if (!node.targetExpr) {
        if (localIdx != -1) {
            emitOp(locals_[localIdx].byRef ? OpCode::OP_POP_REF : OpCode::OP_POP_LOCAL);
            emitUint32(static_cast<uint32_t>(localIdx));
        } else {
            emitOp(OpCode::OP_POP_GLOBAL);
            emitUint32(addName(node.target));
        }
    }
}

void BytecodeCompiler::visit(MemberAssignStmt& node) {
    auto memExpr = std::make_shared<MemberAccessExpr>(node.target, node.member, node.line);
    if (compileRefExpr(memExpr)) {
        compileExpr(node.value);
        emitOp(OpCode::OP_SET_LVALUE);
        return;
    }

    int localIdx = -1;
    std::string varName;
    if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(node.target)) {
        varName = varExpr->name;
        localIdx = isInsideSubroutine_ ? findLocal(varName) : -1;
    }
    compileExpr(node.target);
    compileExpr(node.value);
    emitOp(OpCode::OP_RECORD_SET);
    emitUint32(addName(node.member));
    if (!varName.empty()) {
        if (localIdx != -1) {
            emitOp(locals_[localIdx].byRef ? OpCode::OP_POP_REF : OpCode::OP_POP_LOCAL);
            emitUint32(static_cast<uint32_t>(localIdx));
        } else {
            emitOp(OpCode::OP_POP_GLOBAL);
            emitUint32(addName(varName));
        }
    }
}

std::string BytecodeCompiler::resolveExprType(const std::shared_ptr<Expr>& expr) const {
    if (!expr) return "";
    if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(expr)) {
        std::string vNorm = normalize(varExpr->name);
        if (currentProg_ && !currentSubroutineName_.empty()) {
            for (const auto& proc : currentProg_->procedures) {
                if (normalize(proc.name) == currentSubroutineName_) {
                    for (const auto& p : proc.params) {
                        if (normalize(p.name) == vNorm) return p.paramType;
                    }
                    for (const auto& vd : proc.localVars) {
                        for (const auto& n : vd.names) {
                            if (normalize(n) == vNorm) return vd.varType;
                        }
                    }
                }
            }
            for (const auto& func : currentProg_->functions) {
                if (normalize(func.name) == currentSubroutineName_) {
                    for (const auto& p : func.params) {
                        if (normalize(p.name) == vNorm) return p.paramType;
                    }
                    for (const auto& vd : func.localVars) {
                        for (const auto& n : vd.names) {
                            if (normalize(n) == vNorm) return vd.varType;
                        }
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
        auto it = typeMap_.find(normalize(objType));
        if (it != typeMap_.end()) {
            for (const auto& f : it->second.fields) {
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
        auto it = typeMap_.find(normalize(t));
        if (it != typeMap_.end() && it->second.kind == TypeKind::Pointer) {
            return it->second.elemType;
        }
        return "";
    }
    return "";
}

std::string BytecodeCompiler::resolvePointeeType(const std::shared_ptr<Expr>& expr) const {
    std::string t = resolveExprType(expr);
    if (!t.empty() && t[0] == '^') return t.substr(1);
    std::string tNorm = normalize(t);
    if (tNorm.find("pointeur sur ") == 0) return t.substr(13);
    if (tNorm.find("pointeur de ") == 0) return t.substr(12);
    auto it = typeMap_.find(tNorm);
    if (it != typeMap_.end() && it->second.kind == TypeKind::Pointer) {
        return it->second.elemType;
    }
    return t;
}

void BytecodeCompiler::visit(DerefAssignStmt& node) {
    auto derefExpr = std::make_shared<DerefExpr>(node.target, node.line);
    compileRefExpr(derefExpr);
    compileExpr(node.value);
    emitOp(OpCode::OP_SET_LVALUE);
}

void BytecodeCompiler::visit(AllocStmt& node) {
    if (!compileRefExpr(node.target)) {
        throw AlgoError("Cible d'allocation non valide", node.line);
    }
    std::string pointeeType = resolvePointeeType(node.target);
    emitOp(OpCode::OP_ALLOC_PTR);
    emitUint32(addName(pointeeType));
}

void BytecodeCompiler::visit(FreeStmt& node) {
    if (!compileRefExpr(node.target)) {
        throw AlgoError("Cible de libération non valide", node.line);
    }
    emitOp(OpCode::OP_FREE_PTR);
}

void BytecodeCompiler::visit(WriteStmt& node) {
    for (const auto& arg : node.args) {
        compileExpr(arg);
    }
    emitOp(OpCode::OP_WRITE);
    emitUint32(static_cast<uint32_t>(node.args.size()));
    emitByte(node.isNl ? 1 : 0);
    emitUint32(addName(node.sep));
    emitUint32(addName(node.fin));
}

bool BytecodeCompiler::isFileType(const TypeSpec& ts) const {
    if (ts.kind == TypeKind::FileText || ts.kind == TypeKind::FileData) return true;
    if (toLower(ts.name).find("fichier") != std::string::npos) return true;
    if (ts.kind == TypeKind::Simple && !ts.name.empty()) {
        auto it = typeMap_.find(normalize(ts.name));
        if (it != typeMap_.end()) {
            return isFileType(it->second);
        }
    }
    return false;
}

bool BytecodeCompiler::isFileVar(const std::string& varName) const {
    std::string norm = normalize(varName);
    auto checkVar = [&](const std::vector<VarDecl>& decls) {
        for (const auto& vd : decls) {
            for (const auto& n : vd.names) {
                if (normalize(n) == norm) {
                    if (isFileType(vd.typeSpec)) return true;
                    if (toLower(vd.varType).find("fichier") != std::string::npos) return true;
                    auto it = typeMap_.find(normalize(vd.varType));
                    if (it != typeMap_.end() && isFileType(it->second)) return true;
                }
            }
        }
        return false;
    };
    if (checkVar(varDecls_)) return true;
    if (currentLocalVars_ && checkVar(*currentLocalVars_)) return true;
    return false;
}

void BytecodeCompiler::visit(ReadStmt& node) {
    if (node.targets.size() == 2 && isFileVar(node.targets[0].name)) {
        if (compileRefExpr(node.targets[1].targetExpr)) {
            emitOp(OpCode::OP_READ_FILE);
            emitUint32(addName(node.targets[0].name));
            emitOp(OpCode::OP_SET_LVALUE);
            return;
        }
    }

    for (const auto& tgt : node.targets) {
        if (compileRefExpr(tgt.targetExpr)) {
            emitOp(OpCode::OP_READ_LVALUE);
        }
    }
}

void BytecodeCompiler::visit(IfStmt& node) {
    compileExpr(node.condition);
    size_t thenJump = emitJump(OpCode::OP_JUMP_IF_FALSE);

    compileBlock(node.thenBlock);

    std::vector<size_t> endJumps;
    endJumps.push_back(emitJump(OpCode::OP_JUMP));

    patchJump(thenJump);

    for (const auto& eb : node.elseIfBlocks) {
        compileExpr(eb.condition);
        size_t nextJump = emitJump(OpCode::OP_JUMP_IF_FALSE);

        compileBlock(eb.body);
        endJumps.push_back(emitJump(OpCode::OP_JUMP));

        patchJump(nextJump);
    }

    if (!node.elseBlock.empty()) {
        compileBlock(node.elseBlock);
    }

    for (size_t j : endJumps) {
        patchJump(j);
    }
}

void BytecodeCompiler::visit(ForStmt& node) {
    // 1. Initialize loop var: var = start
    compileExpr(node.start);
    int localIdx = isInsideSubroutine_ ? findLocal(node.varName) : -1;
    if (localIdx != -1) {
        emitOp(locals_[localIdx].byRef ? OpCode::OP_POP_REF : OpCode::OP_POP_LOCAL);
        emitUint32(static_cast<uint32_t>(localIdx));
    } else {
        emitOp(OpCode::OP_POP_GLOBAL);
        emitUint32(addName(node.varName));
    }

    size_t loopStart = currentChunk_->code.size();

    // 2. Condition check: var <= end (or >= if step < 0)
    if (localIdx != -1) {
        emitOp(locals_[localIdx].byRef ? OpCode::OP_GET_REF : OpCode::OP_GET_LOCAL);
        emitUint32(static_cast<uint32_t>(localIdx));
    } else {
        emitOp(OpCode::OP_GET_GLOBAL);
        emitUint32(addName(node.varName));
    }
    compileExpr(node.end);

    // Determine direction
    bool isNegativeStep = false;
    if (node.step) {
        if (auto num = std::dynamic_pointer_cast<NumberExpr>(node.step)) {
            if (num->value == 0) {
                throw AlgoError("Le pas de la boucle Pour ne peut pas être nul", node.line);
            }
            if (num->value < 0) isNegativeStep = true;
        } else if (auto un = std::dynamic_pointer_cast<UnaryExpr>(node.step)) {
            if (un->op == "-") {
                if (auto num = std::dynamic_pointer_cast<NumberExpr>(un->operand)) {
                    if (num->value == 0) {
                        throw AlgoError("Le pas de la boucle Pour ne peut pas être nul", node.line);
                    }
                    if (num->value > 0) isNegativeStep = true;
                }
            }
        }
    }
    emitOp(isNegativeStep ? OpCode::OP_GE : OpCode::OP_LE);

    size_t exitJump = emitJump(OpCode::OP_JUMP_IF_FALSE);

    // 3. Body
    compileBlock(node.body);

    // If we reached the end boundary, break without incrementing so the loop variable keeps the final iteration value
    if (localIdx != -1) {
        emitOp(locals_[localIdx].byRef ? OpCode::OP_GET_REF : OpCode::OP_GET_LOCAL);
        emitUint32(static_cast<uint32_t>(localIdx));
    } else {
        emitOp(OpCode::OP_GET_GLOBAL);
        emitUint32(addName(node.varName));
    }
    compileExpr(node.end);
    emitOp(isNegativeStep ? OpCode::OP_LE : OpCode::OP_GE);
    size_t lastIterJump = emitJump(OpCode::OP_JUMP_IF_TRUE);

    // 4. Increment: var = var + step
    if (localIdx != -1) {
        emitOp(locals_[localIdx].byRef ? OpCode::OP_GET_REF : OpCode::OP_GET_LOCAL);
        emitUint32(static_cast<uint32_t>(localIdx));
    } else {
        emitOp(OpCode::OP_GET_GLOBAL);
        emitUint32(addName(node.varName));
    }

    if (node.step) {
        compileExpr(node.step);
    } else {
        emitOp(OpCode::OP_CONST);
        emitUint32(addConstant(Value((int64_t)1)));
    }
    emitOp(OpCode::OP_ADD);

    if (localIdx != -1) {
        emitOp(locals_[localIdx].byRef ? OpCode::OP_POP_REF : OpCode::OP_POP_LOCAL);
        emitUint32(static_cast<uint32_t>(localIdx));
    } else {
        emitOp(OpCode::OP_POP_GLOBAL);
        emitUint32(addName(node.varName));
    }

    // 5. Loop back
    emitLoop(loopStart);
    patchJump(exitJump);
    patchJump(lastIterJump);
}

void BytecodeCompiler::visit(WhileStmt& node) {
    size_t loopStart = currentChunk_->code.size();
    compileExpr(node.condition);
    size_t exitJump = emitJump(OpCode::OP_JUMP_IF_FALSE);

    compileBlock(node.body);
    emitLoop(loopStart);

    patchJump(exitJump);
}

void BytecodeCompiler::visit(RepeatStmt& node) {
    size_t loopStart = currentChunk_->code.size();
    compileBlock(node.body);
    compileExpr(node.condition);
    // Repeats until condition is true, so jump if condition is false
    emitOp(OpCode::OP_JUMP_IF_FALSE);
    // Backward jump: target is loopStart
    int32_t backOffset = static_cast<int32_t>(loopStart) - static_cast<int32_t>(currentChunk_->code.size() + 4);
    emitInt32(backOffset);
}

bool BytecodeCompiler::compileRefExpr(const std::shared_ptr<Expr>& expr) {
    if (!expr) return false;
    currentLine_ = expr->line > 0 ? expr->line : currentLine_;

    if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(expr)) {
        int localIdx = isInsideSubroutine_ ? findLocal(varExpr->name) : -1;
        if (localIdx != -1) {
            if (locals_[localIdx].byRef) {
                emitOp(OpCode::OP_PUSH_REF_REF);
            } else {
                emitOp(OpCode::OP_PUSH_REF_LOCAL);
            }
            emitUint32(static_cast<uint32_t>(localIdx));
        } else {
            emitOp(OpCode::OP_PUSH_REF_GLOBAL);
            emitUint32(addName(varExpr->name));
        }
        return true;
    }

    if (auto arrExpr = std::dynamic_pointer_cast<ArrayAccessExpr>(expr)) {
        if (arrExpr->target) {
            if (!compileRefExpr(arrExpr->target)) return false;
        } else {
            int localIdx = isInsideSubroutine_ ? findLocal(arrExpr->name) : -1;
            if (localIdx != -1) {
                if (locals_[localIdx].byRef) {
                    emitOp(OpCode::OP_PUSH_REF_REF);
                } else {
                    emitOp(OpCode::OP_PUSH_REF_LOCAL);
                }
                emitUint32(static_cast<uint32_t>(localIdx));
            } else {
                emitOp(OpCode::OP_PUSH_REF_GLOBAL);
                emitUint32(addName(arrExpr->name));
            }
        }
        compileExpr(arrExpr->index);
        if (arrExpr->index2) {
            compileExpr(arrExpr->index2);
            emitOp(OpCode::OP_REF_ARRAY2D);
        } else {
            emitOp(OpCode::OP_REF_ARRAY);
        }
        return true;
    }

    if (auto memExpr = std::dynamic_pointer_cast<MemberAccessExpr>(expr)) {
        if (!compileRefExpr(memExpr->object)) return false;
        emitOp(OpCode::OP_REF_MEMBER);
        emitUint32(addName(memExpr->member));
        return true;
    }

    if (auto derefExpr = std::dynamic_pointer_cast<DerefExpr>(expr)) {
        if (!compileRefExpr(derefExpr->target)) {
            compileExpr(derefExpr->target);
        }
        emitOp(OpCode::OP_REF_DEREF);
        return true;
    }

    return false;
}

void BytecodeCompiler::compileCallArgument(const std::shared_ptr<Expr>& arg, bool isByRef) {
    if (isByRef) {
        if (compileRefExpr(arg)) {
            return;
        }
    }
    compileExpr(arg);
}

void BytecodeCompiler::visit(CallStmt& node) {
    std::string norm = normalize(node.name);
    if ((norm == "lire_ligne" || norm == "lireligne") && node.args.size() >= 2) {
        if (compileRefExpr(node.args[1])) {
            compileExpr(node.args[0]);
            emitOp(OpCode::OP_CALL_BUILTIN);
            emitUint32(BI_LIRE_LIGNE);
            emitUint32(1);
            emitOp(OpCode::OP_SET_LVALUE);
            return;
        }
    }

    bool isUserProc = (paramByRefMap_.find(norm) != paramByRefMap_.end());
    auto bit = builtinIds_.find(norm);
    if (!isUserProc && bit != builtinIds_.end()) {
        for (const auto& arg : node.args) {
            compileExpr(arg);
        }
        emitOp(OpCode::OP_CALL_BUILTIN);
        emitUint32(bit->second);
        emitUint32(static_cast<uint32_t>(node.args.size()));
        emitOp(OpCode::OP_POP); // discard result
        return;
    }

    const std::vector<bool>* refFlags = nullptr;
    auto rit = paramByRefMap_.find(norm);
    if (rit != paramByRefMap_.end()) {
        refFlags = &rit->second;
    }

    for (size_t i = 0; i < node.args.size(); ++i) {
        bool isRef = (refFlags && i < refFlags->size()) ? (*refFlags)[i] : false;
        compileCallArgument(node.args[i], isRef);
    }
    emitOp(OpCode::OP_CALL);
    emitUint32(addName(node.name));
    emitUint32(static_cast<uint32_t>(node.args.size()));
    emitOp(OpCode::OP_POP); // discard result
}

void BytecodeCompiler::visit(SwitchStmt& node) {
    // Evaluate target expr and keep it on stack
    compileExpr(node.expr);

    std::vector<size_t> endJumps;
    const SwitchCase* defaultCase = nullptr;

    for (const auto& sc : node.cases) {
        if (sc.isDefault) {
            defaultCase = &sc;
            continue;
        }

        std::vector<size_t> matchJumps;
        if (!sc.items.empty()) {
            for (const auto& item : sc.items) {
                if (item.isRange()) {
                    emitOp(OpCode::OP_DUP); // dup target
                    compileExpr(item.start);
                    emitOp(OpCode::OP_GE);
                    size_t failJump = emitJump(OpCode::OP_JUMP_IF_FALSE);

                    emitOp(OpCode::OP_DUP); // dup target
                    compileExpr(item.end);
                    emitOp(OpCode::OP_LE);
                    matchJumps.push_back(emitJump(OpCode::OP_JUMP_IF_TRUE));

                    patchJump(failJump);
                } else {
                    emitOp(OpCode::OP_DUP);
                    compileExpr(item.start);
                    emitOp(OpCode::OP_EQ);
                    matchJumps.push_back(emitJump(OpCode::OP_JUMP_IF_TRUE));
                }
            }
        } else {
            for (const auto& valExpr : sc.values) {
                emitOp(OpCode::OP_DUP);
                compileExpr(valExpr);
                emitOp(OpCode::OP_EQ);
                matchJumps.push_back(emitJump(OpCode::OP_JUMP_IF_TRUE));
            }
        }

        size_t nextCaseJump = emitJump(OpCode::OP_JUMP);

        // Body for matched case
        for (size_t mj : matchJumps) {
            patchJump(mj);
        }
        compileBlock(sc.body);
        endJumps.push_back(emitJump(OpCode::OP_JUMP));

        patchJump(nextCaseJump);
    }

    if (defaultCase != nullptr) {
        compileBlock(defaultCase->body);
    }

    for (size_t ej : endJumps) {
        patchJump(ej);
    }

    emitOp(OpCode::OP_POP); // pop original target expr
}

void BytecodeCompiler::visit(ReturnStmt& node) {
    compileExpr(node.value);
    emitOp(OpCode::OP_RETURN);
}

void BytecodeCompiler::visit(LocalVarDeclStmt& node) {
    for (const auto& name : node.names) {
        addLocal(name, false);
    }
}

// ---------------- Expressions ----------------

void BytecodeCompiler::visit(NumberExpr& node) {
    emitOp(OpCode::OP_CONST);
    emitUint32(addConstant(Value(node.value)));
}

void BytecodeCompiler::visit(RealExpr& node) {
    emitOp(OpCode::OP_CONST);
    emitUint32(addConstant(Value(node.value)));
}

void BytecodeCompiler::visit(StringExpr& node) {
    emitOp(OpCode::OP_CONST);
    emitUint32(addConstant(Value(node.value)));
}

void BytecodeCompiler::visit(BoolExpr& node) {
    emitOp(OpCode::OP_CONST);
    emitUint32(addConstant(Value(node.value)));
}

void BytecodeCompiler::visit(VariableExpr& node) {
    int localIdx = isInsideSubroutine_ ? findLocal(node.name) : -1;
    if (localIdx != -1) {
        emitOp(locals_[localIdx].byRef ? OpCode::OP_GET_REF : OpCode::OP_GET_LOCAL);
        emitUint32(static_cast<uint32_t>(localIdx));
    } else {
        emitOp(OpCode::OP_GET_GLOBAL);
        emitUint32(addName(node.name));
    }
}

void BytecodeCompiler::visit(ArrayAccessExpr& node) {
    std::string arrName;
    if (!node.target && !node.name.empty()) {
        arrName = node.name;
    } else if (node.target) {
        if (auto vExpr = std::dynamic_pointer_cast<VariableExpr>(node.target)) {
            arrName = vExpr->name;
        }
    }

    if (!arrName.empty() && !node.index2) {
        compileExpr(node.index);
        int localIdx = isInsideSubroutine_ ? findLocal(arrName) : -1;
        if (localIdx != -1) {
            emitOp(locals_[localIdx].byRef ? OpCode::OP_ARRAY_GET_REF : OpCode::OP_ARRAY_GET_LOCAL);
            emitUint32(static_cast<uint32_t>(localIdx));
        } else {
            emitOp(OpCode::OP_ARRAY_GET_GLOBAL);
            emitUint32(addName(arrName));
        }
        return;
    }

    if (node.target) {
        compileExpr(node.target);
    } else {
        int localIdx = isInsideSubroutine_ ? findLocal(node.name) : -1;
        if (localIdx != -1) {
            emitOp(locals_[localIdx].byRef ? OpCode::OP_GET_REF : OpCode::OP_GET_LOCAL);
            emitUint32(static_cast<uint32_t>(localIdx));
        } else {
            emitOp(OpCode::OP_GET_GLOBAL);
            emitUint32(addName(node.name));
        }
    }

    compileExpr(node.index);
    if (node.index2) {
        compileExpr(node.index2);
        emitOp(OpCode::OP_ARRAY2D_GET);
    } else {
        emitOp(OpCode::OP_ARRAY_GET);
    }
}

void BytecodeCompiler::visit(MemberAccessExpr& node) {
    compileExpr(node.object);
    emitOp(OpCode::OP_RECORD_GET);
    emitUint32(addName(node.member));
}

void BytecodeCompiler::visit(UnaryExpr& node) {
    compileExpr(node.operand);
    if (node.op == "-") {
        emitOp(OpCode::OP_NEG);
    } else if (node.op == "Non" || normalize(node.op) == "non") {
        emitOp(OpCode::OP_NOT);
    }
}

void BytecodeCompiler::visit(BinaryExpr& node) {
    if (node.op == "Et") {
        compileExpr(node.left);
        size_t shortCircuit = emitJump(OpCode::OP_JUMP_IF_FALSE_PEEK);
        compileExpr(node.right);
        patchJump(shortCircuit);
        return;
    }
    if (node.op == "Ou") {
        compileExpr(node.left);
        size_t shortCircuit = emitJump(OpCode::OP_JUMP_IF_TRUE_PEEK);
        compileExpr(node.right);
        patchJump(shortCircuit);
        return;
    }

    compileExpr(node.left);
    compileExpr(node.right);

    const std::string& op = node.op;
    if (op == "+") emitOp(OpCode::OP_ADD);
    else if (op == "-") emitOp(OpCode::OP_SUB);
    else if (op == "*") emitOp(OpCode::OP_MUL);
    else if (op == "^" || op == "**") emitOp(OpCode::OP_POW);
    else if (op == "/") emitOp(OpCode::OP_DIV_REAL);
    else if (op == "div") emitOp(OpCode::OP_DIV_INT);
    else if (op == "mod" || op == "%") emitOp(OpCode::OP_MOD);
    else if (op == "=") emitOp(OpCode::OP_EQ);
    else if (op == "≠" || op == "!=") emitOp(OpCode::OP_NE);
    else if (op == "<") emitOp(OpCode::OP_LT);
    else if (op == "<=" || op == "≤") emitOp(OpCode::OP_LE);
    else if (op == ">") emitOp(OpCode::OP_GT);
    else if (op == ">=" || op == "≥") emitOp(OpCode::OP_GE);
    else if (op == "∈") emitOp(OpCode::OP_IN);
}

void BytecodeCompiler::visit(CallExpr& node) {
    std::string norm = normalize(node.name);
    bool isUserFunc = (isProcedureMap_.find(norm) != isProcedureMap_.end());

    if (!isUserFunc) {
        auto bit = builtinIds_.find(norm);
        if (bit != builtinIds_.end()) {
            for (const auto& arg : node.args) {
                compileExpr(arg);
            }
            emitOp(OpCode::OP_CALL_BUILTIN);
            emitUint32(bit->second);
            emitUint32(static_cast<uint32_t>(node.args.size()));
            return;
        }
    }

    const std::vector<bool>* refFlags = nullptr;
    auto rit = paramByRefMap_.find(norm);
    if (rit != paramByRefMap_.end()) {
        refFlags = &rit->second;
    }

    for (size_t i = 0; i < node.args.size(); ++i) {
        bool isRef = (refFlags && i < refFlags->size()) ? (*refFlags)[i] : false;
        compileCallArgument(node.args[i], isRef);
    }
    emitOp(OpCode::OP_CALL);
    emitUint32(addName(node.name));
    emitUint32(static_cast<uint32_t>(node.args.size()));
}

void BytecodeCompiler::visit(SetLiteralExpr& node) {
    for (const auto& elem : node.elements) {
        compileExpr(elem);
    }
    emitOp(OpCode::OP_MAKE_SET);
    emitUint32(static_cast<uint32_t>(node.elements.size()));
}

void BytecodeCompiler::visit(DerefExpr& node) {
    compileExpr(node.target);
    emitOp(OpCode::OP_DEREF);
}

void BytecodeCompiler::visit(NilExpr& node) {
    emitOp(OpCode::OP_CONST);
    emitUint32(addConstant(Value::makeNil()));
}

} // namespace algo
