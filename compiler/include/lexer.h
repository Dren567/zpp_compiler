#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <unordered_map>

// TokenType enum: Represents all possible token types in the ZPP language
// This includes literals (integers, floats, strings), keywords, operators, and delimiters
enum class TokenType {
    // Literals
    INTEGER,
    FLOAT,
    STRING,
    IDENTIFIER,
    TRUE_LIT,
    FALSE_LIT,
    ARRAY,    

    // Keywords
    IF,
    ELIF,
    ELSE,
    WHILE,
    FOR,
    RETURN,
    PRINT,
    INT,
    FLOAT_KW,
    BOOL,
    VOID,
    LET,
    INPUT,
    KEY_PRESSED,
    SCREEN,
    DRAW_PIXEL,
    DRAW_RECT,
    DRAW_LINE,
    DRAW_CIRCLE,
    CLEAR_SCREEN,
    DISPLAY,
    QUIT,
    IS_KEY_DOWN,
    UPDATE_INPUT,
    
    // Operators
    PLUS,
    MINUS,
    STAR,
    SLASH,
    PERCENT,
    ASSIGN,
    EQUAL,
    NOT_EQUAL,
    LESS,
    GREATER,
    LESS_EQUAL,
    GREATER_EQUAL,
    AND,
    OR,
    NOT,
    
    // Delimiters
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    LBRACKET,
    RBRACKET,
    SEMICOLON,
    COMMA,
    DOT,
    COLON,
    
    // Special
    NEWLINE,
    END_OF_FILE,
    UNKNOWN
};

// Token struct: Represents a single lexical token with type, value, and source location
struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    
    Token(TokenType t = TokenType::UNKNOWN, const std::string& v = "", int l = 1, int c = 1)
        : type(t), value(v), line(l), column(c) {}
};

// Lexer class: Converts source code text into a stream of tokens
// Handles keywords, operators, string/number literals, and comments
class Lexer {
public:
    // Constructor: Initialize lexer with source code
    explicit Lexer(const std::string& source);
    
    // nextToken(): Returns the next token in the stream
    Token nextToken();
    
    // tokenize(): Tokenizes entire source and returns all tokens at once
    std::vector<Token> tokenize();
    
private:
    std::string source;           // The source code being tokenized
    size_t position;              // Current position in source
    int line, column;             // Current line and column for error reporting
    
    static const std::unordered_map<std::string, TokenType> keywords;  // Keyword lookup table
    
    // Helper methods for character and string processing
    char currentChar();                           // Get current character
    char peekChar(int offset = 1);               // Peek ahead at next character(s)
    void advance();                              // Move to next character
    void skipWhitespace();                       // Skip spaces/tabs (not newlines)
    void skipComment();                          // Skip // comments
    
    // Token-specific parsing methods
    Token readNumber();                          // Parse integer or float literals
    Token readString();                          // Parse string literals
    Token readIdentifierOrKeyword();             // Parse identifiers and check for keywords
    Token readOperatorOrDelimiter();             // Parse operators and delimiters
};

#endif // LEXER_H
