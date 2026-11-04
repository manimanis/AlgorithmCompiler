#include "../include/parser.hpp"

namespace algo {

Parser::Parser(std::vector<Token> tokens, std::shared_ptr<Arena> arena)
    : tokens_(std::move(tokens)), pos_(0), arena_(std::move(arena)) {
    if (!arena_) {
        arena_ = makeNode<Arena>();
    }
}

const Token& Parser::peek(size_t offset) const {
    if (pos_ + offset >= tokens_.size()) return tokens_.back();
    return tokens_[pos_ + offset];
}

const Token& Parser::next() {
    if (pos_ >= tokens_.size()) return tokens_.back();
    return tokens_[pos_++];
}

bool Parser::match(TokenType type, const std::string& val) {
    if (peek().type != type) return false;
    if (!val.empty() && peek().value != val) return false;
    next();
    return true;
}

bool Parser::matchKeyword(const std::string& kw) {
    if (peek().type != TokenType::KEYWORD && peek().type != TokenType::IDENTIFIER) return false;
    if (normalize(peek().value) != normalize(kw)) return false;
    next();
    return true;
}

const Token& Parser::expect(TokenType type, const std::string& val) {
    const Token& tok = peek();
    bool ok = (tok.type == type);
    if (ok && !val.empty()) {
        if (type == TokenType::KEYWORD) {
            ok = (normalize(tok.value) == normalize(val));
        } else {
            ok = (tok.value == val);
        }
    }
    if (!ok) {
        throw AlgoError("Attendu '" + (val.empty() ? tokenTypeToString(type) : val) + "', trouvé '" + tok.value + "'", tok.line);
    }
    return next();
}

const Token& Parser::expectKeyword(const std::string& kw) {
    const Token& tok = peek();
    bool ok = (tok.type == TokenType::KEYWORD || tok.type == TokenType::IDENTIFIER) &&
              (normalize(tok.value) == normalize(kw));
    if (!ok) {
        throw AlgoError("Attendu mot-clé '" + kw + "', trouvé '" + tok.value + "'", tok.line);
    }
    return next();
}

void Parser::error(const std::string& msg) {
    throw AlgoError(msg + " près de '" + peek().value + "'", peek().line);
}

void Parser::consumeSemicolons() {
    while (peek().type == TokenType::DELIMITER && peek().value == ";") {
        next();
    }
}

Program Parser::parse() {
    Program prog;
    prog.arena = arena_;
    while (peek().type != TokenType::END_OF_FILE) {
        if (peek().type == TokenType::COMMENT) {
            next();
            continue;
        }

        std::string norm = normalize(peek().value);

        // Algorithme <name>
        if (norm == "algorithme") {
            int algoLine = peek().line;
            next();
            while (peek().type != TokenType::END_OF_FILE && peek().line == algoLine) {
                prog.name += (prog.name.empty() ? "" : " ") + next().value;
            }
            continue;
        }

        // TDNT (Tableau de Déclaration des Nouveaux Types)
        if (norm == "tdnt") {
            parseTDNT(prog);
            continue;
        }

        // TDO / TDOG (Tableau des Objets Globaux)
        if (norm == "tdo" || norm == "tdog") {
            parseTDO(prog);
            continue;
        }

        // Top-level Type declaration
        if (norm == "type") {
            prog.typeDecls.push_back(parseTypeDeclaration());
            continue;
        }

        // Top-level Var declaration
        if (norm == "var") {
            auto decls = parseVarDeclarations(true);
            for (auto& d : decls) prog.varDecls.push_back(std::move(d));
            continue;
        }

        // Variable declaration without 'Var' keyword before Début
        if (peek().type == TokenType::IDENTIFIER &&
            (peek(1).value == ":" || peek(1).value == ",")) {
            auto decls = parseVarDeclarations(false);
            for (auto& d : decls) prog.varDecls.push_back(std::move(d));
            continue;
        }

        // Procedure
        if (norm == "procedure") {
            prog.procedures.push_back(parseProcedureDeclaration());
            continue;
        }

        // Function
        if (norm == "fonction") {
            prog.functions.push_back(parseFunctionDeclaration());
            continue;
        }

        // Main program: Début
        if (norm == "debut") {
            next(); // consume Début
            prog.mainBody = parseBlock("fin");
            continue;
        }

        // Skip any stray delimiter
        if (peek().type == TokenType::DELIMITER && peek().value == ";") {
            next();
            continue;
        }

        error("Instruction ou déclaration inattendue");
    }
    return prog;
}

TypeSpec Parser::parseTypeSpec() {
    TypeSpec ts;
    std::string n = normalize(peek().value);

    if (peek().type == TokenType::KEYWORD && (n == "fonction" || n == "procedure")) {
        next();
        ts.kind = TypeKind::Simple;
        ts.name = n;
        return ts;
    }

    if (n == "tableau") {
        next(); // consume tableau

        if (match(TokenType::DELIMITER, "[")) {
            Token low1 = expect(TokenType::NUMBER);
            expect(TokenType::DELIMITER, "..");
            Token high1 = expect(TokenType::NUMBER);
            int dim1 = std::stoi(high1.value) - std::stoi(low1.value) + 1;

            if (match(TokenType::DELIMITER, ",")) {
                Token low2 = expect(TokenType::NUMBER);
                expect(TokenType::DELIMITER, "..");
                Token high2 = expect(TokenType::NUMBER);
                int dim2 = std::stoi(high2.value) - std::stoi(low2.value) + 1;
                expect(TokenType::DELIMITER, "]");
                expectKeyword("de");
                ts.kind = TypeKind::Array2D;
                ts.rows = dim1;
                ts.cols = dim2;
                if (peek().type == TokenType::TYPE || peek().type == TokenType::IDENTIFIER) {
                    ts.elemType = next().value;
                } else {
                    error("Attendu un type élément pour la matrice");
                }
                return ts;
            } else {
                expect(TokenType::DELIMITER, "]");
                expectKeyword("de");
                ts.kind = TypeKind::Array1D;
                ts.rows = dim1;
                if (peek().type == TokenType::TYPE || peek().type == TokenType::IDENTIFIER) {
                    ts.elemType = next().value;
                } else {
                    error("Attendu un type élément pour le tableau");
                }
                return ts;
            }
        }

        expectKeyword("de");
        Token nTok = expect(TokenType::NUMBER);
        int dim1 = std::stoi(nTok.value);

        std::string nextNorm = normalize(peek().value);
        if (nextNorm == "lignes" || nextNorm == "ligne") {
            next(); // consume lignes
            if (peek().value == "*" || normalize(peek().value) == "x") next();
            Token mTok = expect(TokenType::NUMBER);
            int dim2 = std::stoi(mTok.value);
            if (normalize(peek().value) == "colonnes" || normalize(peek().value) == "colonne") {
                next(); // consume colonnes
            }
            if (normalize(peek().value) == "de") next();
            ts.kind = TypeKind::Array2D;
            ts.rows = dim1;
            ts.cols = dim2;
            if (peek().type == TokenType::TYPE || peek().type == TokenType::IDENTIFIER) {
                ts.elemType = next().value;
            } else {
                error("Attendu un type élément pour la matrice");
            }
        } else if (peek().value == "*" || peek().value == ",") {
            next(); // consume * or ,
            Token mTok = expect(TokenType::NUMBER);
            int dim2 = std::stoi(mTok.value);
            if (normalize(peek().value) == "colonnes" || normalize(peek().value) == "colonne") {
                next();
            }
            if (normalize(peek().value) == "de") next();
            ts.kind = TypeKind::Array2D;
            ts.rows = dim1;
            ts.cols = dim2;
            if (peek().type == TokenType::TYPE || peek().type == TokenType::IDENTIFIER) {
                ts.elemType = next().value;
            } else {
                error("Attendu un type élément pour la matrice");
            }
        } else {
            // 1D Array
            if (normalize(peek().value) == "de") next();
            ts.kind = TypeKind::Array1D;
            ts.rows = dim1;
            if (peek().type == TokenType::TYPE || peek().type == TokenType::IDENTIFIER) {
                ts.elemType = next().value;
            } else {
                error("Attendu un type élément pour le tableau");
            }
        }
        return ts;
    }

    if (n == "matrice") {
        next(); // consume matrice
        if (normalize(peek().value) == "de") next();
        Token nTok = expect(TokenType::NUMBER);
        int dim1 = std::stoi(nTok.value);
        if (peek().value == "*" || peek().value == ",") next();
        Token mTok = expect(TokenType::NUMBER);
        int dim2 = std::stoi(mTok.value);
        if (normalize(peek().value) == "colonnes" || normalize(peek().value) == "colonne") next();
        if (normalize(peek().value) == "de") next();
        ts.kind = TypeKind::Array2D;
        ts.rows = dim1;
        ts.cols = dim2;
        if (peek().type == TokenType::TYPE || peek().type == TokenType::IDENTIFIER) {
            ts.elemType = next().value;
        } else {
            error("Attendu un type élément pour la matrice");
        }
        return ts;
    }

    if (n == "enregistrement") {
        next(); // consume enregistrement
        ts.kind = TypeKind::Record;
        while (peek().type != TokenType::END_OF_FILE) {
            consumeSemicolons();
            if (peek().type == TokenType::COMMENT) { next(); continue; }
            if (normalize(peek().value) == "fin") {
                Token finTok = next(); // consume fin
                if (peek().type == TokenType::IDENTIFIER && peek().line == finTok.line) {
                    std::string idNorm = normalize(peek().value);
                    if (idNorm != "si" && idNorm != "pour" && idNorm != "tant" && idNorm != "selon") {
                        next();
                    }
                }
                break;
            }
            if (peek().type == TokenType::IDENTIFIER) {
                auto fieldDecl = makeNode<VarDecl>();
                Token firstFld = expect(TokenType::IDENTIFIER);
                fieldDecl->names.push_back(firstFld.value);
                fieldDecl->line = firstFld.line;
                while (match(TokenType::DELIMITER, ",")) {
                    fieldDecl->names.push_back(expect(TokenType::IDENTIFIER).value);
                }
                expect(TokenType::DELIMITER, ":");
                fieldDecl->typeSpec = parseTypeSpec();
                fieldDecl->varType = fieldDecl->typeSpec.elemType.empty() ? fieldDecl->typeSpec.name : fieldDecl->typeSpec.elemType;
                ts.fields.push_back(fieldDecl);
                consumeSemicolons();
            } else {
                break;
            }
        }
        return ts;
    }

    if (n == "fichier") {
        next(); // consume fichier
        if (normalize(peek().value) == "texte") {
            next(); // consume texte
            ts.kind = TypeKind::FileText;
            ts.name = "Fichier Texte";
            return ts;
        }
        if (normalize(peek().value) == "de") next();
        ts.kind = TypeKind::FileData;
        if (peek().type == TokenType::TYPE || peek().type == TokenType::IDENTIFIER) {
            ts.elemType = next().value;
        } else {
            error("Attendu un type élément pour le fichier");
        }
        return ts;
    }

    // Simple type
    if (peek().type == TokenType::TYPE || peek().type == TokenType::IDENTIFIER) {
        ts.kind = TypeKind::Simple;
        ts.name = next().value;
        return ts;
    }

    error("Attendu une spécification de type valide");
    return ts;
}

TypeDecl Parser::parseTypeDeclaration() {
    expectKeyword("type");
    Token nameTok = expect(TokenType::IDENTIFIER);
    expect(TokenType::OPERATOR, "=");
    TypeSpec ts = parseTypeSpec();

    TypeDecl td;
    td.name = nameTok.value;
    td.typeSpec = ts;
    td.arraySize = ts.rows;
    td.elemType = ts.elemType.empty() ? ts.name : ts.elemType;
    td.line = nameTok.line;
    return td;
}

std::vector<VarDecl> Parser::parseVarDeclarations(bool hasVarKeyword) {
    if (hasVarKeyword) {
        expectKeyword("var");
    }

    std::vector<VarDecl> decls;

    auto parseOneVarLine = [this]() -> VarDecl {
        VarDecl d;
        Token firstId = expect(TokenType::IDENTIFIER);
        d.names.push_back(firstId.value);
        d.line = firstId.line;

        while (match(TokenType::DELIMITER, ",")) {
            d.names.push_back(expect(TokenType::IDENTIFIER).value);
        }

        expect(TokenType::DELIMITER, ":");
        d.typeSpec = parseTypeSpec();
        d.varType = d.typeSpec.elemType.empty() ? d.typeSpec.name : d.typeSpec.elemType;
        return d;
    };

    decls.push_back(parseOneVarLine());
    consumeSemicolons();

    // Continuation lines: identifier followed by ',' or ':'
    while (peek().type == TokenType::IDENTIFIER &&
           (peek(1).value == ":" || peek(1).value == ",")) {
        // Make sure it's not a statement inside Début or another section
        std::string n = normalize(peek().value);
        if (n == "debut" || n == "fin" || n == "procedure" || n == "fonction" ||
            n == "type" || n == "var" || n == "tdo" || n == "tdnt" || n == "tdol") {
            break;
        }
        decls.push_back(parseOneVarLine());
        consumeSemicolons();
    }

    return decls;
}

ProcedureDecl Parser::parseProcedureDeclaration() {
    expectKeyword("procedure");
    Token nameTok = expect(TokenType::IDENTIFIER);
    auto params = parseParamList();

    ProcedureDecl proc;
    proc.name = nameTok.value;
    proc.params = std::move(params);
    proc.line = nameTok.line;

    // Optional local vars before Début
    if (normalize(peek().value) == "var") {
        proc.localVars = parseVarDeclarations(true);
    } else if (peek().type == TokenType::IDENTIFIER &&
               (peek(1).value == ":" || peek(1).value == ",")) {
        std::string n = normalize(peek().value);
        if (n != "debut") {
            proc.localVars = parseVarDeclarations(false);
        }
    }

    expectKeyword("debut");
    proc.body = parseBlock("fin");

    // Check if immediately followed by TDOL
    if (normalize(peek().value) == "tdol") {
        parseTDOL(proc.localVars);
    }

    return proc;
}

FunctionDecl Parser::parseFunctionDeclaration() {
    expectKeyword("fonction");
    Token nameTok = expect(TokenType::IDENTIFIER);
    auto params = parseParamList();
    expect(TokenType::DELIMITER, ":");

    TypeSpec ts = parseTypeSpec();
    std::string retType = ts.kind == TypeKind::FileText ? "Fichier Texte" : (ts.elemType.empty() ? ts.name : ts.elemType);

    FunctionDecl func;
    func.name = nameTok.value;
    func.params = std::move(params);
    func.returnType = retType;
    func.line = nameTok.line;

    // Optional local vars before Début
    if (normalize(peek().value) == "var") {
        func.localVars = parseVarDeclarations(true);
    } else if (peek().type == TokenType::IDENTIFIER &&
               (peek(1).value == ":" || peek(1).value == ",")) {
        std::string n = normalize(peek().value);
        if (n != "debut") {
            func.localVars = parseVarDeclarations(false);
        }
    }

    expectKeyword("debut");
    func.body = parseBlock("fin");

    // Check if immediately followed by TDOL
    if (normalize(peek().value) == "tdol") {
        parseTDOL(func.localVars);
    }

    return func;
}

std::vector<ParamDecl> Parser::parseParamList() {
    std::vector<ParamDecl> params;
    if (!match(TokenType::DELIMITER, "(")) return params;

    if (peek().type == TokenType::DELIMITER && peek().value == ")") {
        next();
        return params;
    }

    auto parseParamGroup = [this, &params]() {
        std::vector<std::pair<std::string, bool>> names;
        do {
            bool byRef = false;
            if (match(TokenType::DELIMITER, "@")) {
                byRef = true;
            }
            Token id = expect(TokenType::IDENTIFIER);
            names.push_back({id.value, byRef});
        } while (match(TokenType::DELIMITER, ","));

        expect(TokenType::DELIMITER, ":");
        TypeSpec ts = parseTypeSpec();
        std::string pType = ts.kind == TypeKind::FileText ? "Fichier Texte" : (ts.elemType.empty() ? ts.name : ts.elemType);

        for (const auto& item : names) {
            ParamDecl p;
            p.name = item.first;
            p.paramType = pType;
            p.byRef = item.second;
            params.push_back(p);
        }
    };

    parseParamGroup();
    while (match(TokenType::DELIMITER, ";") || match(TokenType::DELIMITER, ",")) {
        if (peek().type == TokenType::DELIMITER && peek().value == ")") break;
        parseParamGroup();
    }

    expect(TokenType::DELIMITER, ")");
    return params;
}

void Parser::parseTDNT(Program& prog) {
    expectKeyword("tdnt");
    while (peek().type != TokenType::END_OF_FILE) {
        if (peek().type == TokenType::COMMENT) { next(); continue; }
        std::string n = normalize(peek().value);
        if (n == "tdo" || n == "tdog" || n == "tdol" || n == "debut" || n == "fin" ||
            n == "procedure" || n == "fonction" || n == "algorithme") {
            break;
        }

        if (n == "type") next();
        Token nameTok = expect(TokenType::IDENTIFIER);
        expect(TokenType::OPERATOR, "=");
        TypeSpec ts = parseTypeSpec();

        TypeDecl td;
        td.name = nameTok.value;
        td.typeSpec = ts;
        td.arraySize = ts.rows;
        td.elemType = ts.elemType.empty() ? ts.name : ts.elemType;
        td.line = nameTok.line;
        prog.typeDecls.push_back(td);

        consumeSemicolons();
    }
}

void Parser::parseTDO(Program& prog) {
    if (normalize(peek().value) == "tdog") expectKeyword("tdog");
    else expectKeyword("tdo");

    // Skip empty lines / comments and optional header line: 'Objet : Type/Nature'
    while (peek().type == TokenType::COMMENT) next();
    if (normalize(peek().value) == "objet") {
        int objLine = peek().line;
        while (peek().type != TokenType::END_OF_FILE && peek().line == objLine) {
            next();
        }
    }

    while (peek().type != TokenType::END_OF_FILE) {
        if (peek().type == TokenType::COMMENT) { next(); continue; }
        std::string n = normalize(peek().value);
        if (n == "tdo" || n == "tdog" || n == "tdol" || n == "tdnt" || n == "debut" || n == "fin" ||
            n == "procedure" || n == "fonction" || n == "algorithme") {
            break;
        }

        if (peek().type == TokenType::IDENTIFIER && (peek(1).value == ":" || peek(1).value == ",")) {
            auto decls = parseVarDeclarations(false);
            for (auto& d : decls) prog.varDecls.push_back(std::move(d));
            consumeSemicolons();
        } else {
            break;
        }
    }
}

void Parser::parseTDOL(std::vector<VarDecl>& localVars) {
    expectKeyword("tdol");
    while (peek().type == TokenType::COMMENT) next();
    if (normalize(peek().value) == "objet") {
        int objLine = peek().line;
        while (peek().type != TokenType::END_OF_FILE && peek().line == objLine) {
            next();
        }
    }

    while (peek().type != TokenType::END_OF_FILE) {
        if (peek().type == TokenType::COMMENT) { next(); continue; }
        std::string n = normalize(peek().value);
        if (n == "tdo" || n == "tdog" || n == "tdol" || n == "tdnt" || n == "debut" || n == "fin" ||
            n == "procedure" || n == "fonction" || n == "algorithme") {
            break;
        }

        if (peek().type == TokenType::IDENTIFIER && (peek(1).value == ":" || peek(1).value == ",")) {
            auto decls = parseVarDeclarations(false);
            for (auto& d : decls) localVars.push_back(std::move(d));
            consumeSemicolons();
        } else {
            break;
        }
    }
}

std::vector<std::shared_ptr<Stmt>> Parser::parseBlock(const std::string& endKeyword) {
    std::vector<std::shared_ptr<Stmt>> stmts;
    while (peek().type != TokenType::END_OF_FILE) {
        if (peek().type == TokenType::COMMENT) {
            next();
            continue;
        }

        consumeSemicolons();
        if (peek().type == TokenType::END_OF_FILE) break;

        std::string n = normalize(peek().value);
        if (endKeyword == "fin" && (n == "fin" || n == "jusqu'a")) {
            if (n == "fin") {
                Token finTok = next(); // consume 'fin'
                // Optional trailing name: "Fin <nom>"
                if (peek().type == TokenType::IDENTIFIER && peek().line == finTok.line) {
                    std::string idNorm = normalize(peek().value);
                    if (idNorm != "si" && idNorm != "pour" && idNorm != "tant" && idNorm != "que" && idNorm != "selon") {
                        next();
                    }
                }
            }
            break;
        }
        if (n == "sinon" || n == "jusqu'a") {
            break;
        }

        stmts.push_back(parseStatement());
        consumeSemicolons();
    }
    return stmts;
}

std::shared_ptr<Stmt> Parser::parseStatement() {
    std::string n = normalize(peek().value);

    if (n == "ecrire" || n == "ecrire_nl") return parseWrite();
    if (n == "lire") return parseRead();
    if (n == "lire_ligne") {
        Token tok = next();
        expect(TokenType::DELIMITER, "(");
        Token fTok = expect(TokenType::IDENTIFIER);
        expect(TokenType::DELIMITER, ",");
        Token varTok = expect(TokenType::IDENTIFIER);
        expect(TokenType::DELIMITER, ")");
        return makeNode<CallStmt>("lire_ligne",
            std::vector<std::shared_ptr<Expr>>{
                makeNode<VariableExpr>(fTok.value, tok.line),
                makeNode<VariableExpr>(varTok.value, tok.line)
            }, tok.line);
    }
    if (n == "si") return parseIf();
    if (n == "pour") return parseFor();
    if (n == "tant" || n == "tantque") return parseWhile();
    if (n == "repeter") return parseRepeat();
    if (n == "retourner") return parseReturn();
    if (n == "selon") return parseSwitch();

    return parseAssignOrCall();
}

std::shared_ptr<Stmt> Parser::parseAssignOrCall() {
    Token idTok = expect(TokenType::IDENTIFIER);
    int line = idTok.line;

    // Procedure call: id(args) or id()
    if (match(TokenType::DELIMITER, "(")) {
        std::vector<std::shared_ptr<Expr>> args;
        if (!match(TokenType::DELIMITER, ")")) {
            do {
                args.push_back(parseExpression());
            } while (match(TokenType::DELIMITER, ","));
            expect(TokenType::DELIMITER, ")");
        }
        return makeNode<CallStmt>(idTok.value, std::move(args), line);
    }

    std::shared_ptr<Expr> lhs = makeNode<VariableExpr>(idTok.value, line);
    while (true) {
        if (match(TokenType::DELIMITER, ".")) {
            Token mTok = expect(TokenType::IDENTIFIER);
            lhs = makeNode<MemberAccessExpr>(lhs, mTok.value, mTok.line);
        } else if (match(TokenType::DELIMITER, "[")) {
            auto idx1 = parseExpression();
            std::shared_ptr<Expr> idx2 = nullptr;
            if (match(TokenType::DELIMITER, ",")) {
                idx2 = parseExpression();
            }
            expect(TokenType::DELIMITER, "]");
            if (idx2 == nullptr && match(TokenType::DELIMITER, "[")) {
                idx2 = parseExpression();
                expect(TokenType::DELIMITER, "]");
            }
            lhs = makeNode<ArrayAccessExpr>(lhs, idx1, idx2, line);
        } else {
            break;
        }
    }

    if (peek().type == TokenType::ARROW) {
        next(); // consume arrow
        auto val = parseExpression();
        if (auto mem = std::dynamic_pointer_cast<MemberAccessExpr>(lhs)) {
            return makeNode<MemberAssignStmt>(mem->object, mem->member, val, line);
        }
        if (auto arr = std::dynamic_pointer_cast<ArrayAccessExpr>(lhs)) {
            if (arr->target) {
                if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(arr->target)) {
                    return makeNode<ArrayAssignStmt>(varExpr->name, arr->index, val, arr->index2, line);
                }
                return makeNode<ArrayAssignStmt>(arr->target, arr->index, val, arr->index2, line);
            }
            return makeNode<ArrayAssignStmt>(arr->name, arr->index, val, arr->index2, line);
        }
        if (auto var = std::dynamic_pointer_cast<VariableExpr>(lhs)) {
            return makeNode<AssignStmt>(var->name, val, line);
        }
    }

