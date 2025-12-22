#include "lexer.h"
#include <cctype>
#include <iostream>

const std::unordered_map<std::string, TokenType> Lexer::keywords = {
    {"if", TokenType::IF},
    {"elif", TokenType::ELIF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"for", TokenType::FOR},
    {"return", TokenType::RETURN},
    {"print", TokenType::PRINT},
    {"int", TokenType::INT},
    {"float", TokenType::FLOAT_KW},
    {"bool", TokenType::BOOL},
    {"void", TokenType::VOID},
    {"true", TokenType::TRUE_LIT},
    {"false", TokenType::FALSE_LIT},
    {"let", TokenType::LET},
    {"input", TokenType::INPUT},
    {"key_pressed", TokenType::KEY_PRESSED},
    {"screen", TokenType::SCREEN},
    {"drawPixel", TokenType::DRAW_PIXEL},
    {"drawRect", TokenType::DRAW_RECT},
    {"drawLine", TokenType::DRAW_LINE},
    {"drawCircle", TokenType::DRAW_CIRCLE},
    {"clearScreen", TokenType::CLEAR_SCREEN},
    {"display", TokenType::DISPLAY},
    {"quit", TokenType::QUIT},
    {"isKeyDown", TokenType::IS_KEY_DOWN},
    {"updateInput", TokenType::UPDATE_INPUT}
};

Lexer::Lexer(const std::string& source)
    : source(source), position(0), line(1), column(1) {}

char Lexer::currentChar() {
    if (position >= source.length()) {
        return '\0';
    }
    return source[position];
}

char Lexer::peekChar(int offset) {
    size_t peekPos = position + offset;
    if (peekPos >= source.length()) {
        return '\0';
    }
    return source[peekPos];
}

