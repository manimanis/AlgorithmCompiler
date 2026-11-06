#pragma once

#include "value.hpp"
#include "arena.hpp"

namespace algo {

// Forward declarations for Expressions
struct NumberExpr;
struct RealExpr;
struct StringExpr;
struct BoolExpr;
struct VariableExpr;
struct ArrayAccessExpr;
struct MemberAccessExpr;
struct UnaryExpr;
struct BinaryExpr;
struct CallExpr;
struct SetLiteralExpr;
struct DerefExpr;
struct NilExpr;

// Forward declarations for Statements
struct AssignStmt;
struct ArrayAssignStmt;
struct MemberAssignStmt;
struct DerefAssignStmt;
struct AllocStmt;
struct FreeStmt;
struct WriteStmt;
struct ReadStmt;
struct IfStmt;
struct ForStmt;
struct WhileStmt;
struct RepeatStmt;
struct CallStmt;
struct SwitchStmt;
struct ReturnStmt;
struct LocalVarDeclStmt;

enum class ExprKind {
    Number,
    Real,
    String,
    Bool,
    Variable,
    ArrayAccess,
    MemberAccess,
    Unary,
    Binary,
    Call,
    SetLiteral,
    Deref,
    Nil
};

enum class StmtKind {
    Assign,
    ArrayAssign,
    MemberAssign,
    DerefAssign,
    Alloc,
    Free,
    Write,
    Read,
    If,
    For,
    While,
    Repeat,
    Call,
    Switch,
    Return,
    LocalVarDecl
};

class ExprVisitor {
public:
    virtual ~ExprVisitor() = default;
    virtual void visit(NumberExpr& node) = 0;
    virtual void visit(RealExpr& node) = 0;
    virtual void visit(StringExpr& node) = 0;
    virtual void visit(BoolExpr& node) = 0;
    virtual void visit(VariableExpr& node) = 0;
    virtual void visit(ArrayAccessExpr& node) = 0;
    virtual void visit(MemberAccessExpr& node) = 0;
    virtual void visit(UnaryExpr& node) = 0;
    virtual void visit(BinaryExpr& node) = 0;
    virtual void visit(CallExpr& node) = 0;
    virtual void visit(SetLiteralExpr& node) = 0;
    virtual void visit(DerefExpr& node) = 0;
    virtual void visit(NilExpr& node) = 0;
};

class StmtVisitor {
public:
    virtual ~StmtVisitor() = default;
    virtual void visit(AssignStmt& node) = 0;
    virtual void visit(ArrayAssignStmt& node) = 0;
    virtual void visit(MemberAssignStmt& node) = 0;
    virtual void visit(DerefAssignStmt& node) = 0;
    virtual void visit(AllocStmt& node) = 0;
    virtual void visit(FreeStmt& node) = 0;
    virtual void visit(WriteStmt& node) = 0;
    virtual void visit(ReadStmt& node) = 0;
    virtual void visit(IfStmt& node) = 0;
    virtual void visit(ForStmt& node) = 0;
    virtual void visit(WhileStmt& node) = 0;
    virtual void visit(RepeatStmt& node) = 0;
    virtual void visit(CallStmt& node) = 0;
    virtual void visit(SwitchStmt& node) = 0;
    virtual void visit(ReturnStmt& node) = 0;
    virtual void visit(LocalVarDeclStmt& node) = 0;
};

// Base Expression
struct Expr {
    ExprKind kind;
    int line = -1;

