#include "parser.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), current(0) {}

Token Parser::currentToken() {
    if (current < tokens.size()) return tokens[current];
    return Token(TokenType::END_OF_FILE, "", 0, 0);
}

Token Parser::peekToken(int offset) {
    size_t idx = current + offset;
    if (idx < tokens.size()) return tokens[idx];
    return Token(TokenType::END_OF_FILE, "", 0, 0);
}

Token Parser::advance() {
    if (current < tokens.size()) ++current;
    return currentToken();
}

bool Parser::check(TokenType type) {
    return currentToken().type == type;
}

bool Parser::match(const std::vector<TokenType>& types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

void Parser::consume(TokenType type, const std::string& message) {
    if (!check(type)) throw std::runtime_error(message);
    advance();
}

bool Parser::isType(TokenType type) {
    return type == TokenType::INT || type == TokenType::FLOAT_KW || type == TokenType::BOOL || type == TokenType::VOID || type == TokenType::IDENTIFIER;
}

std::string Parser::tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::INT: return "int";
        case TokenType::FLOAT_KW: return "float";
        case TokenType::BOOL: return "bool";
        case TokenType::VOID: return "void";
        case TokenType::IDENTIFIER: return currentToken().value;
        default: return "unknown";
    }
}

ProgramPtr Parser::parse() {
    auto program = std::make_shared<Program>();
    while (!check(TokenType::END_OF_FILE)) {
        while (check(TokenType::NEWLINE)) advance();
        if (check(TokenType::END_OF_FILE)) break;
        program->functions.push_back(parseFunction());
    }
    return program;
}

FunctionDeclPtr Parser::parseFunction() {
    std::string returnType = "void";
    // Handle optional return type. Disambiguate IDENTIFIER: if the current token
    // is an identifier and the next token is '(', then this identifier is the
    // function name (no explicit return type). Otherwise treat it as a type.
    if (isType(currentToken().type)) {
        if (currentToken().type == TokenType::IDENTIFIER && peekToken().type == TokenType::LPAREN) {
            // no explicit return type; leave returnType as "void"
        } else {
            returnType = tokenTypeToString(currentToken().type);
            advance();
        }
    }
    if (!check(TokenType::IDENTIFIER)) throw std::runtime_error("Expected function name");
    std::string name = currentToken().value;
    advance();
    consume(TokenType::LPAREN, "Expected '('");
    std::vector<std::pair<std::string, std::string>> params;
    if (!check(TokenType::RPAREN)) {
        do {
            std::string paramType = tokenTypeToString(currentToken().type);
            advance();
            if (!check(TokenType::IDENTIFIER)) throw std::runtime_error("Expected parameter name");
            std::string paramName = currentToken().value;
            advance();
            params.emplace_back(paramType, paramName);
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RPAREN, "Expected ')'");
    auto body = parseBlockStatement();
    auto func = std::make_shared<FunctionDecl>(returnType, name);
    func->parameters = params;
    func->body = body;
    return func;
}

StatementPtr Parser::parseStatement() {
    if (check(TokenType::LBRACE)) return parseBlockStatement();
    if (check(TokenType::RETURN)) return parseReturnStatement();
    if (check(TokenType::IF)) return parseIfStatement();
    if (check(TokenType::WHILE)) return parseWhileStatement();
    if (check(TokenType::FOR)) return parseForStatement();
    // Disambiguate IDENTIFIER: only treat an IDENTIFIER as a type when the
    // following token is also an IDENTIFIER (i.e. `MyType var;`). This avoids
    // parsing assignments like `x = 1;` as declarations.
    bool looksLikeType = false;
    if (currentToken().type == TokenType::IDENTIFIER) {
        looksLikeType = (peekToken().type == TokenType::IDENTIFIER);
    } else {
        looksLikeType = isType(currentToken().type);
    }
    if (looksLikeType || check(TokenType::LET)) return parseVariableDeclaration();
    if (check(TokenType::PRINT)) return parsePrintStatement();
    return parseExpressionStatement();
}

StatementPtr Parser::parseBlockStatement() {
    consume(TokenType::LBRACE, "Expected '{'");
    auto block = std::make_shared<BlockStatement>();
    while (!check(TokenType::RBRACE) && !check(TokenType::END_OF_FILE)) {
        while (check(TokenType::NEWLINE)) advance();
        if (check(TokenType::RBRACE) || check(TokenType::END_OF_FILE)) break;
        block->statements.push_back(parseStatement());
    }
    consume(TokenType::RBRACE, "Expected '}'");
    return block;
}

StatementPtr Parser::parseReturnStatement() {
    consume(TokenType::RETURN, "Expected 'return'");
    ExpressionPtr expr = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        expr = parseExpression();
    }
    consume(TokenType::SEMICOLON, "Expected ';' after return");
    return std::make_shared<ReturnStatement>(expr);
}