void Lexer::advance() {
    if (position < source.length()) {
        if (source[position] == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        position++;
    }
}

void Lexer::skipWhitespace() {
    while (currentChar() != '\0' && std::isspace(currentChar()) && currentChar() != '\n') {
        advance();
    }
}

void Lexer::skipComment() {
    if (currentChar() == '/' && peekChar() == '/') {
        // Line comment
        advance();
        advance();
        while (currentChar() != '\0' && currentChar() != '\n') {
            advance();
        }
    } else if (currentChar() == '/' && peekChar() == '*') {
        // Block comment
        advance();
        advance();
        while (currentChar() != '\0') {
            if (currentChar() == '*' && peekChar() == '/') {
                advance();
                advance();
                break;
            }
            advance();
        }
    }
}

Token Lexer::readNumber() {
    int startCol = column;
    std::string number;
    
    while (currentChar() != '\0' && (std::isdigit(currentChar()) || currentChar() == '.')) {
        number += currentChar();
        advance();
    }
    
    if (number.find('.') != std::string::npos) {
        return Token(TokenType::FLOAT, number, line, startCol);
    } else {
        return Token(TokenType::INTEGER, number, line, startCol);
    }
}

Token Lexer::readString() {
    int startCol = column;
    char quote = currentChar();
    advance(); // Skip opening quote
    
    std::string str;
    while (currentChar() != '\0' && currentChar() != quote) {
        if (currentChar() == '\\') {
            advance();
            switch (currentChar()) {
                case 'n':
                    str += '\n';
                    break;
                case 't':
                    str += '\t';
                    break;
                case '\\':
                    str += '\\';
                    break;
                case '"':
                    str += '"';
                    break;
                case '\'':
                    str += '\'';
                    break;
                default:
                    str += currentChar();
            }
            advance();
        } else {
            str += currentChar();
            advance();
        }
    }
    
    if (currentChar() == quote) {
        advance(); // Skip closing quote
    }
    
    return Token(TokenType::STRING, str, line, startCol);
}

Token Lexer::readIdentifierOrKeyword() {
    int startCol = column;
    std::string identifier;
    
    while (currentChar() != '\0' && (std::isalnum(currentChar()) || currentChar() == '_')) {
        identifier += currentChar();
        advance();
    }
    
    // Check if it's a keyword
    auto it = keywords.find(identifier);
    if (it != keywords.end()) {
        return Token(it->second, identifier, line, startCol);
    } else {
        return Token(TokenType::IDENTIFIER, identifier, line, startCol);
    }
}

Token Lexer::readOperatorOrDelimiter() {
    int startCol = column;
    char ch = currentChar();
    
    switch (ch) {
        case '+':
            advance();
            return Token(TokenType::PLUS, "+", line, startCol);
        case '-':
            advance();
            return Token(TokenType::MINUS, "-", line, startCol);
        case '*':
            advance();
            return Token(TokenType::STAR, "*", line, startCol);
        case '/':
            advance();
            return Token(TokenType::SLASH, "/", line, startCol);
        case '%':
            advance();
            return Token(TokenType::PERCENT, "%", line, startCol);
        case '=':
            advance();
            if (currentChar() == '=') {
                advance();
                return Token(TokenType::EQUAL, "==", line, startCol);
            }
            return Token(TokenType::ASSIGN, "=", line, startCol);
        case '!':
            advance();
            if (currentChar() == '=') {
                advance();
                return Token(TokenType::NOT_EQUAL, "!=", line, startCol);
            }
            return Token(TokenType::NOT, "!", line, startCol);
        case '<':
            advance();
            if (currentChar() == '=') {
                advance();
                return Token(TokenType::LESS_EQUAL, "<=", line, startCol);
            }
            return Token(TokenType::LESS, "<", line, startCol);
        case '>':
            advance();
            if (currentChar() == '=') {
                advance();
                return Token(TokenType::GREATER_EQUAL, ">=", line, startCol);
            }
            return Token(TokenType::GREATER, ">", line, startCol);
        case '&':
            advance();
            if (currentChar() == '&') {
                advance();
                return Token(TokenType::AND, "&&", line, startCol);
            }
            break;
        case '|':
            advance();
            if (currentChar() == '|') {
                advance();
                return Token(TokenType::OR, "||", line, startCol);
            }
            break;
        case '(':
            advance();
            return Token(TokenType::LPAREN, "(", line, startCol);
        case ')':
            advance();
            return Token(TokenType::RPAREN, ")", line, startCol);
        case '{':
            advance();
            return Token(TokenType::LBRACE, "{", line, startCol);
        case '}':
            advance();
            return Token(TokenType::RBRACE, "}", line, startCol);
        case '[':
            advance();
            return Token(TokenType::LBRACKET, "[", line, startCol);
        case ']':
            advance();
            return Token(TokenType::RBRACKET, "]", line, startCol);
        case ';':
            advance();
            return Token(TokenType::SEMICOLON, ";", line, startCol);
        case ',':
            advance();
            return Token(TokenType::COMMA, ",", line, startCol);
        case '.':
            advance();
            return Token(TokenType::DOT, ".", line, startCol);
        case ':':
            advance();
            return Token(TokenType::COLON, ":", line, startCol);
        default:
            advance();
            return Token(TokenType::UNKNOWN, std::string(1, ch), line, startCol);
    }
    
    advance();
    return Token(TokenType::UNKNOWN, std::string(1, ch), line, startCol);
}

Token Lexer::nextToken() {
    skipWhitespace();
    
    // Skip comments
    while ((currentChar() == '/' && (peekChar() == '/' || peekChar() == '*'))) {
        skipComment();
        skipWhitespace();
    }
    
    if (currentChar() == '\0') {
        return Token(TokenType::END_OF_FILE, "", line, column);
    }
    
    if (currentChar() == '\n') {
        int startCol = column;
        advance();
        return Token(TokenType::NEWLINE, "\n", line - 1, startCol);
    }
    
    if (std::isdigit(currentChar())) {
        return readNumber();
    }
    
    if (currentChar() == '"' || currentChar() == '\'') {
        return readString();
    }
    
    if (std::isalpha(currentChar()) || currentChar() == '_') {
        return readIdentifierOrKeyword();
    }
    
    return readOperatorOrDelimiter();
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    Token token = nextToken();
    
    while (token.type != TokenType::END_OF_FILE) {
        tokens.push_back(token);
        token = nextToken();
    }
    
    tokens.push_back(token); // Add EOF token
    return tokens;
}
