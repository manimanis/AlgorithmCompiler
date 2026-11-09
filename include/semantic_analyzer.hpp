#pragma once

#include "ast.hpp"
#include "common.hpp"
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <set>

namespace algo {

enum class SymbolKind {
    Variable,
    Constant,
    Procedure,
    Function,
    Type
};

struct Symbol {
    std::string name;
    std::string typeName;
    TypeSpec typeSpec;
    SymbolKind kind;
    int declLine = -1;
    bool isUsed = false;
};

struct SemanticDiagnostic {
    bool isError = true;
    int line = -1;
    int column = -1;
    std::string objectName;
    std::string message;
    std::string suggestion;
    std::string formatted;
};

class SemanticAnalyzer {
public:
    explicit SemanticAnalyzer(const std::string& sourceCode);

    std::vector<SemanticDiagnostic> analyze(const Program& prog);

private:
    std::string sourceCode_;
    std::vector<std::string> sourceLines_;
    std::vector<std::map<std::string, Symbol>> scopes_;
    std::map<std::string, TypeDecl> typeMap_;
    std::map<std::string, ProcedureDecl> procMap_;
    std::map<std::string, FunctionDecl> funcMap_;
    std::vector<SemanticDiagnostic> diagnostics_;

    std::string currentFunctionName_;
    std::string currentFunctionReturnType_;
    bool hasReturnInCurrentFunction_ = false;

    std::set<std::string> assignedVars_;
    std::set<std::string> globallyAssignedVars_;
    std::set<std::string> warnedUnassignedVars_;
    std::set<std::string> constNames_;
    std::map<std::string, std::string> enumValueToType_;

    void splitSourceLines();
    void pushScope();
    void popScope();
    bool declareSymbol(const std::string& name, const std::string& typeName, const TypeSpec& ts, SymbolKind kind, int line);
    const Symbol* findSymbol(const std::string& name, bool markUsed = true);
    std::vector<std::string> getVisibleSymbolNames() const;

    std::string findClosestMatch(const std::string& target, const std::vector<std::string>& candidates, int maxDist = 3);
    std::string formatDiagnostic(int line, int col, const std::string& symbol, const std::string& msg, const std::string& suggestion);
    void reportError(int line, int col, const std::string& objectName, const std::string& msg, const std::string& suggestion = "");
    void reportWarning(int line, int col, const std::string& objectName, const std::string& msg);

    void collectTypes(const Program& prog);
    void collectGlobalDeclarations(const Program& prog);
    void checkProcedure(const ProcedureDecl& proc);
    void checkFunction(const FunctionDecl& func);
    void checkBlock(const std::vector<std::shared_ptr<Stmt>>& stmts);
    void checkStmt(const std::shared_ptr<Stmt>& stmt);
    std::string checkExpr(const std::shared_ptr<Expr>& expr);

    struct ExprTypeInfo {
        std::string typeName = "inconnu";
        TypeSpec typeSpec;
    };
    ExprTypeInfo resolveExprTypeInfo(const std::shared_ptr<Expr>& expr);

    bool areTypesCompatible(const std::string& targetType, const std::string& srcType);
    bool isNumericType(const std::string& type);
    bool isBooleanType(const std::string& type);
    bool isStringType(const std::string& type);
    bool isScalarType(const std::string& type);
    bool isCharType(const std::string& type);
    bool isLValue(const std::shared_ptr<Expr>& expr);
};

} // namespace algo
