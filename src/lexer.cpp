#include "../include/lexer.hpp"
#include <unordered_set>

namespace algo {

static const std::unordered_set<std::string> KEYWORDS = {
    "si", "alors", "sinon", "sinonsi", "fin", "pour", "faire",
    "pas", "repeter", "jusqu'a", "tant", "que", "procedure",
    "fonction", "debut", "type", "var", "retourner", "et", "ou",
    "non", "vrai", "faux", "ecrire", "ecrire_nl", "lire", "lire_ligne",
    "algorithme", "tdo", "tdnt", "tdol", "tdog", "objet", "const", "constante", "constantes",
    "finsi", "fin_si", "finpour", "fin_pour", "fintantque", "tantque", "selon", "finselon", "fin_selon", "cas", "autrement",
    "allouer", "liberer", "nil", "null", "nul", "pointeur", "sur"
};

static const std::unordered_set<std::string> TYPES = {
    "entier", "reel", "caractere", "chaine", "booleen", "tableau", "matrice", "enregistrement", "fichier", "pointeur"
};

Lexer::Lexer(std::string source)
    : source_(std::move(source)), pos_(0), line_(1), col_(1) {}

char Lexer::peek(size_t offset) const {
    if (pos_ + offset >= source_.size()) return '\0';
    return source_[pos_ + offset];
}

char Lexer::advance() {
    if (pos_ >= source_.size()) return '\0';
    char c = source_[pos_++];
    if (c == '\n') {
        line_++;
        col_ = 1;
    } else {
        col_++;
    }
    return c;
}

bool Lexer::matchPrefix(const std::string& prefix) const {
    if (pos_ + prefix.size() > source_.size()) return false;
    return source_.compare(pos_, prefix.size(), prefix) == 0;
}

void Lexer::skipWhitespace() {
    while (pos_ < source_.size()) {
        char c = source_[pos_];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
        } else {
            break;
        }
    }
}

Token Lexer::readString() {
    int startLine = line_;
    int startCol = col_;
    char quoteChar = advance(); // skip opening " or '
    std::string str;
    while (pos_ < source_.size() && peek() != quoteChar) {
        if (peek() == '\\' && pos_ + 1 < source_.size()) {
            advance();
            char esc = advance();
            if (esc == 'n') str += '\n';
            else if (esc == 't') str += '\t';
            else if (esc == '\\') str += '\\';
            else if (esc == quoteChar) str += quoteChar;
            else { str += '\\'; str += esc; }
        } else {
            str += advance();
        }
    }
    if (pos_ >= source_.size()) {
        throw AlgoError("Chaîne non terminée", startLine);
    }
    advance(); // skip closing quote
    return Token(TokenType::STRING, str, startLine, startCol);
}

Token Lexer::readNumber() {
    int startLine = line_;
    int startCol = col_;
    std::string num;
    bool isReal = false;
    bool hasExponent = false;

    while (pos_ < source_.size()) {
        char c = peek();
        if (c >= '0' && c <= '9') {
            num += advance();
        } else if (c == '.') {
            if (isReal || hasExponent) break;
            // Lookahead: ensure following is a digit (and not double dot or method)
            char nextChar = peek(1);
            if (nextChar >= '0' && nextChar <= '9') {
                isReal = true;
                num += advance();
            } else {
                break;
            }
        } else if ((c == 'e' || c == 'E') && !hasExponent) {
            char next1 = peek(1);
            if (next1 >= '0' && next1 <= '9') {
                isReal = true;
                hasExponent = true;
                num += advance(); // consume 'e' or 'E'
                while (pos_ < source_.size() && peek() >= '0' && peek() <= '9') {
                    num += advance();
                }
            } else if ((next1 == '+' || next1 == '-') && peek(2) >= '0' && peek(2) <= '9') {
                isReal = true;
                hasExponent = true;
                num += advance(); // consume 'e' or 'E'
                num += advance(); // consume '+' or '-'
                while (pos_ < source_.size() && peek() >= '0' && peek() <= '9') {
                    num += advance();
                }
            } else {
                break;
            }
        } else {
            break;
        }
    }

    return Token(isReal ? TokenType::REAL : TokenType::NUMBER, num, startLine, startCol);
}

Token Lexer::readIdentifier() {
    int startLine = line_;
    int startCol = col_;
    std::string id;

    while (pos_ < source_.size()) {
        unsigned char c = (unsigned char)peek();
        if (isIdentPart(c)) {
            id += advance();
        } else {
            break;
        }
    }

    std::string norm = normalize(id);

    if (norm == "div" || norm == "mod" || norm == "dans") {
        return Token(TokenType::OPERATOR, norm == "dans" ? "∈" : norm, startLine, startCol);
    }
    if (KEYWORDS.find(norm) != KEYWORDS.end()) {
        return Token(TokenType::KEYWORD, norm, startLine, startCol);
    }
    if (TYPES.find(norm) != TYPES.end()) {
        return Token(TokenType::TYPE, norm, startLine, startCol);
    }

    return Token(TokenType::IDENTIFIER, id, startLine, startCol);
}