    error("Instruction inattendue après identifiant");
    return nullptr;
}

std::shared_ptr<Stmt> Parser::parseWrite() {
    Token wTok = next(); // consume ecrire or ecrire_nl
    int line = wTok.line;
    bool isNl = (normalize(wTok.value) == "ecrire_nl");
    expect(TokenType::DELIMITER, "(");

    std::vector<std::shared_ptr<Expr>> args;
    std::string sep = " ";
    std::string fin = "\n";

    if (!match(TokenType::DELIMITER, ")")) {
        while (true) {
            // Check for sep = "..." or fin = "..."
            if ((peek().type == TokenType::IDENTIFIER || peek().type == TokenType::KEYWORD) && peek(1).value == "=") {
                std::string paramName = normalize(peek().value);
                if (paramName == "sep" || paramName == "fin" || paramName == "end") {
                    next(); // param name
                    next(); // =
                    Token valTok = expect(TokenType::STRING);
                    if (paramName == "sep") sep = valTok.value;
                    else fin = valTok.value;

                    if (match(TokenType::DELIMITER, ",")) continue;
                    break;
                }
            }

            args.push_back(parseExpression());
            if (!match(TokenType::DELIMITER, ",")) break;
        }
        expect(TokenType::DELIMITER, ")");
    }

    return makeNode<WriteStmt>(std::move(args), sep, fin, line, isNl);
}