StatementPtr Parser::parseIfStatement() {
    if (check(TokenType::IF)) {
        advance();
    } else if (check(TokenType::ELIF)) {
        advance();
    } else {
        throw std::runtime_error("Expected 'if' or 'elif'");
    }
    
    consume(TokenType::LPAREN, "Expected '(' after condition");
    auto condition = parseExpression();
    consume(TokenType::RPAREN, "Expected ')' after condition");
    auto thenBranch = parseStatement();
    StatementPtr elseBranch = nullptr;
    if (check(TokenType::ELIF)) {
        elseBranch = parseIfStatement();
    } else if (check(TokenType::ELSE)) {
        advance();
        elseBranch = parseStatement();
    }
    return std::make_shared<IfStatement>(condition, thenBranch, elseBranch);
}

StatementPtr Parser::parseWhileStatement() {
    consume(TokenType::WHILE, "Expected 'while'");
    consume(TokenType::LPAREN, "Expected '(' after 'while'");
    auto condition = parseExpression();
    consume(TokenType::RPAREN, "Expected ')' after while condition");
    auto body = parseStatement();
    return std::make_shared<WhileStatement>(condition, body);
}

StatementPtr Parser::parseForStatement() {
    consume(TokenType::FOR, "Expected 'for'");
    consume(TokenType::LPAREN, "Expected '(' after 'for'");
    StatementPtr init = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        // Parse init without consuming the trailing semicolon (for-loop will consume it)
        if (check(TokenType::LET)) {
            // parse let name:type = expr  but do not consume the final ';'
            consume(TokenType::LET, "Expected 'let'");
            if (!check(TokenType::IDENTIFIER)) throw std::runtime_error("Expected variable name");
            std::string name = currentToken().value;
            advance();
            consume(TokenType::COLON, "Expected ':' after variable name");
            if (!isType(currentToken().type)) throw std::runtime_error("Expected type after ':'");
            std::string type = tokenTypeToString(currentToken().type);
            advance();
            consume(TokenType::ASSIGN, "Expected '=' after type");
            auto initializer = parseExpression();
            init = std::make_shared<VariableDecl>(name, type, initializer);
        } else if (isType(currentToken().type) || currentToken().type == TokenType::IDENTIFIER) {
            // C-style: type name [= initializer]  (don't consume semicolon)
            if (!isType(currentToken().type)) throw std::runtime_error("Expected type for variable declaration");
            std::string type = tokenTypeToString(currentToken().type);
            advance();
            if (!check(TokenType::IDENTIFIER)) throw std::runtime_error("Expected variable name");
            std::string name = currentToken().value;
            advance();
            ExpressionPtr initializer = nullptr;
            if (check(TokenType::ASSIGN)) {
                advance();
                initializer = parseExpression();
            }
            init = std::make_shared<VariableDecl>(name, type, initializer);
        } else {
            init = parseExpressionStatement();
        }
    }
    consume(TokenType::SEMICOLON, "Expected ';' after for-init");
    ExpressionPtr condition = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        condition = parseExpression();
    }
    consume(TokenType::SEMICOLON, "Expected ';' after for-condition");
    ExpressionPtr increment = nullptr;
    if (!check(TokenType::RPAREN)) {
        increment = parseExpression();
    }
    consume(TokenType::RPAREN, "Expected ')' after for clauses");
    auto body = parseStatement();
    return std::make_shared<ForStatement>(init, condition, increment, body);
}