Token Lexer::readSingleLineComment() {
    int startLine = line_;
    int startCol = col_;
    advance(); // skip /
    advance(); // skip /
    std::string comment;
    while (pos_ < source_.size() && peek() != '\n') {
        comment += advance();
    }
    return Token(TokenType::COMMENT, comment, startLine, startCol);
}

Token Lexer::readMultiLineComment() {
    int startLine = line_;
    int startCol = col_;
    advance(); // skip /
    advance(); // skip *
    std::string comment;
    while (pos_ < source_.size()) {
        if (peek() == '*' && peek(1) == '/') {
            advance(); // skip *
            advance(); // skip /
            return Token(TokenType::COMMENT, comment, startLine, startCol);
        }
        comment += advance();
    }
    throw AlgoError("Commentaire multiligne non fermé", startLine);
}

Token Lexer::readOperatorOrDelimiter() {
    int startLine = line_;
    int startCol = col_;

    // Multi-byte UTF-8 arrows and operators:
    // ← is \xE2\x86\x90
    if (matchPrefix("\xE2\x86\x90")) {
        advance(); advance(); advance();
        return Token(TokenType::ARROW, "←", startLine, startCol);
    }
    // ≠ is \xE2\x89\xA0
    if (matchPrefix("\xE2\x89\xA0")) {
        advance(); advance(); advance();
        return Token(TokenType::OPERATOR, "≠", startLine, startCol);
    }
    // ≤ is \xE2\x89\xA4
    if (matchPrefix("\xE2\x89\xA4")) {
        advance(); advance(); advance();
        return Token(TokenType::OPERATOR, "≤", startLine, startCol);
    }
    // ≥ is \xE2\x89\xA5
    if (matchPrefix("\xE2\x89\xA5")) {
        advance(); advance(); advance();
        return Token(TokenType::OPERATOR, "≥", startLine, startCol);
    }
    // ∈ is \xE2\x88\x88
    if (matchPrefix("\xE2\x88\x88")) {
        advance(); advance(); advance();
        return Token(TokenType::OPERATOR, "∈", startLine, startCol);
    }

    // Two-character ASCII operators:
    if (peek() == '*' && peek(1) == '*') {
        advance(); advance();
        return Token(TokenType::OPERATOR, "**", startLine, startCol);
    }
    if (peek() == '<' && peek(1) == '-') {
        advance(); advance();
        return Token(TokenType::ARROW, "<-", startLine, startCol);
    }
    if (peek() == ':' && peek(1) == '=') {
        advance(); advance();
        return Token(TokenType::ARROW, ":=", startLine, startCol);
    }
    if (peek() == '<' && peek(1) == '=') {
        advance(); advance();
        return Token(TokenType::OPERATOR, "<=", startLine, startCol);
    }
    if (peek() == '>' && peek(1) == '=') {
        advance(); advance();
        return Token(TokenType::OPERATOR, ">=", startLine, startCol);
    }
    if (peek() == '!' && peek(1) == '=') {
        advance(); advance();
        return Token(TokenType::OPERATOR, "!=", startLine, startCol);
    }
    if (peek() == '<' && peek(1) == '>') {
        advance(); advance();
        return Token(TokenType::OPERATOR, "!=", startLine, startCol);
    }

    // Range delimiter ..
    if (peek() == '.' && peek(1) == '.') {
        advance(); advance();
        return Token(TokenType::DELIMITER, "..", startLine, startCol);
    }

    // Delimiters
    char c = peek();
    if (c == ':' || c == ',' || c == '@' || c == '.' || c == '(' || c == ')' ||
        c == '[' || c == ']' || c == ';' || c == '{' || c == '}') {
        advance();
        return Token(TokenType::DELIMITER, std::string(1, c), startLine, startCol);
    }

    // Single character operators
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '=' || c == '<' || c == '>' || c == '%') {
        advance();
        return Token(TokenType::OPERATOR, std::string(1, c), startLine, startCol);
    }

    // Other unexpected character
    advance();
    return Token(TokenType::OPERATOR, std::string(1, c), startLine, startCol);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (pos_ < source_.size()) {
        skipWhitespace();
        if (pos_ >= source_.size()) break;

        char c = peek();
        if (c == '"' || c == '\'') {
            tokens.push_back(readString());
        } else if (c >= '0' && c <= '9') {
            tokens.push_back(readNumber());
        } else if (c == '/' && peek(1) == '/') {
            tokens.push_back(readSingleLineComment());
        } else if (c == '/' && peek(1) == '*') {
            tokens.push_back(readMultiLineComment());
        } else if (isIdentStart((unsigned char)c)) {
            // Check if it's the UTF-8 arrow or math symbols first before identifier
            if (matchPrefix("\xE2\x86\x90") || matchPrefix("\xE2\x89\xA0") ||
                matchPrefix("\xE2\x89\xA4") || matchPrefix("\xE2\x89\xA5") ||
                matchPrefix("\xE2\x88\x88")) {
                tokens.push_back(readOperatorOrDelimiter());
            } else {
                tokens.push_back(readIdentifier());
            }
        } else {
            tokens.push_back(readOperatorOrDelimiter());
        }
    }
    tokens.emplace_back(TokenType::END_OF_FILE, "EOF", line_, col_);
    return tokens;
}

} // namespace algo