std::shared_ptr<Stmt> Parser::parseRead() {
    Token rTok = next(); // consume lire
    int line = rTok.line;
    expect(TokenType::DELIMITER, "(");

    std::vector<ReadTarget> targets;
    do {
        Token idTok = expect(TokenType::IDENTIFIER);
        ReadTarget tgt;
        tgt.name = idTok.value;

        std::shared_ptr<Expr> expr = makeNode<VariableExpr>(idTok.value, idTok.line);
        while (true) {
            if (match(TokenType::DELIMITER, ".")) {
                Token mTok = expect(TokenType::IDENTIFIER);
                tgt.member = mTok.value;
                expr = makeNode<MemberAccessExpr>(expr, mTok.value, mTok.line);
            } else if (match(TokenType::DELIMITER, "[")) {
                tgt.isArray = true;
                tgt.index = parseExpression();
                if (match(TokenType::DELIMITER, ",")) {
                    tgt.is2D = true;
                    tgt.index2 = parseExpression();
                }
                expect(TokenType::DELIMITER, "]");
                if (tgt.index2 == nullptr && match(TokenType::DELIMITER, "[")) {
                    tgt.is2D = true;
                    tgt.index2 = parseExpression();
                    expect(TokenType::DELIMITER, "]");
                }
                expr = makeNode<ArrayAccessExpr>(expr, tgt.index, tgt.index2, idTok.line);
            } else {
                break;
            }
        }
        tgt.targetExpr = expr;
        targets.push_back(tgt);
    } while (match(TokenType::DELIMITER, ","));

    expect(TokenType::DELIMITER, ")");
    return makeNode<ReadStmt>(std::move(targets), line);
}

