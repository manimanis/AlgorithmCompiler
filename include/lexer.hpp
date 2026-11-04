#pragma once

#include "token.hpp"

namespace algo {

class Lexer {
public:
    explicit Lexer(std::string source);
    std::vector<Token> tokenize();

private:
    std::string source_;
    size_t pos_;
    int line_;
    int col_;

    char peek(size_t offset = 0) const;
    char advance();
    void skipWhitespace();
    Token readString();
    Token readNumber();
    Token readIdentifier();
    Token readSingleLineComment();
    Token readMultiLineComment();
    Token readOperatorOrDelimiter();

    bool matchPrefix(const std::string& prefix) const;
};

} // namespace algo
