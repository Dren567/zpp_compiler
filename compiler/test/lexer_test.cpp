#include <cassert>
#include <iostream>
#include "../include/lexer.h"

void testBasicTokens() {
    std::cout << "Testing basic tokens..." << std::endl;
    
    Lexer lexer("int x = 42;");
    auto tokens = lexer.tokenize();
    
    assert(tokens.size() >= 6);
    assert(tokens[0].type == TokenType::INT);
    assert(tokens[1].type == TokenType::IDENTIFIER);
    assert(tokens[1].value == "x");
    assert(tokens[2].type == TokenType::ASSIGN);
    assert(tokens[3].type == TokenType::INTEGER);
    assert(tokens[3].value == "42");
    assert(tokens[4].type == TokenType::SEMICOLON);
    
    std::cout << "✓ Basic tokens test passed" << std::endl;
}

void testKeywords() {
    std::cout << "Testing keywords..." << std::endl;
    
    Lexer lexer("if else while for return int float void");
    auto tokens = lexer.tokenize();
    
    assert(tokens[0].type == TokenType::IF);
    assert(tokens[1].type == TokenType::ELSE);
    assert(tokens[2].type == TokenType::WHILE);
    assert(tokens[3].type == TokenType::FOR);
    assert(tokens[4].type == TokenType::RETURN);
    assert(tokens[5].type == TokenType::INT);
    assert(tokens[6].type == TokenType::FLOAT_KW);
    assert(tokens[7].type == TokenType::VOID);
    
    std::cout << "✓ Keywords test passed" << std::endl;
}

void testOperators() {
    std::cout << "Testing operators..." << std::endl;
    
    Lexer lexer("+ - * / % = == != < > <= >= && ||");
    auto tokens = lexer.tokenize();
    
    assert(tokens[0].type == TokenType::PLUS);
    assert(tokens[1].type == TokenType::MINUS);
    assert(tokens[2].type == TokenType::STAR);
    assert(tokens[3].type == TokenType::SLASH);
    assert(tokens[4].type == TokenType::PERCENT);
    assert(tokens[5].type == TokenType::ASSIGN);
    assert(tokens[6].type == TokenType::EQUAL);
    assert(tokens[7].type == TokenType::NOT_EQUAL);
    assert(tokens[8].type == TokenType::LESS);
    assert(tokens[9].type == TokenType::GREATER);
    assert(tokens[10].type == TokenType::LESS_EQUAL);
    assert(tokens[11].type == TokenType::GREATER_EQUAL);
    assert(tokens[12].type == TokenType::AND);
    assert(tokens[13].type == TokenType::OR);
    
    std::cout << "✓ Operators test passed" << std::endl;
}

void testDelimiters() {
    std::cout << "Testing delimiters..." << std::endl;
    
    Lexer lexer("( ) { } [ ] ; , . :");
    auto tokens = lexer.tokenize();
    
    assert(tokens[0].type == TokenType::LPAREN);
    assert(tokens[1].type == TokenType::RPAREN);
    assert(tokens[2].type == TokenType::LBRACE);
    assert(tokens[3].type == TokenType::RBRACE);
    assert(tokens[4].type == TokenType::LBRACKET);
    assert(tokens[5].type == TokenType::RBRACKET);
    assert(tokens[6].type == TokenType::SEMICOLON);
    assert(tokens[7].type == TokenType::COMMA);
    assert(tokens[8].type == TokenType::DOT);
    assert(tokens[9].type == TokenType::COLON);
    
    std::cout << "✓ Delimiters test passed" << std::endl;
}

void testStrings() {
    std::cout << "Testing string literals..." << std::endl;
    
    Lexer lexer("\"hello world\" 'single'");
    auto tokens = lexer.tokenize();
    
    assert(tokens[0].type == TokenType::STRING);
    assert(tokens[0].value == "hello world");
    assert(tokens[1].type == TokenType::STRING);
    assert(tokens[1].value == "single");
    
    std::cout << "✓ String literals test passed" << std::endl;
}

void testNumbers() {
    std::cout << "Testing numeric literals..." << std::endl;
    
    Lexer lexer("42 3.14 0 100");
    auto tokens = lexer.tokenize();
    
    assert(tokens[0].type == TokenType::INTEGER);
    assert(tokens[0].value == "42");
    assert(tokens[1].type == TokenType::FLOAT);
    assert(tokens[1].value == "3.14");
    assert(tokens[2].type == TokenType::INTEGER);
    assert(tokens[2].value == "0");
    assert(tokens[3].type == TokenType::INTEGER);
    assert(tokens[3].value == "100");
    
    std::cout << "✓ Numeric literals test passed" << std::endl;
}

void testLineAndColumn() {
    std::cout << "Testing line and column tracking..." << std::endl;
    
    Lexer lexer("int x\ny = 5");
    auto tokens = lexer.tokenize();
    
    // int on line 1
    assert(tokens[0].line == 1);
    // x on line 1
    assert(tokens[1].line == 1);
    // Newline token on line 1
    assert(tokens[2].type == TokenType::NEWLINE);
    // y on line 2
    assert(tokens[3].line == 2);
    
    std::cout << "✓ Line and column tracking test passed" << std::endl;
}

void testComments() {
    std::cout << "Testing comment skipping..." << std::endl;
    
    Lexer lexer("int x; // comment\nint y;");
    auto tokens = lexer.tokenize();
    
    // Should skip the comment and properly tokenize
    assert(tokens[0].type == TokenType::INT);
    assert(tokens[1].type == TokenType::IDENTIFIER);
    assert(tokens[1].value == "x");
    assert(tokens[2].type == TokenType::SEMICOLON);
    
    std::cout << "✓ Comments test passed" << std::endl;
}

void testComplexExpression() {
    std::cout << "Testing complex expression..." << std::endl;
    
    Lexer lexer("(a + b) * c / 2");
    auto tokens = lexer.tokenize();
    
    assert(tokens[0].type == TokenType::LPAREN);
    assert(tokens[1].type == TokenType::IDENTIFIER);
    assert(tokens[1].value == "a");
    assert(tokens[2].type == TokenType::PLUS);
    assert(tokens[3].type == TokenType::IDENTIFIER);
    assert(tokens[3].value == "b");
    assert(tokens[4].type == TokenType::RPAREN);
    assert(tokens[5].type == TokenType::STAR);
    assert(tokens[6].type == TokenType::IDENTIFIER);
    assert(tokens[6].value == "c");
    assert(tokens[7].type == TokenType::SLASH);
    assert(tokens[8].type == TokenType::INTEGER);
    assert(tokens[8].value == "2");
    
    std::cout << "✓ Complex expression test passed" << std::endl;
}

void testEscapeSequences() {
    std::cout << "Testing escape sequences..." << std::endl;
    
    Lexer lexer("\"hello\\nworld\\t!\"");
    auto tokens = lexer.tokenize();
    
    assert(tokens[0].type == TokenType::STRING);
    assert(tokens[0].value == "hello\nworld\t!");
    
    std::cout << "✓ Escape sequences test passed" << std::endl;
}

int main() {
    std::cout << "=== LEXER TESTS ===" << std::endl << std::endl;
    
    try {
        testBasicTokens();
        testKeywords();
        testOperators();
        testDelimiters();
        testStrings();
        testNumbers();
        testLineAndColumn();
        testComments();
        testComplexExpression();
        testEscapeSequences();
        
        std::cout << "\n=== ALL TESTS PASSED ===" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n!!! TEST FAILED !!!" << std::endl;
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