std::shared_ptr<Stmt> Parser::parseIf() {
    Token ifTok = next(); // consume si
    int line = ifTok.line;
    auto cond = parseExpression();
    expectKeyword("alors");

    std::vector<std::shared_ptr<Stmt>> thenBlock;
    std::vector<ElseIfBlock> elseIfs;
    std::vector<std::shared_ptr<Stmt>> elseBlock;

    while (peek().type != TokenType::END_OF_FILE) {
        if (peek().type == TokenType::COMMENT) { next(); continue; }
        consumeSemicolons();

        std::string n = normalize(peek().value);
        if (n == "finsi" || n == "fin_si") {
            next();
            break;
        }
        if (n == "fin") {
            if (normalize(peek(1).value) == "si") {
                next(); // consume fin
                next(); // consume si
                break;
            } else if (peek(1).type == TokenType::END_OF_FILE) {
                next();
                break;
            }
        }

        // Chained else-if: "Sinon Si <cond> Alors" on the SAME line, or single keyword "sinonsi"
        if (n == "sinonsi" || (n == "sinon" && normalize(peek(1).value) == "si" && peek(1).line == peek().line)) {
            next(); // consume sinon or sinonsi
            if (n == "sinon") {
                next(); // consume si
            }
            auto eiCond = parseExpression();
            expectKeyword("alors");
            std::vector<std::shared_ptr<Stmt>> eiBody;
            while (peek().type != TokenType::END_OF_FILE) {
                consumeSemicolons();
                if (peek().type == TokenType::COMMENT) { next(); continue; }
                std::string in = normalize(peek().value);
                if (in == "sinon" || in == "sinonsi" || in == "finsi" || in == "fin_si") break;
                if (in == "fin" && (normalize(peek(1).value) == "si" || peek(1).type == TokenType::END_OF_FILE)) break;
                eiBody.push_back(parseStatement());
                consumeSemicolons();
            }
            elseIfs.push_back({eiCond, std::move(eiBody)});
            continue;
        }

        if (n == "sinon") {
            next(); // consume sinon
            // Else block
            while (peek().type != TokenType::END_OF_FILE) {
                consumeSemicolons();
                if (peek().type == TokenType::COMMENT) { next(); continue; }
                std::string in = normalize(peek().value);
                if (in == "finsi" || in == "fin_si") { next(); break; }
                if (in == "fin") {
                    if (normalize(peek(1).value) == "si") {
                        next(); // consume fin
                        next(); // consume si
                        break;
                    } else if (peek(1).type == TokenType::END_OF_FILE) {
                        next();
                        break;
                    } else {
                        break;
                    }
                }
                elseBlock.push_back(parseStatement());
                consumeSemicolons();
            }
            break;
        }

        thenBlock.push_back(parseStatement());
        consumeSemicolons();
    }

    return makeNode<IfStmt>(cond, std::move(thenBlock), std::move(elseIfs), std::move(elseBlock), line);
}