StatementPtr Parser::parseVariableDeclaration() {
    // Support two forms:
    // 1) let name:type = expr;
    // 2) type name = expr;   (C-style declarations used by tests)
    if (check(TokenType::LET)) {
        consume(TokenType::LET, "Expected 'let'");
        if (!check(TokenType::IDENTIFIER)) throw std::runtime_error("Expected variable name");
        std::string name = currentToken().value;
        advance();
        consume(TokenType::COLON, "Expected ':' after variable name");
        if (!isType(currentToken().type)) throw std::runtime_error("Expected type after ':'");
        std::string type = tokenTypeToString(currentToken().type);
        advance();
        consume(TokenType::ASSIGN, "Expected '=' after type");
        auto initializer = parseExpression();
        consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");
        return std::make_shared<VariableDecl>(name, type, initializer);
    } else {
        // C-style: type name [= initializer] ;
        if (!isType(currentToken().type)) throw std::runtime_error("Expected type for variable declaration");
        std::string type = tokenTypeToString(currentToken().type);
        advance();
        if (!check(TokenType::IDENTIFIER)) throw std::runtime_error("Expected variable name");
        std::string name = currentToken().value;
        advance();
        ExpressionPtr initializer = nullptr;
        if (check(TokenType::ASSIGN)) {
            advance();
            initializer = parseExpression();
        }
        consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");
        return std::make_shared<VariableDecl>(name, type, initializer);
    }
}

StatementPtr Parser::parsePrintStatement() {
    consume(TokenType::PRINT, "Expected 'print'");
    consume(TokenType::LPAREN, "Expected '(' after 'print'");
    auto expr = parseExpression();
    consume(TokenType::RPAREN, "Expected ')'");
    consume(TokenType::SEMICOLON, "Expected ';' after print statement");
    return std::make_shared<PrintStatement>(expr);
}

StatementPtr Parser::parseExpressionStatement() {
    auto expr = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression");
    return std::make_shared<ExpressionStatement>(expr);
}

// --- Expression Parsing ---
ExpressionPtr Parser::parseExpression() { return parseComma(); }

ExpressionPtr Parser::parseComma() {
    auto left = parseAssignment();
    
    while (check(TokenType::COMMA)) {
        Token op = currentToken();
        advance();
        auto right = parseAssignment();
        
        left = std::make_shared<BinaryOp>(left, op.type, right);
    }
    
    return left;
}

ExpressionPtr Parser::parseAssignment() {
    auto left = parseLogicalOr();
    if (check(TokenType::ASSIGN)) {
        advance();
        if (auto id = std::dynamic_pointer_cast<Identifier>(left)) {
            auto value = parseAssignment();
            return std::make_shared<Assignment>(id->name, value);
        } else {
            throw std::runtime_error("Invalid assignment target");
        }
    }
    return left;
}

ExpressionPtr Parser::parseLogicalOr() {
    auto left = parseLogicalAnd();
    while (check(TokenType::OR)) {
        TokenType op = currentToken().type;
        advance();
        auto right = parseLogicalAnd();
        left = std::make_shared<BinaryOp>(left, op, right);
    }
    return left;
}

ExpressionPtr Parser::parseLogicalAnd() {
    auto left = parseEquality();
    while (check(TokenType::AND)) {
        TokenType op = currentToken().type;
        advance();
        auto right = parseEquality();
        left = std::make_shared<BinaryOp>(left, op, right);
    }
    return left;
}

ExpressionPtr Parser::parseEquality() {
    auto left = parseComparison();
    while (check(TokenType::EQUAL) || check(TokenType::NOT_EQUAL)) {
        TokenType op = currentToken().type;
        advance();
        auto right = parseComparison();
        left = std::make_shared<BinaryOp>(left, op, right);
    }
    return left;
}

