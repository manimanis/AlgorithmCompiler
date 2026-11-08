#pragma once

#include "ast.hpp"
#include "environment.hpp"
#include <chrono>
#include <fstream>
#include <random>

namespace algo {

inline uint32_t generateRandomSeed() {
    uint32_t s = 0;
    try {
        std::random_device rd;
        s = rd();
    } catch (...) {
        s = 0;
    }
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    uint32_t timeVal = static_cast<uint32_t>(now ^ (now >> 32));
    if (s == 0) {
        s = timeVal;
    } else {
        s ^= timeVal;
    }
    return s != 0 ? s : 1;
}

struct InterpreterOptions {
    uint64_t maxIterations = 1000000;
    uint64_t timeoutMs = 50000;
    uint32_t seed = 0;
    bool hasSeed = false;
    bool leakCheck = false;

    InterpreterOptions() : seed(generateRandomSeed()), hasSeed(false) {}
    explicit InterpreterOptions(uint32_t explicitSeed) : seed(explicitSeed), hasSeed(true) {}

    static InterpreterOptions fromSource(const std::string& source);
    static InterpreterOptions fromSource(const std::string& source, const InterpreterOptions& base);
};

struct AlgoFile {
    std::string path;
    std::string mode;
    bool isText = true;
    std::string elemType;
    std::fstream stream;
    bool isOpen = false;
    std::vector<std::streampos> recordOffsets;
    int64_t currentRecordIndex = 0;
};

// Mulberry32 32-bit PRNG
class Mulberry32RNG {
public:
    uint32_t state;
    explicit Mulberry32RNG(uint32_t seed = 0) : state(seed) {}

    void reseed(uint32_t s) { state = s; }

    double nextDouble() {
        uint32_t t = (state += 0x6D2B79F5);
        t = (t ^ (t >> 15)) * (t | 1);
        t ^= t + ((t ^ (t >> 7)) * (t | 61));
        return (double)((t ^ (t >> 14))) / 4294967296.0;
    }

    int64_t randomInt(int64_t minVal, int64_t maxVal) {
        if (minVal > maxVal) std::swap(minVal, maxVal);
        double r = nextDouble();
        return (int64_t)std::floor(r * (double)(maxVal - minVal + 1)) + minVal;
    }
};

class Interpreter : public StmtVisitor, public ExprVisitor {
public:
    explicit Interpreter(InterpreterOptions options = InterpreterOptions());

    void setOutputCallback(std::function<void(const std::string&)> cb) { outputCallback_ = std::move(cb); }
    void setInputCallback(std::function<std::string(const std::string&)> cb) { inputCallback_ = std::move(cb); }

    void addInput(const std::string& input) { inputBuffer_.push_back(input); }
    const std::vector<std::string>& getOutput() const { return output_; }
    std::shared_ptr<Environment> getGlobalEnv() const { return globalEnv_; }
    const MemoryTracker& getMemoryTracker() const { return memoryTracker_; }
    MemoryTracker& getMemoryTracker() { return memoryTracker_; }
    const std::vector<StackFrame>& getCallStack() const { return callStack_; }

    void run(const Program& prog);

    Value evaluateExpression(const std::shared_ptr<Expr>& expr, const std::shared_ptr<Environment>& env);
    void executeBlock(const std::vector<std::shared_ptr<Stmt>>& stmts, const std::shared_ptr<Environment>& env);
    void executeStatement(const std::shared_ptr<Stmt>& stmt, const std::shared_ptr<Environment>& env);

    Value getDefaultValue(const std::string& typeName) const;
    Value getDefaultValue(const TypeSpec& ts, const std::string& varName = "") const;
    std::string resolveExprType(const std::shared_ptr<Expr>& expr) const;
    std::string resolvePointeeType(const std::shared_ptr<Expr>& expr) const;
    void assignToLValue(const std::shared_ptr<Expr>& lhs, const Value& val, const std::shared_ptr<Environment>& env, int line = -1);
    void writeFileData(AlgoFile& f, const Value& val);
    Value readFileData(AlgoFile& f);

    // StmtVisitor interface
    void visit(AssignStmt& node) override;
    void visit(ArrayAssignStmt& node) override;
    void visit(MemberAssignStmt& node) override;
    void visit(DerefAssignStmt& node) override;
    void visit(AllocStmt& node) override;
    void visit(FreeStmt& node) override;
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

    // ExprVisitor interface
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
    void visit(DerefExpr& node) override;
    void visit(NilExpr& node) override;

private:
    InterpreterOptions options_;
    std::shared_ptr<Environment> globalEnv_;
    std::unordered_map<std::string, TypeDecl> typeDefs_;
    std::unordered_map<std::string, int64_t> enumOrdinals_;
    std::unordered_map<std::string, ProcedureDecl> procedures_;
    std::unordered_map<std::string, FunctionDecl> functions_;
    std::vector<VarDecl> varDecls_;
    std::string currentSubroutine_;
    std::map<std::string, std::shared_ptr<AlgoFile>> openFiles_;

    std::vector<std::string> output_;
    std::vector<std::string> inputBuffer_;
    size_t inputBufferIdx_ = 0;

    std::function<void(const std::string&)> outputCallback_;
    std::function<std::string(const std::string&)> inputCallback_;

    struct ReturnValue {
        bool set = false;
        Value value;
    } returnValue_;

    uint64_t iterationCount_ = 0;
    std::chrono::steady_clock::time_point startTime_;
    Mulberry32RNG rng_;
    MemoryTracker memoryTracker_;
    std::vector<StackFrame> callStack_;

    // Visitor execution context
    std::shared_ptr<Environment> currentEnv_;
    Value evalResult_;

    void checkIterationLimit(int line = -1);
    void checkTimeout(int line = -1);
    void addOutput(const std::string& str);
    std::string requestInput(const std::string& prompt);
    Value convertInput(const std::string& inputStr, const Value& currentVal);

    void executeProcedure(const std::string& name, const std::vector<std::shared_ptr<Expr>>& args,
                          const std::shared_ptr<Environment>& callerEnv, int line = -1);
    Value executeFunction(const std::string& name, const std::vector<std::shared_ptr<Expr>>& args,
                          const std::shared_ptr<Environment>& callerEnv, int line = -1);
    Value evaluateCall(const std::string& name, const std::vector<std::shared_ptr<Expr>>& args,
                       const std::shared_ptr<Environment>& env, int line = -1);
};

} // namespace algo