std::shared_ptr<Stmt> Parser::parseFor() {
    Token forTok = next(); // consume pour
    int line = forTok.line;
    Token varTok = expect(TokenType::IDENTIFIER);
    expectKeyword("de");
    auto startExpr = parseExpression();

    // expect 'à' or 'a'
    if (normalize(peek().value) == "a") next();
    else error("Attendu 'à' dans la boucle Pour");

    auto endExpr = parseExpression();

    std::shared_ptr<Expr> stepExpr = nullptr;
    bool hasBracket = false;
    bool hasParen = false;
    if (match(TokenType::DELIMITER, "[")) hasBracket = true;
    else if (match(TokenType::DELIMITER, "(")) hasParen = true;

    if (normalize(peek().value) == "pas") {
        next(); // consume pas
        // Check optional =: "pas = -1"
        match(TokenType::OPERATOR, "=");
        stepExpr = parseExpression();
        if (hasBracket) expect(TokenType::DELIMITER, "]");
        if (hasParen) expect(TokenType::DELIMITER, ")");
    } else if (hasBracket || hasParen) {
        error("Attendu 'Pas' dans les crochets de la boucle Pour");
    }

    expectKeyword("faire");
    std::vector<std::shared_ptr<Stmt>> body;

    while (peek().type != TokenType::END_OF_FILE) {
        consumeSemicolons();
        std::string n = normalize(peek().value);
        if (n == "finpour") { next(); break; }
        if (n == "fin") {
            next();
            if (normalize(peek().value) == "pour") next();
            break;
        }
        body.push_back(parseStatement());
        consumeSemicolons();
    }

    return makeNode<ForStmt>(varTok.value, startExpr, endExpr, stepExpr, std::move(body), line);
}