ExpressionPtr Parser::parseComparison() {
    auto left = parseAdditive();
    while (check(TokenType::LESS) || check(TokenType::LESS_EQUAL) || check(TokenType::GREATER) || check(TokenType::GREATER_EQUAL)) {
        TokenType op = currentToken().type;
        advance();
        auto right = parseAdditive();
        left = std::make_shared<BinaryOp>(left, op, right);
    }
    return left;
}

ExpressionPtr Parser::parseAdditive() {
    auto left = parseMultiplicative();
    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        TokenType op = currentToken().type;
        advance();
        auto right = parseMultiplicative();
        left = std::make_shared<BinaryOp>(left, op, right);
    }
    return left;
}

ExpressionPtr Parser::parseMultiplicative() {
    auto left = parseUnary();
    while (check(TokenType::STAR) || check(TokenType::SLASH) || check(TokenType::PERCENT)) {
        TokenType op = currentToken().type;
        advance();
        auto right = parseUnary();
        left = std::make_shared<BinaryOp>(left, op, right);
    }
    return left;
}

ExpressionPtr Parser::parseUnary() {
    if (check(TokenType::NOT) || check(TokenType::MINUS)) {
        TokenType op = currentToken().type;
        advance();
        auto operand = parseUnary();
        return std::make_shared<UnaryOp>(op, operand);
    }
    return parsePostfix();
}