    explicit Expr(ExprKind k, int l = -1) : kind(k), line(l) {}
    virtual ~Expr() = default;
    virtual void accept(ExprVisitor& visitor) = 0;
};

struct NumberExpr : public Expr {
    int64_t value;
    NumberExpr(int64_t v, int line = -1) : Expr(ExprKind::Number, line), value(v) {}
    void accept(ExprVisitor& visitor) override { visitor.visit(*this); }
};

struct RealExpr : public Expr {
    double value;
    RealExpr(double v, int line = -1) : Expr(ExprKind::Real, line), value(v) {}
    void accept(ExprVisitor& visitor) override { visitor.visit(*this); }
};

struct StringExpr : public Expr {
    std::string value;
    StringExpr(std::string v, int line = -1) : Expr(ExprKind::String, line), value(std::move(v)) {}
    void accept(ExprVisitor& visitor) override { visitor.visit(*this); }
};

struct BoolExpr : public Expr {
    bool value;
    BoolExpr(bool v, int line = -1) : Expr(ExprKind::Bool, line), value(v) {}
    void accept(ExprVisitor& visitor) override { visitor.visit(*this); }
};

struct VariableExpr : public Expr {
    std::string name;
    VariableExpr(std::string n, int line = -1) : Expr(ExprKind::Variable, line), name(std::move(n)) {}
    void accept(ExprVisitor& visitor) override { visitor.visit(*this); }
};

struct ArrayAccessExpr : public Expr {
    std::shared_ptr<Expr> target;
    std::string name;
    std::shared_ptr<Expr> index;
    std::shared_ptr<Expr> index2; // for 2D
    ArrayAccessExpr(std::string n, std::shared_ptr<Expr> idx, std::shared_ptr<Expr> idx2 = nullptr, int line = -1)
        : Expr(ExprKind::ArrayAccess, line), target(nullptr), name(std::move(n)), index(std::move(idx)), index2(std::move(idx2)) {}
    ArrayAccessExpr(std::shared_ptr<Expr> tgt, std::shared_ptr<Expr> idx, std::shared_ptr<Expr> idx2 = nullptr, int line = -1)
        : Expr(ExprKind::ArrayAccess, line), target(tgt), index(std::move(idx)), index2(std::move(idx2)) {
        if (auto v = std::dynamic_pointer_cast<VariableExpr>(tgt)) {
            name = v->name;
        }
    }
    void accept(ExprVisitor& visitor) override { visitor.visit(*this); }
};

struct MemberAccessExpr : public Expr {
    std::shared_ptr<Expr> object;
    std::string member;
    MemberAccessExpr(std::shared_ptr<Expr> obj, std::string m, int line = -1)
        : Expr(ExprKind::MemberAccess, line), object(std::move(obj)), member(std::move(m)) {}
    void accept(ExprVisitor& visitor) override { visitor.visit(*this); }
};

struct UnaryExpr : public Expr {
    std::string op;
    std::shared_ptr<Expr> operand;
    UnaryExpr(std::string op, std::shared_ptr<Expr> operand, int line = -1)
        : Expr(ExprKind::Unary, line), op(std::move(op)), operand(std::move(operand)) {}
    void accept(ExprVisitor& visitor) override { visitor.visit(*this); }
};

struct BinaryExpr : public Expr {
    std::string op;
    std::shared_ptr<Expr> left;
    std::shared_ptr<Expr> right;
    BinaryExpr(std::string op, std::shared_ptr<Expr> left, std::shared_ptr<Expr> right, int line = -1)
        : Expr(ExprKind::Binary, line), op(std::move(op)), left(std::move(left)), right(std::move(right)) {}
    void accept(ExprVisitor& visitor) override { visitor.visit(*this); }
};

struct CallExpr : public Expr {
    std::string name;
    std::vector<std::shared_ptr<Expr>> args;
    CallExpr(std::string n, std::vector<std::shared_ptr<Expr>> a, int line = -1)
        : Expr(ExprKind::Call, line), name(std::move(n)), args(std::move(a)) {}
    void accept(ExprVisitor& visitor) override { visitor.visit(*this); }
};

struct SetLiteralExpr : public Expr {
    std::vector<std::shared_ptr<Expr>> elements;
    SetLiteralExpr(std::vector<std::shared_ptr<Expr>> el, int line = -1)
        : Expr(ExprKind::SetLiteral, line), elements(std::move(el)) {}
    void accept(ExprVisitor& visitor) override { visitor.visit(*this); }
};

struct DerefExpr : public Expr {
    std::shared_ptr<Expr> target;
    DerefExpr(std::shared_ptr<Expr> tgt, int line = -1)
        : Expr(ExprKind::Deref, line), target(std::move(tgt)) {}
    void accept(ExprVisitor& visitor) override { visitor.visit(*this); }
};

struct NilExpr : public Expr {
    NilExpr(int line = -1) : Expr(ExprKind::Nil, line) {}
    void accept(ExprVisitor& visitor) override { visitor.visit(*this); }
};

// Base Statement
struct Stmt {
    StmtKind kind;
    int line = -1;