std::shared_ptr<Stmt> Parser::parseWhile() {
    Token wTok = next(); // consume tant
    int line = wTok.line;
    if (normalize(wTok.value) == "tant") {
        expectKeyword("que");
    }
    auto cond = parseExpression();
    expectKeyword("faire");

    std::vector<std::shared_ptr<Stmt>> body;
    while (peek().type != TokenType::END_OF_FILE) {
        consumeSemicolons();
        std::string n = normalize(peek().value);
        if (n == "fintantque") { next(); break; }
        if (n == "fin") {
            next();
            if (normalize(peek().value) == "tantque") {
                next();
            } else if (normalize(peek().value) == "tant") {
                next();
                if (normalize(peek().value) == "que") next();
            }
            break;
        }
        body.push_back(parseStatement());
        consumeSemicolons();
    }

    return makeNode<WhileStmt>(cond, std::move(body), line);
}

std::shared_ptr<Stmt> Parser::parseRepeat() {
    Token rTok = next(); // consume repeter
    int line = rTok.line;
    std::vector<std::shared_ptr<Stmt>> body;

    while (peek().type != TokenType::END_OF_FILE) {
        consumeSemicolons();
        std::string n = normalize(peek().value);
        if (n == "jusqu'a") break;
        body.push_back(parseStatement());
        consumeSemicolons();
    }

    expectKeyword("jusqu'a");
    auto cond = parseExpression();
    return makeNode<RepeatStmt>(std::move(body), cond, line);
}

std::shared_ptr<Stmt> Parser::parseReturn() {
    Token retTok = next(); // consume retourner
    int line = retTok.line;
    auto val = parseExpression();
    return makeNode<ReturnStmt>(val, line);
}

// Expression parsing
std::shared_ptr<Expr> Parser::parseExpression() {
    return parseOr();
}