ExpressionPtr Parser::parsePostfix() {
    auto expr = parsePrimary();
    while (check(TokenType::LPAREN) || check(TokenType::LBRACKET)) {
        if (check(TokenType::LPAREN)) {
            advance();
            std::vector<ExpressionPtr> args;
            if (!check(TokenType::RPAREN)) {
                do {
                    args.push_back(parseAssignment());
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RPAREN, "Expected ')' after arguments");
            if (auto id = std::dynamic_pointer_cast<Identifier>(expr)) {
                expr = std::make_shared<FunctionCall>(id->name, args);
            } else {
                throw std::runtime_error("Invalid function call");
            }
        } else if (check(TokenType::LBRACKET)) {
            advance();
            auto index = parseExpression();
            consume(TokenType::RBRACKET, "Expected ']' after index");
            expr = std::make_shared<ArrayAccess>(expr, index);
        }
    }
    return expr;
}

ExpressionPtr Parser::parsePrimary() {
    if (check(TokenType::TRUE_LIT)) {
        advance();
        return std::make_shared<Literal>(TokenType::TRUE_LIT, "1");
    }
    if (check(TokenType::FALSE_LIT)) {
        advance();
        return std::make_shared<Literal>(TokenType::FALSE_LIT, "0");
    }
    if (check(TokenType::INTEGER) || check(TokenType::FLOAT) || check(TokenType::STRING)) {
        auto tok = currentToken();
        advance();
        return std::make_shared<Literal>(tok.type, tok.value);
    }
    if (check(TokenType::IDENTIFIER)) {
        std::string name = currentToken().value;
        advance();
        return std::make_shared<Identifier>(name);
    }
    if (check(TokenType::INPUT)) {
        // Support both `input`, `input()` and `input(<expr>)` forms
        advance();
        ExpressionPtr prompt = nullptr;
        if (check(TokenType::LPAREN)) {
            advance();
            if (!check(TokenType::RPAREN)) {
                prompt = parseExpression();
            }
            consume(TokenType::RPAREN, "Expected ')'");
        }
        return std::make_shared<InputCall>(prompt);
    }
    if (check(TokenType::KEY_PRESSED)) {
        // Support `key_pressed`, `key_pressed()` and `key_pressed(<expr>)`
        advance();
        ExpressionPtr prompt = nullptr;
        if (check(TokenType::LPAREN)) {
            advance();
            if (!check(TokenType::RPAREN)) {
                prompt = parseExpression();
            }
            consume(TokenType::RPAREN, "Expected ')'");
        }
        return std::make_shared<KeyPressedCall>(prompt);
    }
    if (check(TokenType::SCREEN)) {
        // Support `screen(width, height, caption)` form
        advance();
        std::vector<ExpressionPtr> args;
        if (check(TokenType::LPAREN)) {
            advance();
            if (!check(TokenType::RPAREN)) {
                do {
                    args.push_back(parseAssignment());
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RPAREN, "Expected ')'");
        }
        return std::make_shared<FunctionCall>("screen", args);
    }
    if (check(TokenType::CLEAR_SCREEN)) {
        // Support `clearScreen(r, g, b)` form
        advance();
        std::vector<ExpressionPtr> args;
        if (check(TokenType::LPAREN)) {
            advance();
            if (!check(TokenType::RPAREN)) {
                do {
                    args.push_back(parseAssignment());
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RPAREN, "Expected ')'");
        }
        return std::make_shared<FunctionCall>("clearScreen", args);
    }
    if (check(TokenType::DRAW_PIXEL)) {
        advance();
        std::vector<ExpressionPtr> args;
        if (check(TokenType::LPAREN)) {
            advance();
            if (!check(TokenType::RPAREN)) {
                do {
                    args.push_back(parseAssignment());
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RPAREN, "Expected ')'");
        }
        return std::make_shared<FunctionCall>("drawPixel", args);
    }
    if (check(TokenType::DRAW_RECT)) {
        advance();
        std::vector<ExpressionPtr> args;
        if (check(TokenType::LPAREN)) {
            advance();
            if (!check(TokenType::RPAREN)) {
                do {
                    args.push_back(parseAssignment());
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RPAREN, "Expected ')'");
        }
        return std::make_shared<FunctionCall>("drawRect", args);
    }
    if (check(TokenType::DRAW_LINE)) {
        advance();
        std::vector<ExpressionPtr> args;
        if (check(TokenType::LPAREN)) {
            advance();
            if (!check(TokenType::RPAREN)) {
                do {
                    args.push_back(parseAssignment());
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RPAREN, "Expected ')'");
        }
        return std::make_shared<FunctionCall>("drawLine", args);
    }
    if (check(TokenType::DRAW_CIRCLE)) {
        advance();
        std::vector<ExpressionPtr> args;
        if (check(TokenType::LPAREN)) {
            advance();
            if (!check(TokenType::RPAREN)) {
                do {
                    args.push_back(parseAssignment());
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RPAREN, "Expected ')'");
        }
        return std::make_shared<FunctionCall>("drawCircle", args);
    }
    if (check(TokenType::DISPLAY)) {
        advance();
        std::vector<ExpressionPtr> args;
        if (check(TokenType::LPAREN)) {
            advance();
            if (!check(TokenType::RPAREN)) {
                do {
                    args.push_back(parseAssignment());
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RPAREN, "Expected ')'");
        }
        return std::make_shared<FunctionCall>("display", args);
    }
    if (check(TokenType::QUIT)) {
        advance();
        std::vector<ExpressionPtr> args;
        if (check(TokenType::LPAREN)) {
            advance();
            if (!check(TokenType::RPAREN)) {
                do {
                    args.push_back(parseAssignment());
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RPAREN, "Expected ')'");
        }
        return std::make_shared<FunctionCall>("quit", args);
    }
    if (check(TokenType::IS_KEY_DOWN)) {
        advance();
        std::vector<ExpressionPtr> args;
        if (check(TokenType::LPAREN)) {
            advance();
            if (!check(TokenType::RPAREN)) {
                do {
                    args.push_back(parseAssignment());
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RPAREN, "Expected ')'");
        }
        return std::make_shared<FunctionCall>("isKeyDown", args);
    }
    if (check(TokenType::UPDATE_INPUT)) {
        advance();
        std::vector<ExpressionPtr> args;
        if (check(TokenType::LPAREN)) {
            advance();
            if (!check(TokenType::RPAREN)) {
                do {
                    args.push_back(parseAssignment());
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RPAREN, "Expected ')'");
        }
        return std::make_shared<FunctionCall>("updateInput", args);
    }
    if (check(TokenType::LPAREN)) {
        advance();
        auto expr = parseExpression();
        consume(TokenType::RPAREN, "Expected ')'");
        return expr;
    }
    {
        auto tok = currentToken();
        std::string msg = "Unexpected token in expression: '" + tok.value + "' (" + std::to_string((int)tok.type) + ")";
        throw std::runtime_error(msg);
    }
}