    explicit Stmt(StmtKind k, int l = -1) : kind(k), line(l) {}
    virtual ~Stmt() = default;
    virtual void accept(StmtVisitor& visitor) = 0;
};

struct AssignStmt : public Stmt {
    std::string target;
    std::shared_ptr<Expr> value;
    AssignStmt(std::string t, std::shared_ptr<Expr> v, int line = -1)
        : Stmt(StmtKind::Assign, line), target(std::move(t)), value(std::move(v)) {}
    void accept(StmtVisitor& visitor) override { visitor.visit(*this); }
};

struct ArrayAssignStmt : public Stmt {
    std::shared_ptr<Expr> targetExpr;
    std::string target;
    std::shared_ptr<Expr> index;
    std::shared_ptr<Expr> index2; // for 2D
    std::shared_ptr<Expr> value;
    ArrayAssignStmt(std::string t, std::shared_ptr<Expr> idx, std::shared_ptr<Expr> val, std::shared_ptr<Expr> idx2 = nullptr, int line = -1)
        : Stmt(StmtKind::ArrayAssign, line), targetExpr(nullptr), target(std::move(t)), index(std::move(idx)), index2(std::move(idx2)), value(std::move(val)) {}
    ArrayAssignStmt(std::shared_ptr<Expr> tgt, std::shared_ptr<Expr> idx, std::shared_ptr<Expr> val, std::shared_ptr<Expr> idx2 = nullptr, int line = -1)
        : Stmt(StmtKind::ArrayAssign, line), targetExpr(std::move(tgt)), target(""), index(std::move(idx)), index2(std::move(idx2)), value(std::move(val)) {}
    void accept(StmtVisitor& visitor) override { visitor.visit(*this); }
};

struct MemberAssignStmt : public Stmt {
    std::shared_ptr<Expr> target;
    std::string member;
    std::shared_ptr<Expr> value;
    MemberAssignStmt(std::shared_ptr<Expr> t, std::string m, std::shared_ptr<Expr> v, int line = -1)
        : Stmt(StmtKind::MemberAssign, line), target(std::move(t)), member(std::move(m)), value(std::move(v)) {}
    void accept(StmtVisitor& visitor) override { visitor.visit(*this); }
};

struct DerefAssignStmt : public Stmt {
    std::shared_ptr<Expr> target;
    std::shared_ptr<Expr> value;
    DerefAssignStmt(std::shared_ptr<Expr> t, std::shared_ptr<Expr> v, int line = -1)
        : Stmt(StmtKind::DerefAssign, line), target(std::move(t)), value(std::move(v)) {}
    void accept(StmtVisitor& visitor) override { visitor.visit(*this); }
};

struct AllocStmt : public Stmt {
    std::shared_ptr<Expr> target;
    AllocStmt(std::shared_ptr<Expr> t, int line = -1)
        : Stmt(StmtKind::Alloc, line), target(std::move(t)) {}
    void accept(StmtVisitor& visitor) override { visitor.visit(*this); }
};

struct FreeStmt : public Stmt {
    std::shared_ptr<Expr> target;
    FreeStmt(std::shared_ptr<Expr> t, int line = -1)
        : Stmt(StmtKind::Free, line), target(std::move(t)) {}
    void accept(StmtVisitor& visitor) override { visitor.visit(*this); }
};

struct WriteStmt : public Stmt {
    std::vector<std::shared_ptr<Expr>> args;
    std::string sep = " ";
    std::string fin = "\n";
    bool isNl = false;
    WriteStmt(std::vector<std::shared_ptr<Expr>> a, std::string sep = " ", std::string fin = "\n", int line = -1, bool isNl = false)
        : Stmt(StmtKind::Write, line), args(std::move(a)), sep(std::move(sep)), fin(std::move(fin)), isNl(isNl) {}
    void accept(StmtVisitor& visitor) override { visitor.visit(*this); }
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
        : Stmt(StmtKind::Read, line), targets(std::move(t)) {}
    void accept(StmtVisitor& visitor) override { visitor.visit(*this); }
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
        : Stmt(StmtKind::If, line), condition(std::move(cond)), thenBlock(std::move(th)),
          elseIfBlocks(std::move(ei)), elseBlock(std::move(el)) {}
    void accept(StmtVisitor& visitor) override { visitor.visit(*this); }
};

struct ForStmt : public Stmt {
    std::string varName;
    std::shared_ptr<Expr> start;
    std::shared_ptr<Expr> end;
    std::shared_ptr<Expr> step;
    std::vector<std::shared_ptr<Stmt>> body;
    ForStmt(std::string v, std::shared_ptr<Expr> s, std::shared_ptr<Expr> e,
            std::shared_ptr<Expr> st, std::vector<std::shared_ptr<Stmt>> b, int line = -1)
        : Stmt(StmtKind::For, line), varName(std::move(v)), start(std::move(s)), end(std::move(e)),
          step(std::move(st)), body(std::move(b)) {}
    void accept(StmtVisitor& visitor) override { visitor.visit(*this); }
};

struct WhileStmt : public Stmt {
    std::shared_ptr<Expr> condition;
    std::vector<std::shared_ptr<Stmt>> body;
    WhileStmt(std::shared_ptr<Expr> cond, std::vector<std::shared_ptr<Stmt>> b, int line = -1)
        : Stmt(StmtKind::While, line), condition(std::move(cond)), body(std::move(b)) {}
    void accept(StmtVisitor& visitor) override { visitor.visit(*this); }
};

struct RepeatStmt : public Stmt {
    std::vector<std::shared_ptr<Stmt>> body;
    std::shared_ptr<Expr> condition;
    RepeatStmt(std::vector<std::shared_ptr<Stmt>> b, std::shared_ptr<Expr> cond, int line = -1)
        : Stmt(StmtKind::Repeat, line), body(std::move(b)), condition(std::move(cond)) {}
    void accept(StmtVisitor& visitor) override { visitor.visit(*this); }
};

struct CallStmt : public Stmt {
    std::string name;
    std::vector<std::shared_ptr<Expr>> args;
    CallStmt(std::string n, std::vector<std::shared_ptr<Expr>> a, int line = -1)
        : Stmt(StmtKind::Call, line), name(std::move(n)), args(std::move(a)) {}
    void accept(StmtVisitor& visitor) override { visitor.visit(*this); }
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
        : Stmt(StmtKind::Switch, line), expr(std::move(e)), cases(std::move(c)) {}
    void accept(StmtVisitor& visitor) override { visitor.visit(*this); }
};

struct ReturnStmt : public Stmt {
    std::shared_ptr<Expr> value;
    ReturnStmt(std::shared_ptr<Expr> v, int line = -1)
        : Stmt(StmtKind::Return, line), value(std::move(v)) {}
    void accept(StmtVisitor& visitor) override { visitor.visit(*this); }
};

enum class TypeKind {
    Simple,
    Array1D,
    Array2D,
    Record,
    FileText,
    FileData,
    Enum,
    Subrange,
    Pointer
};

struct VarDecl;

struct TypeSpec {
    TypeKind kind = TypeKind::Simple;
    std::string name;
    int rows = 0;
    int cols = 0;
    std::string elemType;
    std::vector<std::shared_ptr<VarDecl>> fields;
    std::vector<std::string> enumValues;
    std::shared_ptr<Expr> subrangeMin;
    std::shared_ptr<Expr> subrangeMax;
    int64_t minInt = 0;
    int64_t maxInt = 0;
};

struct LocalVarDeclStmt : public Stmt {
    std::vector<std::string> names;
    std::string varType;
    TypeSpec typeSpec;
    LocalVarDeclStmt(std::vector<std::string> n, std::string t, int line = -1)
        : Stmt(StmtKind::LocalVarDecl, line), names(std::move(n)), varType(std::move(t)) {}
    void accept(StmtVisitor& visitor) override { visitor.visit(*this); }
};

struct ConstDecl {
    std::string name;
    std::shared_ptr<Expr> valueExpr;
    Value value;
    int line = 1;
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
    std::vector<ConstDecl> constDecls;
    std::vector<VarDecl> localVars;
    std::vector<std::shared_ptr<Stmt>> body;
    int line = -1;
};

struct FunctionDecl {
    std::string name;
    std::vector<ParamDecl> params;
    std::string returnType;
    std::vector<ConstDecl> constDecls;
    std::vector<VarDecl> localVars;
    std::vector<std::shared_ptr<Stmt>> body;
    int line = -1;
};

struct Program {
    std::string name;
    std::vector<TypeDecl> typeDecls;
    std::vector<ConstDecl> constDecls;
    std::vector<VarDecl> varDecls;
    std::vector<ProcedureDecl> procedures;
    std::vector<FunctionDecl> functions;
    std::vector<std::shared_ptr<Stmt>> mainBody;
    std::shared_ptr<Arena> arena;
};

} // namespace algo