std::shared_ptr<Expr> Parser::parseOr() {
    auto left = parseAnd();
    while (normalize(peek().value) == "ou") {
        Token opTok = next();
        auto right = parseAnd();
        left = makeNode<BinaryExpr>("Ou", left, right, opTok.line);
    }
    return left;
}

std::shared_ptr<Expr> Parser::parseAnd() {
    auto left = parseComparison();
    while (normalize(peek().value) == "et") {
        Token opTok = next();
        auto right = parseComparison();
        left = makeNode<BinaryExpr>("Et", left, right, opTok.line);
    }
    return left;
}

std::shared_ptr<Expr> Parser::parseComparison() {
    auto left = parseAddition();

    struct CompItem {
        std::shared_ptr<Expr> left;
        std::string op;
        std::shared_ptr<Expr> right;
        int line;
    };
    std::vector<CompItem> comparisons;

    while (true) {
        std::string op = peek().value;
        int line = peek().line;

        if (peek().type == TokenType::OPERATOR &&
            (op == "=" || op == "<" || op == ">" || op == "<=" || op == ">=" ||
             op == "≠" || op == "!=" || op == "≤" || op == "≥" || op == "∈")) {
            next();
            std::shared_ptr<Expr> right;
            if (op == "∈") {
                if (match(TokenType::DELIMITER, "{")) {
                    std::vector<std::shared_ptr<Expr>> elements;
                    if (!match(TokenType::DELIMITER, "}")) {
                        do {
                            elements.push_back(parseExpression());
                        } while (match(TokenType::DELIMITER, ","));
                        expect(TokenType::DELIMITER, "}");
                    }
                    right = makeNode<SetLiteralExpr>(std::move(elements), line);
                } else if (match(TokenType::DELIMITER, "[")) {
                    auto firstExpr = parseExpression();
                    if (match(TokenType::DELIMITER, "..")) {
                        auto secondExpr = parseExpression();
                        expect(TokenType::DELIMITER, "]");
                        auto ge = makeNode<BinaryExpr>(">=", left, firstExpr, line);
                        auto le = makeNode<BinaryExpr>("<=", left, secondExpr, line);
                        left = makeNode<BinaryExpr>("Et", ge, le, line);
                        continue;
                    } else {
                        std::vector<std::shared_ptr<Expr>> elements;
                        elements.push_back(firstExpr);
                        while (match(TokenType::DELIMITER, ",")) {
                            elements.push_back(parseExpression());
                        }
                        expect(TokenType::DELIMITER, "]");
                        right = makeNode<SetLiteralExpr>(std::move(elements), line);
                    }
                } else {
                    right = parseAddition();
                }
            } else {
                right = parseAddition();
            }

            comparisons.push_back({left, op, right, line});
            left = right;
        } else {
            break;
        }
    }

    if (comparisons.empty()) return left;

    // Chained comparisons: a <= b <= c -> (a <= b) Et (b <= c)
    std::shared_ptr<Expr> result = makeNode<BinaryExpr>(
        comparisons[0].op, comparisons[0].left, comparisons[0].right, comparisons[0].line
    );

    for (size_t i = 1; i < comparisons.size(); i++) {
        auto nextComp = makeNode<BinaryExpr>(
            comparisons[i].op, comparisons[i - 1].right, comparisons[i].right, comparisons[i].line
        );
        result = makeNode<BinaryExpr>("Et", result, nextComp, comparisons[i].line);
    }

    return result;
}

std::shared_ptr<Expr> Parser::parseAddition() {
    auto left = parseTerm();
    while (peek().type == TokenType::OPERATOR && (peek().value == "+" || peek().value == "-")) {
        Token opTok = next();
        auto right = parseTerm();
        left = makeNode<BinaryExpr>(opTok.value, left, right, opTok.line);
    }
    return left;
}

std::shared_ptr<Expr> Parser::parseTerm() {
    auto left = parseFactor();
    while (peek().type == TokenType::OPERATOR &&
           (peek().value == "*" || peek().value == "/" || peek().value == "%" ||
            normalize(peek().value) == "div" || normalize(peek().value) == "mod")) {
        Token opTok = next();
        auto right = parseFactor();
        left = makeNode<BinaryExpr>(normalize(opTok.value), left, right, opTok.line);
    }
    return left;
}

