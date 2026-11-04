#pragma once

#include "lexer.hpp"
#include "ast.hpp"

namespace algo {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    Program parse();

private:
    std::vector<Token> tokens_;
    size_t pos_;

    const Token& peek(size_t offset = 0) const;
    const Token& next();
    bool match(TokenType type, const std::string& val = "");
    bool matchKeyword(const std::string& kw);
    const Token& expect(TokenType type, const std::string& val = "");
    const Token& expectKeyword(const std::string& kw);
    void error(const std::string& msg);

    void consumeSemicolons();

    // Top-level
    void parseTopLevel(Program& prog);
    TypeDecl parseTypeDeclaration();
    TypeSpec parseTypeSpec();
    std::vector<VarDecl> parseVarDeclarations(bool hasVarKeyword = true);
    ProcedureDecl parseProcedureDeclaration();
    FunctionDecl parseFunctionDeclaration();
    std::vector<ParamDecl> parseParamList();
    std::vector<std::shared_ptr<Stmt>> parseBlock(const std::string& endKeyword = "fin");

    // TDO, TDNT, TDOL
    void parseTDNT(Program& prog);
    void parseTDO(Program& prog);
    void parseTDOL(std::vector<VarDecl>& localVars);

    // Statements
    std::shared_ptr<Stmt> parseStatement();
    std::shared_ptr<Stmt> parseAssignOrCall();
    std::shared_ptr<Stmt> parseWrite();
    std::shared_ptr<Stmt> parseRead();
    std::shared_ptr<Stmt> parseIf();
    std::shared_ptr<Stmt> parseFor();
    std::shared_ptr<Stmt> parseWhile();
    std::shared_ptr<Stmt> parseRepeat();
    std::shared_ptr<Stmt> parseReturn();
    std::shared_ptr<Stmt> parseSwitch();
    bool isCaseHeader();

    // Expressions
    std::shared_ptr<Expr> parseExpression();
    std::shared_ptr<Expr> parseOr();
    std::shared_ptr<Expr> parseAnd();
    std::shared_ptr<Expr> parseComparison();
    std::shared_ptr<Expr> parseAddition();
    std::shared_ptr<Expr> parseTerm();
    std::shared_ptr<Expr> parseFactor();
    std::vector<std::shared_ptr<Expr>> parseArgs();
};

} // namespace algo
