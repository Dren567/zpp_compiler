#include <cassert>
#include <iostream>
#include "../include/lexer.h"
#include "../include/parser.h"

void testSimpleFunction() {
    std::cout << "Testing simple function parsing..." << std::endl;
    
    Lexer lexer("int main() { return 0; }");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    
    assert(program->functions.size() == 1);
    assert(program->functions[0]->name == "main");
    assert(program->functions[0]->returnType == "int");
    assert(program->functions[0]->parameters.size() == 0);
    
    std::cout << "✓ Simple function parsing test passed" << std::endl;
}

void testFunctionWithParameters() {
    std::cout << "Testing function with parameters..." << std::endl;
    
    Lexer lexer("int add(int a, int b) { return a + b; }");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    
    assert(program->functions.size() == 1);
    assert(program->functions[0]->name == "add");
    assert(program->functions[0]->returnType == "int");
    assert(program->functions[0]->parameters.size() == 2);
    assert(program->functions[0]->parameters[0].first == "int");
    assert(program->functions[0]->parameters[0].second == "a");
    assert(program->functions[0]->parameters[1].first == "int");
    assert(program->functions[0]->parameters[1].second == "b");
    
    std::cout << "✓ Function with parameters test passed" << std::endl;
}

void testMultipleFunctions() {
    std::cout << "Testing multiple functions..." << std::endl;
    
    Lexer lexer("int foo() { return 1; } void bar() { }");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    
    assert(program->functions.size() == 2);
    assert(program->functions[0]->name == "foo");
    assert(program->functions[0]->returnType == "int");
    assert(program->functions[1]->name == "bar");
    assert(program->functions[1]->returnType == "void");
    
    std::cout << "✓ Multiple functions test passed" << std::endl;
}

void testReturnStatement() {
    std::cout << "Testing return statement..." << std::endl;
    
    Lexer lexer("int test() { return 42; }");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    
    auto func = program->functions[0];
    auto block = std::dynamic_pointer_cast<BlockStatement>(func->body);
    assert(block != nullptr);
    assert(block->statements.size() == 1);
    
    auto returnStmt = std::dynamic_pointer_cast<ReturnStatement>(block->statements[0]);
    assert(returnStmt != nullptr);
    
    std::cout << "✓ Return statement test passed" << std::endl;
}

void testVariableDeclaration() {
    std::cout << "Testing variable declaration..." << std::endl;
    
    Lexer lexer("int main() { int x = 5; }");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    
    auto func = program->functions[0];
    auto block = std::dynamic_pointer_cast<BlockStatement>(func->body);
    assert(block != nullptr);
    assert(block->statements.size() == 1);
    
    auto varDecl = std::dynamic_pointer_cast<VariableDecl>(block->statements[0]);
    assert(varDecl != nullptr);
    assert(varDecl->type == "int");
    assert(varDecl->name == "x");
    
    std::cout << "✓ Variable declaration test passed" << std::endl;
}

void testIfStatement() {
    std::cout << "Testing if statement..." << std::endl;
    
    Lexer lexer("int main() { if (x > 0) { return 1; } }");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    
    auto func = program->functions[0];
    auto block = std::dynamic_pointer_cast<BlockStatement>(func->body);
    assert(block != nullptr);
    assert(block->statements.size() == 1);
    
    auto ifStmt = std::dynamic_pointer_cast<IfStatement>(block->statements[0]);
    assert(ifStmt != nullptr);
    assert(ifStmt->condition != nullptr);
    assert(ifStmt->thenBranch != nullptr);
    
    std::cout << "✓ If statement test passed" << std::endl;
}

void testIfElseStatement() {
    std::cout << "Testing if-else statement..." << std::endl;
    
    Lexer lexer("int main() { if (x > 0) { return 1; } else { return 0; } }");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    
    auto func = program->functions[0];
    auto block = std::dynamic_pointer_cast<BlockStatement>(func->body);
    
    auto ifStmt = std::dynamic_pointer_cast<IfStatement>(block->statements[0]);
    assert(ifStmt != nullptr);
    assert(ifStmt->elseBranch != nullptr);
    
    std::cout << "✓ If-else statement test passed" << std::endl;
}

void testWhileLoop() {
    std::cout << "Testing while loop..." << std::endl;
    
    Lexer lexer("int main() { while (x < 10) { x = x + 1; } }");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    
    auto func = program->functions[0];
    auto block = std::dynamic_pointer_cast<BlockStatement>(func->body);
    
    auto whileStmt = std::dynamic_pointer_cast<WhileStatement>(block->statements[0]);
    assert(whileStmt != nullptr);
    assert(whileStmt->condition != nullptr);
    assert(whileStmt->body != nullptr);
    
    std::cout << "✓ While loop test passed" << std::endl;
}