std::shared_ptr<Expr> Parser::parseFactor() {
    Token tok = peek();

    // Unary minus
    if (tok.type == TokenType::OPERATOR && tok.value == "-") {
        next();
        return makeNode<UnaryExpr>("-", parseFactor(), tok.line);
    }

    // Unary Non
    if (normalize(tok.value) == "non") {
        next();
        return makeNode<UnaryExpr>("Non", parseFactor(), tok.line);
    }

    // Parentheses
    if (match(TokenType::DELIMITER, "(")) {
        auto expr = parseExpression();
        expect(TokenType::DELIMITER, ")");
        return expr;
    }

    // Number literal (integer)
    if (tok.type == TokenType::NUMBER) {
        next();
        return makeNode<NumberExpr>(std::stoll(tok.value), tok.line);
    }

    // Real literal (float)
    if (tok.type == TokenType::REAL) {
        next();
        return makeNode<RealExpr>(std::stod(tok.value), tok.line);
    }

    // String literal
    if (tok.type == TokenType::STRING) {
        next();
        return makeNode<StringExpr>(tok.value, tok.line);
    }

    // Boolean literals
    if (normalize(tok.value) == "vrai") {
        next();
        return makeNode<BoolExpr>(true, tok.line);
    }
    if (normalize(tok.value) == "faux") {
        next();
        return makeNode<BoolExpr>(false, tok.line);
    }

    // Identifier: variable, array access, or function call
    if (tok.type == TokenType::IDENTIFIER) {
        Token idTok = next();
        int line = idTok.line;

        std::shared_ptr<Expr> expr;
        // Function call: name(args)
        if (match(TokenType::DELIMITER, "(")) {
            std::vector<std::shared_ptr<Expr>> args;
            if (!match(TokenType::DELIMITER, ")")) {
                do {
                    args.push_back(parseExpression());
                } while (match(TokenType::DELIMITER, ","));
                expect(TokenType::DELIMITER, ")");
            }
            expr = makeNode<CallExpr>(idTok.value, std::move(args), line);
        } else {
            expr = makeNode<VariableExpr>(idTok.value, line);
        }

        while (true) {
            if (match(TokenType::DELIMITER, ".")) {
                Token mTok = expect(TokenType::IDENTIFIER);
                expr = makeNode<MemberAccessExpr>(expr, mTok.value, mTok.line);
            } else if (peek().type == TokenType::DELIMITER && peek().value == "[") {
                if (normalize(peek(1).value) == "pas") {
                    break;
                }
                next(); // consume [
                auto idx1 = parseExpression();
                std::shared_ptr<Expr> idx2 = nullptr;
                if (match(TokenType::DELIMITER, ",")) {
                    idx2 = parseExpression();
                }
                expect(TokenType::DELIMITER, "]");
                if (idx2 == nullptr && match(TokenType::DELIMITER, "[")) {
                    idx2 = parseExpression();
                    expect(TokenType::DELIMITER, "]");
                }
                expr = makeNode<ArrayAccessExpr>(expr, idx1, idx2, line);
            } else {
                break;
            }
        }
        return expr;
    }

    error("Expression inattendue");
    return nullptr;
}

bool Parser::isCaseHeader() {
    if (peek().type == TokenType::END_OF_FILE) return false;
    std::string n = normalize(peek().value);
    if (n == "sinon" || n == "autrement" || n == "cas") return true;

    if (n == "finselon" || n == "fin_selon") return false;
    if (n == "fin") {
        if (normalize(peek(1).value) == "selon") return false;
    }

    size_t i = 0;
    int parenDepth = 0;
    while (peek(i).type != TokenType::END_OF_FILE) {
        TokenType t = peek(i).type;
        std::string val = peek(i).value;
        std::string norm = normalize(val);

        if (t == TokenType::ARROW || val == ":=" || val == "<-" ||
            norm == "fin" || norm == "finselon" || norm == "fin_selon" ||
            norm == "si" || norm == "pour" || norm == "tant" || norm == "tantque" ||
            norm == "repeter" || norm == "ecrire" || norm == "ecrire_nl" ||
            norm == "lire" || norm == "retourner" || norm == "selon") {
            return false;
        }

        if (t == TokenType::DELIMITER && val == "(") {
            parenDepth++;
        } else if (t == TokenType::DELIMITER && val == ")") {
            parenDepth--;
        }

        if (parenDepth == 0 && t == TokenType::DELIMITER && val == ":") {
            return i > 0;
        }

        // If line changed and previous token wasn't a continuation token
        if (peek(i + 1).type != TokenType::END_OF_FILE && peek(i + 1).line != peek(i).line) {
            if (val != "," && val != ".." && val != "+" && val != "-") {
                return false;
            }
        }
        i++;
    }
    return false;
}

std::shared_ptr<Stmt> Parser::parseSwitch() {
    Token selonTok = next(); // consume 'selon'
    int line = selonTok.line;
    auto targetExpr = parseExpression();

    // Optional 'faire'
    if (normalize(peek().value) == "faire") {
        next();
    }

    std::vector<SwitchCase> cases;

    while (peek().type != TokenType::END_OF_FILE) {
        if (peek().type == TokenType::COMMENT) { next(); continue; }
        consumeSemicolons();

        std::string n = normalize(peek().value);
        if (n == "finselon" || n == "fin_selon") {
            next();
            break;
        }
        if (n == "fin") {
            next();
            if (normalize(peek().value) == "selon") {
                next();
            }
            break;
        }

        if (!isCaseHeader()) {
            break;
        }

        SwitchCase sc;
        n = normalize(peek().value);
        if (n == "cas") {
            next();
            n = normalize(peek().value);
        }

        if (n == "autrement" || n == "sinon") {
            next(); // consume 'autrement' or 'sinon'
            sc.isDefault = true;
            if (peek().type == TokenType::DELIMITER && peek().value == ":") {
                next();
            }
        } else {
            while (true) {
                auto startExpr = parseExpression();
                std::shared_ptr<Expr> endExpr = nullptr;
                if (peek().type == TokenType::DELIMITER &&
                    (peek().value == ".." || (peek().value == "." && peek(1).value == "."))) {
                    if (peek().value == "..") {
                        next();
                    } else {
                        next();
                        next();
                    }
                    endExpr = parseExpression();
                }
                sc.items.push_back({startExpr, endExpr});
                if (!endExpr) {
                    sc.values.push_back(startExpr);
                }
                if (match(TokenType::DELIMITER, ",")) {
                    continue;
                }
                break;
            }
            expect(TokenType::DELIMITER, ":");
        }

        // Parse case statements
        while (peek().type != TokenType::END_OF_FILE) {
            consumeSemicolons();
            if (peek().type == TokenType::COMMENT) { next(); continue; }

            std::string in = normalize(peek().value);
            if (in == "finselon" || in == "fin_selon") break;
            if (in == "fin" && (normalize(peek(1).value) == "selon" || peek(1).type == TokenType::END_OF_FILE)) break;
            if (isCaseHeader()) break;

            sc.body.push_back(parseStatement());
            consumeSemicolons();
        }

        cases.push_back(std::move(sc));
    }

    return makeNode<SwitchStmt>(targetExpr, std::move(cases), line);
}

} // namespace algo