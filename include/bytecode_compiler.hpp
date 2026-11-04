#pragma once

#include "bytecode.hpp"
#include "ast.hpp"
#include <unordered_map>
#include <string>
#include <vector>

namespace algo {

struct LocalVarInfo {
    std::string name;
    uint32_t slot;
    bool byRef = false;
};

class BytecodeCompiler : public StmtVisitor, public ExprVisitor {
public:
    BytecodeCompiler();
    BytecodeProgram compile(const Program& prog);

    // StmtVisitor
    void visit(AssignStmt& node) override;
    void visit(ArrayAssignStmt& node) override;
    void visit(MemberAssignStmt& node) override;
    void visit(WriteStmt& node) override;
    void visit(ReadStmt& node) override;
    void visit(IfStmt& node) override;
    void visit(ForStmt& node) override;
    void visit(WhileStmt& node) override;
    void visit(RepeatStmt& node) override;
    void visit(CallStmt& node) override;
    void visit(SwitchStmt& node) override;
    void visit(ReturnStmt& node) override;
    void visit(LocalVarDeclStmt& node) override;

    // ExprVisitor
    void visit(NumberExpr& node) override;
    void visit(RealExpr& node) override;
    void visit(StringExpr& node) override;
    void visit(BoolExpr& node) override;
    void visit(VariableExpr& node) override;
    void visit(ArrayAccessExpr& node) override;
    void visit(MemberAccessExpr& node) override;
    void visit(UnaryExpr& node) override;
    void visit(BinaryExpr& node) override;
    void visit(CallExpr& node) override;
    void visit(SetLiteralExpr& node) override;

private:
    Chunk* currentChunk_ = nullptr;
    int currentLine_ = 1;

    std::vector<LocalVarInfo> locals_;
    bool isInsideSubroutine_ = false;

    std::unordered_map<std::string, bool> isProcedureMap_;
    std::unordered_map<std::string, uint32_t> builtinIds_;

    void emitByte(uint8_t byte);
    void emitOp(OpCode op);
    void emitUint32(uint32_t val);
    void emitInt32(int32_t val);

    size_t emitJump(OpCode jumpOp);
    void patchJump(size_t jumpOffset);
    void emitLoop(size_t loopStart);

    uint32_t addConstant(const Value& val);
    uint32_t addName(const std::string& name);

    int findLocal(const std::string& name) const;
    void addLocal(const std::string& name, bool byRef = false);

    void compileBlock(const std::vector<std::shared_ptr<Stmt>>& stmts);
    void compileExpr(const std::shared_ptr<Expr>& expr);
    void compileStmt(const std::shared_ptr<Stmt>& stmt);

    void initBuiltins();
};

} // namespace algo
