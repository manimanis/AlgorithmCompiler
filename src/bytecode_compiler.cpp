#include "../include/bytecode_compiler.hpp"

namespace algo {

enum BuiltinId : uint32_t {
    BI_RACINE = 1,
    BI_CARRE,
    BI_ABS,
    BI_EXP,
    BI_LN,
    BI_LOG,
    BI_ENT,
    BI_ARRONDI,
    BI_LONG,
    BI_CHR,
    BI_ORD,
    BI_CONVCH,
    BI_VALEUR,
    BI_SOUS_CHAINE,
    BI_POS,
    BI_MAJUS,
    BI_MINUS,
    BI_EFFACER,
    BI_ESTNUM,
    BI_ALEA,
    BI_GRAINE,
    BI_OUVRIR,
    BI_FERMER,
    BI_LIRE_LIGNE,
    BI_FIN_FICHIER
};

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

void BytecodeCompiler::compileExpr(const std::shared_ptr<Expr>& expr) {
    if (!expr) return;
    currentLine_ = expr->line > 0 ? expr->line : currentLine_;
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
    BytecodeProgram result;
    result.name = prog.name;
    result.typeDecls = prog.typeDecls;
    result.varDecls = prog.varDecls;

    for (const auto& proc : prog.procedures) {
        isProcedureMap_[normalize(proc.name)] = true;
    }
    for (const auto& func : prog.functions) {
        isProcedureMap_[normalize(func.name)] = false;
    }

    // Compile procedures
    for (const auto& proc : prog.procedures) {
        BytecodeFunction bf;
        bf.name = proc.name;
        bf.isProcedure = true;
        bf.localVars = proc.localVars;

        locals_.clear();
        isInsideSubroutine_ = true;

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
        compileBlock(proc.body);
        emitOp(OpCode::OP_RETURN_VOID);

        result.subroutines[normalize(proc.name)] = std::move(bf);
    }

    // Compile functions
    for (const auto& func : prog.functions) {
        BytecodeFunction bf;
        bf.name = func.name;
        bf.isProcedure = false;
        bf.returnType = func.returnType;
        bf.localVars = func.localVars;

        locals_.clear();
        isInsideSubroutine_ = true;

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
        compileBlock(func.body);
        // Default return null if no explicit return
        emitOp(OpCode::OP_CONST);
        emitUint32(addConstant(Value()));
        emitOp(OpCode::OP_RETURN);

        result.subroutines[normalize(func.name)] = std::move(bf);
    }

    // Compile main body
    locals_.clear();
    isInsideSubroutine_ = false;
    currentChunk_ = &result.mainChunk;

    compileBlock(prog.mainBody);
    emitOp(OpCode::OP_HALT);

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

void BytecodeCompiler::visit(ReadStmt& node) {
    for (const auto& tgt : node.targets) {
        if (tgt.isArray) {
            compileExpr(tgt.index);
            if (tgt.is2D) {
                compileExpr(tgt.index2);
                emitOp(OpCode::OP_READ_ARRAY2D);
            } else {
                emitOp(OpCode::OP_READ_ARRAY);
            }
            emitUint32(addName(tgt.name));
        } else if (!tgt.member.empty()) {
            emitOp(OpCode::OP_READ_RECORD);
            emitUint32(addName(tgt.name));
            emitUint32(addName(tgt.member));
        } else {
            int localIdx = isInsideSubroutine_ ? findLocal(tgt.name) : -1;
            if (localIdx != -1) {
                emitOp(locals_[localIdx].byRef ? OpCode::OP_READ_REF : OpCode::OP_READ_LOCAL);
                emitUint32(static_cast<uint32_t>(localIdx));
            } else {
                emitOp(OpCode::OP_READ_VAR);
                emitUint32(addName(tgt.name));
            }
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
            if (num->value < 0) isNegativeStep = true;
        }
    }
    emitOp(isNegativeStep ? OpCode::OP_GE : OpCode::OP_LE);

    size_t exitJump = emitJump(OpCode::OP_JUMP_IF_FALSE);

    // 3. Body
    compileBlock(node.body);

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

void BytecodeCompiler::visit(CallStmt& node) {
    std::string norm = normalize(node.name);
    auto bit = builtinIds_.find(norm);
    if (bit != builtinIds_.end()) {
        for (const auto& arg : node.args) {
            compileExpr(arg);
        }
        emitOp(OpCode::OP_CALL_BUILTIN);
        emitUint32(bit->second);
        emitUint32(static_cast<uint32_t>(node.args.size()));
        emitOp(OpCode::OP_POP); // discard result
        return;
    }

    for (const auto& arg : node.args) {
        compileExpr(arg);
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

    for (const auto& arg : node.args) {
        compileExpr(arg);
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

} // namespace algo