void testForLoop() {
    std::cout << "Testing for loop..." << std::endl;
    
    Lexer lexer("int main() { for (int i = 0; i < 10; i = i + 1) { } }");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    
    auto func = program->functions[0];
    auto block = std::dynamic_pointer_cast<BlockStatement>(func->body);
    
    auto forStmt = std::dynamic_pointer_cast<ForStatement>(block->statements[0]);
    assert(forStmt != nullptr);
    assert(forStmt->init != nullptr);
    assert(forStmt->condition != nullptr);
    assert(forStmt->increment != nullptr);
    assert(forStmt->body != nullptr);
    
    std::cout << "✓ For loop test passed" << std::endl;
}

void testBinaryExpression() {
    std::cout << "Testing binary expression..." << std::endl;
    
    Lexer lexer("int main() { return a + b; }");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    
    auto func = program->functions[0];
    auto block = std::dynamic_pointer_cast<BlockStatement>(func->body);
    auto returnStmt = std::dynamic_pointer_cast<ReturnStatement>(block->statements[0]);
    
    auto binOp = std::dynamic_pointer_cast<BinaryOp>(returnStmt->expression);
    assert(binOp != nullptr);
    assert(binOp->op == TokenType::PLUS);
    
    std::cout << "✓ Binary expression test passed" << std::endl;
}

void testUnaryExpression() {
    std::cout << "Testing unary expression..." << std::endl;
    
    Lexer lexer("int main() { return -x; }");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    
    auto func = program->functions[0];
    auto block = std::dynamic_pointer_cast<BlockStatement>(func->body);
    auto returnStmt = std::dynamic_pointer_cast<ReturnStatement>(block->statements[0]);
    
    auto unaryOp = std::dynamic_pointer_cast<UnaryOp>(returnStmt->expression);
    assert(unaryOp != nullptr);
    assert(unaryOp->op == TokenType::MINUS);
    
    std::cout << "✓ Unary expression test passed" << std::endl;
}

void testFunctionCall() {
    std::cout << "Testing function call..." << std::endl;
    
    Lexer lexer("int main() { return add(1, 2); }");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    
    auto func = program->functions[0];
    auto block = std::dynamic_pointer_cast<BlockStatement>(func->body);
    auto returnStmt = std::dynamic_pointer_cast<ReturnStatement>(block->statements[0]);
    
    auto funcCall = std::dynamic_pointer_cast<FunctionCall>(returnStmt->expression);
    assert(funcCall != nullptr);
    assert(funcCall->name == "add");
    assert(funcCall->arguments.size() == 2);
    
    std::cout << "✓ Function call test passed" << std::endl;
}

void testAssignment() {
    std::cout << "Testing assignment..." << std::endl;
    
    Lexer lexer("int main() { x = 42; }");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    
    auto func = program->functions[0];
    auto block = std::dynamic_pointer_cast<BlockStatement>(func->body);
    auto exprStmt = std::dynamic_pointer_cast<ExpressionStatement>(block->statements[0]);
    
    auto assignment = std::dynamic_pointer_cast<Assignment>(exprStmt->expression);
    assert(assignment != nullptr);
    assert(assignment->name == "x");
    
    std::cout << "✓ Assignment test passed" << std::endl;
}

void testOperatorPrecedence() {
    std::cout << "Testing operator precedence..." << std::endl;
    
    Lexer lexer("int main() { return a + b * c; }");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    
    auto func = program->functions[0];
    auto block = std::dynamic_pointer_cast<BlockStatement>(func->body);
    auto returnStmt = std::dynamic_pointer_cast<ReturnStatement>(block->statements[0]);
    
    // Should be a + (b * c)
    auto addOp = std::dynamic_pointer_cast<BinaryOp>(returnStmt->expression);
    assert(addOp != nullptr);
    assert(addOp->op == TokenType::PLUS);
    
    auto mulOp = std::dynamic_pointer_cast<BinaryOp>(addOp->right);
    assert(mulOp != nullptr);
    assert(mulOp->op == TokenType::STAR);
    
    std::cout << "✓ Operator precedence test passed" << std::endl;
}

void testArrayAccess() {
    std::cout << "Testing array access..." << std::endl;
    
    Lexer lexer("int main() { return arr[0]; }");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    
    auto func = program->functions[0];
    auto block = std::dynamic_pointer_cast<BlockStatement>(func->body);
    auto returnStmt = std::dynamic_pointer_cast<ReturnStatement>(block->statements[0]);
    
    auto arrayAccess = std::dynamic_pointer_cast<ArrayAccess>(returnStmt->expression);
    assert(arrayAccess != nullptr);
    
    std::cout << "✓ Array access test passed" << std::endl;
}

int main() {
    std::cout << "=== PARSER TESTS ===" << std::endl << std::endl;
    
    try {
        testSimpleFunction();
        testFunctionWithParameters();
        testMultipleFunctions();
        testReturnStatement();
        testVariableDeclaration();
        testIfStatement();
        testIfElseStatement();
        testWhileLoop();
        testForLoop();
        testBinaryExpression();
        testUnaryExpression();
        testFunctionCall();
        testAssignment();
        testOperatorPrecedence();
        testArrayAccess();
        
        std::cout << "\n=== ALL TESTS PASSED ===" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n!!! TEST FAILED !!!" << std::endl;
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
