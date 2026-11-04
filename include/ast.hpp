#pragma once

#include "value.hpp"

namespace algo {

// Base Expression
struct Expr {
    virtual ~Expr() = default;
};

struct NumberExpr : public Expr {
    int64_t value;
    int line;
    NumberExpr(int64_t v, int line = -1) : value(v), line(line) {}
};

struct RealExpr : public Expr {
    double value;
    int line;
    RealExpr(double v, int line = -1) : value(v), line(line) {}
};

struct StringExpr : public Expr {
    std::string value;
    int line;
    StringExpr(std::string v, int line = -1) : value(std::move(v)), line(line) {}
};

struct BoolExpr : public Expr {
    bool value;
    int line;
    BoolExpr(bool v, int line = -1) : value(v), line(line) {}
};

struct VariableExpr : public Expr {
    std::string name;
    int line;
    VariableExpr(std::string n, int line = -1) : name(std::move(n)), line(line) {}
};

struct ArrayAccessExpr : public Expr {
    std::shared_ptr<Expr> target;
    std::string name;
    std::shared_ptr<Expr> index;
    std::shared_ptr<Expr> index2; // for 2D
    int line;
    ArrayAccessExpr(std::string n, std::shared_ptr<Expr> idx, std::shared_ptr<Expr> idx2 = nullptr, int line = -1)
        : target(nullptr), name(std::move(n)), index(std::move(idx)), index2(std::move(idx2)), line(line) {}
    ArrayAccessExpr(std::shared_ptr<Expr> tgt, std::shared_ptr<Expr> idx, std::shared_ptr<Expr> idx2 = nullptr, int line = -1)
        : target(tgt), index(std::move(idx)), index2(std::move(idx2)), line(line) {
        if (auto v = std::dynamic_pointer_cast<VariableExpr>(tgt)) {
            name = v->name;
        }
    }
};

struct MemberAccessExpr : public Expr {
    std::shared_ptr<Expr> object;
    std::string member;
    int line;
    MemberAccessExpr(std::shared_ptr<Expr> obj, std::string m, int line = -1)
        : object(std::move(obj)), member(std::move(m)), line(line) {}
};

struct UnaryExpr : public Expr {
    std::string op;
    std::shared_ptr<Expr> operand;
    int line;
    UnaryExpr(std::string op, std::shared_ptr<Expr> operand, int line = -1)
        : op(std::move(op)), operand(std::move(operand)), line(line) {}
};

struct BinaryExpr : public Expr {
    std::string op;
    std::shared_ptr<Expr> left;
    std::shared_ptr<Expr> right;
    int line;
    BinaryExpr(std::string op, std::shared_ptr<Expr> left, std::shared_ptr<Expr> right, int line = -1)
        : op(std::move(op)), left(std::move(left)), right(std::move(right)), line(line) {}
};

struct CallExpr : public Expr {
    std::string name;
    std::vector<std::shared_ptr<Expr>> args;
    int line;
    CallExpr(std::string n, std::vector<std::shared_ptr<Expr>> a, int line = -1)
        : name(std::move(n)), args(std::move(a)), line(line) {}
};

struct SetLiteralExpr : public Expr {
    std::vector<std::shared_ptr<Expr>> elements;
    int line;
    SetLiteralExpr(std::vector<std::shared_ptr<Expr>> el, int line = -1)
        : elements(std::move(el)), line(line) {}
};

// Base Statement
struct Stmt {
    int line = -1;
    virtual ~Stmt() = default;
};

struct AssignStmt : public Stmt {
    std::string target;
    std::shared_ptr<Expr> value;
    AssignStmt(std::string t, std::shared_ptr<Expr> v, int line = -1)
        : target(std::move(t)), value(std::move(v)) { this->line = line; }
};

struct ArrayAssignStmt : public Stmt {
    std::shared_ptr<Expr> targetExpr;
    std::string target;
    std::shared_ptr<Expr> index;
    std::shared_ptr<Expr> index2; // for 2D
    std::shared_ptr<Expr> value;
    ArrayAssignStmt(std::string t, std::shared_ptr<Expr> idx, std::shared_ptr<Expr> val, std::shared_ptr<Expr> idx2 = nullptr, int line = -1)
        : targetExpr(nullptr), target(std::move(t)), index(std::move(idx)), index2(std::move(idx2)), value(std::move(val)) { this->line = line; }
    ArrayAssignStmt(std::shared_ptr<Expr> tgt, std::shared_ptr<Expr> idx, std::shared_ptr<Expr> val, std::shared_ptr<Expr> idx2 = nullptr, int line = -1)
        : targetExpr(std::move(tgt)), target(""), index(std::move(idx)), index2(std::move(idx2)), value(std::move(val)) { this->line = line; }
};

struct MemberAssignStmt : public Stmt {
    std::shared_ptr<Expr> target;
    std::string member;
    std::shared_ptr<Expr> value;
    MemberAssignStmt(std::shared_ptr<Expr> t, std::string m, std::shared_ptr<Expr> v, int line = -1)
        : target(std::move(t)), member(std::move(m)), value(std::move(v)) { this->line = line; }
};

struct WriteStmt : public Stmt {
    std::vector<std::shared_ptr<Expr>> args;
    std::string sep = " ";
    std::string fin = "\n";
    bool isNl = false;
    WriteStmt(std::vector<std::shared_ptr<Expr>> a, std::string sep = " ", std::string fin = "\n", int line = -1, bool isNl = false)
        : args(std::move(a)), sep(std::move(sep)), fin(std::move(fin)), isNl(isNl) { this->line = line; }
};

struct ReadTarget {
    bool isArray = false;
    bool is2D = false;
    std::string name;
    std::shared_ptr<Expr> index;
    std::shared_ptr<Expr> index2;
    std::string member;
    std::shared_ptr<Expr> targetExpr;
};

struct ReadStmt : public Stmt {
    std::vector<ReadTarget> targets;
    ReadStmt(std::vector<ReadTarget> t, int line = -1)
        : targets(std::move(t)) { this->line = line; }
};

struct ElseIfBlock {
    std::shared_ptr<Expr> condition;
    std::vector<std::shared_ptr<Stmt>> body;
};

struct IfStmt : public Stmt {
    std::shared_ptr<Expr> condition;
    std::vector<std::shared_ptr<Stmt>> thenBlock;
    std::vector<ElseIfBlock> elseIfBlocks;
    std::vector<std::shared_ptr<Stmt>> elseBlock;
    IfStmt(std::shared_ptr<Expr> cond, std::vector<std::shared_ptr<Stmt>> th,
           std::vector<ElseIfBlock> ei = {}, std::vector<std::shared_ptr<Stmt>> el = {}, int line = -1)
        : condition(std::move(cond)), thenBlock(std::move(th)),
          elseIfBlocks(std::move(ei)), elseBlock(std::move(el)) { this->line = line; }
};

struct ForStmt : public Stmt {
    std::string varName;
    std::shared_ptr<Expr> start;
    std::shared_ptr<Expr> end;
    std::shared_ptr<Expr> step;
    std::vector<std::shared_ptr<Stmt>> body;
    ForStmt(std::string v, std::shared_ptr<Expr> s, std::shared_ptr<Expr> e,
            std::shared_ptr<Expr> st, std::vector<std::shared_ptr<Stmt>> b, int line = -1)
        : varName(std::move(v)), start(std::move(s)), end(std::move(e)),
          step(std::move(st)), body(std::move(b)) { this->line = line; }
};

struct WhileStmt : public Stmt {
    std::shared_ptr<Expr> condition;
    std::vector<std::shared_ptr<Stmt>> body;
    WhileStmt(std::shared_ptr<Expr> cond, std::vector<std::shared_ptr<Stmt>> b, int line = -1)
        : condition(std::move(cond)), body(std::move(b)) { this->line = line; }
};

struct RepeatStmt : public Stmt {
    std::vector<std::shared_ptr<Stmt>> body;
    std::shared_ptr<Expr> condition;
    RepeatStmt(std::vector<std::shared_ptr<Stmt>> b, std::shared_ptr<Expr> cond, int line = -1)
        : body(std::move(b)), condition(std::move(cond)) { this->line = line; }
};

struct CallStmt : public Stmt {
    std::string name;
    std::vector<std::shared_ptr<Expr>> args;
    CallStmt(std::string n, std::vector<std::shared_ptr<Expr>> a, int line = -1)
        : name(std::move(n)), args(std::move(a)) { this->line = line; }
};

struct SwitchCaseItem {
    std::shared_ptr<Expr> start;
    std::shared_ptr<Expr> end; // nullptr if single value, non-null if start .. end
    bool isRange() const { return end != nullptr; }
};

struct SwitchCase {
    std::vector<SwitchCaseItem> items;
    std::vector<std::shared_ptr<Expr>> values;
    bool isDefault = false;
    std::vector<std::shared_ptr<Stmt>> body;
};

struct SwitchStmt : public Stmt {
    std::shared_ptr<Expr> expr;
    std::vector<SwitchCase> cases;
    SwitchStmt(std::shared_ptr<Expr> e, std::vector<SwitchCase> c, int line = -1)
        : expr(std::move(e)), cases(std::move(c)) { this->line = line; }
};

struct ReturnStmt : public Stmt {
    std::shared_ptr<Expr> value;
    ReturnStmt(std::shared_ptr<Expr> v, int line = -1)
        : value(std::move(v)) { this->line = line; }
};

enum class TypeKind {
    Simple,
    Array1D,
    Array2D,
    Record,
    FileText,
    FileData
};

struct VarDecl;

struct TypeSpec {
    TypeKind kind = TypeKind::Simple;
    std::string name;
    int rows = 0;
    int cols = 0;
    std::string elemType;
    std::vector<std::shared_ptr<VarDecl>> fields;
};

struct LocalVarDeclStmt : public Stmt {
    std::vector<std::string> names;
    std::string varType;
    TypeSpec typeSpec;
    LocalVarDeclStmt(std::vector<std::string> n, std::string t, int line = -1)
        : names(std::move(n)), varType(std::move(t)) { this->line = line; }
};

// Declarations
struct TypeDecl {
    std::string name;
    TypeSpec typeSpec;
    int arraySize = 0;
    std::string elemType;
    int line = -1;
};

struct VarDecl {
    std::vector<std::string> names;
    std::string varType;
    TypeSpec typeSpec;
    int line = -1;
};

struct ParamDecl {
    std::string name;
    std::string paramType;
    bool byRef = false;
};

struct ProcedureDecl {
    std::string name;
    std::vector<ParamDecl> params;
    std::vector<VarDecl> localVars;
    std::vector<std::shared_ptr<Stmt>> body;
    int line = -1;
};

struct FunctionDecl {
    std::string name;
    std::vector<ParamDecl> params;
    std::string returnType;
    std::vector<VarDecl> localVars;
    std::vector<std::shared_ptr<Stmt>> body;
    int line = -1;
};

struct Program {
    std::string name;
    std::vector<TypeDecl> typeDecls;
    std::vector<VarDecl> varDecls;
    std::vector<ProcedureDecl> procedures;
    std::vector<FunctionDecl> functions;
    std::vector<std::shared_ptr<Stmt>> mainBody;
};

} // namespace algo