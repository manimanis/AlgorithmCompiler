#pragma once

#include "ast.hpp"
#include <map>
#include <set>
#include <string>
#include <vector>
#include <sstream>

namespace algo {

class PythonTranspiler {
public:
    explicit PythonTranspiler(const Program& prog);
    std::string transpile();

private:
    const Program& prog_;
    int indentLevel_ = 0;
    int repeatCounter_ = 0;

    std::map<std::string, TypeDecl> typeMap_;
    std::map<std::string, std::string> varTypeMap_;
    std::map<std::string, std::string> currentLocalTypes_;
    std::map<std::string, ProcedureDecl> procMap_;
    std::map<std::string, FunctionDecl> funcMap_;

    bool needArray_ = false;
    std::set<std::string> mathImports_;
    std::set<std::string> randomImports_;
    std::set<std::string> pickleImports_;
    std::set<std::string> osImports_;

    std::string indent() const { return std::string(indentLevel_ * 4, ' '); }

    void collectTypes();
    void collectNeeds();
    void scanTypeSpec(const TypeSpec& ts);
    void scanExpr(const std::shared_ptr<Expr>& expr);
    void scanStmt(const std::shared_ptr<Stmt>& stmt);

    void transpileImports(std::ostringstream& ss);
    void transpileProceduresAndFunctions(std::ostringstream& ss);
    void transpileProcedure(const ProcedureDecl& proc, std::ostringstream& ss);
    void transpileFunction(const FunctionDecl& func, std::ostringstream& ss);
    void transpileMain(std::ostringstream& ss);

    void transpileBlock(const std::vector<std::shared_ptr<Stmt>>& stmts, std::ostringstream& ss);
    void transpileStmt(const std::shared_ptr<Stmt>& stmt, std::ostringstream& ss);
    std::string transpileExpr(const std::shared_ptr<Expr>& expr, int parentPrec = 0, bool isRight = false);

    bool isWriteToFile(const std::shared_ptr<WriteStmt>& w);
    bool isReadFromFile(const std::shared_ptr<ReadStmt>& rd);
    std::string buildPromptString(const std::vector<std::shared_ptr<Expr>>& args);
    void transpilePromptRead(const std::shared_ptr<WriteStmt>& w, const std::shared_ptr<ReadStmt>& rd, std::ostringstream& ss);

    std::string resolveBaseType(const std::string& typeName);
    int getExprPrecedence(const std::shared_ptr<Expr>& expr);
    std::string formatRangeEnd(const std::shared_ptr<Expr>& endExpr, int delta);

    std::string getInitValuePy(const TypeSpec& ts, const std::string& typeName = "");
    std::string getArrayInitPy(int size, const std::string& elemType);
    std::string getMatrixInitPy(int rows, int cols, const std::string& elemType);
    std::string transpileReadInput(const std::string& typeName, const std::string& prompt = "");
    std::string getExprType(const std::shared_ptr<Expr>& expr);
};

} // namespace algo
