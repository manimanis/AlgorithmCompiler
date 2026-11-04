#pragma once

#include "common.hpp"

namespace algo {

enum class TokenType {
    KEYWORD,
    TYPE,
    IDENTIFIER,
    NUMBER,
    REAL,
    STRING,
    OPERATOR,
    ARROW,
    DELIMITER,
    COMMENT,
    END_OF_FILE
};

inline const char* tokenTypeToString(TokenType t) {
    switch (t) {
        case TokenType::KEYWORD: return "KEYWORD";
        case TokenType::TYPE: return "TYPE";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::REAL: return "REAL";
        case TokenType::STRING: return "STRING";
        case TokenType::OPERATOR: return "OPERATOR";
        case TokenType::ARROW: return "ARROW";
        case TokenType::DELIMITER: return "DELIMITER";
        case TokenType::COMMENT: return "COMMENT";
        case TokenType::END_OF_FILE: return "EOF";
    }
    return "UNKNOWN";
}

struct Token {
    TokenType type;
    std::string value;
    int line;
    int col;

    Token(TokenType type, const std::string& value, int line, int col)
        : type(type), value(value), line(line), col(col) {}
};

} // namespace algo
