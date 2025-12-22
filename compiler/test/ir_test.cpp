#include <cassert>
#include <iostream>
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/ir.h"

void testBasicIRGeneration() {
    std::cout << "Testing basic IR generation..." << std::endl;
    
    std::string source = R"(
        int main() {
            return 42;
        }
    )";
    
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();
    
    IRGenerator irGen(ast);
    auto ir = irGen.generate();
    
    assert(ir.functions.size() == 1);
    assert(ir.functions[0].name == "main");
    assert(ir.functions[0].instructions.size() >= 2);
    
    std::cout << "✓ Basic IR generation test passed" << std::endl;
}

void testBinaryOperations() {
    std::cout << "Testing binary operations in IR..." << std::endl;
    
    std::string source = R"(
        int add(int a, int b) {
            return a + b;
        }
    )";
    
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();
    
    IRGenerator irGen(ast);
    auto ir = irGen.generate();
    
    assert(ir.functions.size() == 1);
    assert(ir.functions[0].name == "add");
    
    // Check for ADD instruction
    bool hasAdd = false;
    for (const auto& instr : ir.functions[0].instructions) {
        if (instr.opcode == IROpCode::ADD) {
            hasAdd = true;
            break;
        }
    }
    assert(hasAdd);
    
    std::cout << "✓ Binary operations test passed" << std::endl;
}

void testVariableDeclaration() {
    std::cout << "Testing variable declaration in IR..." << std::endl;
    
    std::string source = R"(
        int main() {
            int x = 5;
            return x;
        }
    )";
    
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();
    
    IRGenerator irGen(ast);
    auto ir = irGen.generate();
    
    assert(ir.functions.size() == 1);
    
    // Check for STORE instruction (variable assignment)
    bool hasStore = false;
    for (const auto& instr : ir.functions[0].instructions) {
        if (instr.opcode == IROpCode::STORE) {
            hasStore = true;
            break;
        }
    }
    assert(hasStore);
    
    std::cout << "✓ Variable declaration test passed" << std::endl;
}

void testIfStatement() {
    std::cout << "Testing if statement in IR..." << std::endl;
    
    std::string source = R"(
        int main() {
            int x = 5;
            if (x > 0) {
                return 1;
            }
            return 0;
        }
    )";
    
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();
    
    IRGenerator irGen(ast);
    auto ir = irGen.generate();
    
    assert(ir.functions.size() == 1);
    
    // Check for comparison and jump instructions
    bool hasComparison = false;
    bool hasJump = false;
    for (const auto& instr : ir.functions[0].instructions) {
        if (instr.opcode == IROpCode::GT) {
            hasComparison = true;
        }
        if (instr.opcode == IROpCode::JZ || instr.opcode == IROpCode::JMP) {
            hasJump = true;
        }
    }
    assert(hasComparison);
    assert(hasJump);
    
    std::cout << "✓ If statement test passed" << std::endl;
}

void testWhileLoop() {
    std::cout << "Testing while loop in IR..." << std::endl;
    
    std::string source = R"(
        int main() {
            int i = 0;
            while (i < 10) {
                i = i + 1;
            }
            return i;
        }
    )";
    
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();
    
    IRGenerator irGen(ast);
    auto ir = irGen.generate();
    
    assert(ir.functions.size() == 1);
    
    // Check for labels and jumps (loop structure)
    int labelCount = 0;
    int jumpCount = 0;
    for (const auto& instr : ir.functions[0].instructions) {
        if (instr.opcode == IROpCode::LABEL) {
            labelCount++;
        }
        if (instr.opcode == IROpCode::JMP || instr.opcode == IROpCode::JZ) {
            jumpCount++;
        }
    }
    assert(labelCount >= 2);  // At least loop start and end labels
    assert(jumpCount >= 2);   // Back jump and conditional jump
    
    std::cout << "✓ While loop test passed" << std::endl;
}

void testForLoop() {
    std::cout << "Testing for loop in IR..." << std::endl;
    
    std::string source = R"(
        int main() {
            for (int i = 0; i < 10; i = i + 1) {
                return i;
            }
            return 0;
        }
    )";
    
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();
    
    IRGenerator irGen(ast);
    auto ir = irGen.generate();
    
    assert(ir.functions.size() == 1);
    
    // Should have labels for loop control flow
    bool hasLabels = false;
    for (const auto& instr : ir.functions[0].instructions) {
        if (instr.opcode == IROpCode::LABEL) {
            hasLabels = true;
            break;
        }
    }
    assert(hasLabels);
    
    std::cout << "✓ For loop test passed" << std::endl;
}

void testFunctionCall() {
    std::cout << "Testing function call in IR..." << std::endl;
    
    std::string source = R"(
        int main() {
            return add(3, 5);
        }
        
        int add(int a, int b) {
            return a + b;
        }
    )";
    
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();
    
    IRGenerator irGen(ast);
    auto ir = irGen.generate();
    
    assert(ir.functions.size() == 2);
    
    // Check for CALL instruction in main
    bool hasCall = false;
    for (const auto& instr : ir.functions[0].instructions) {
        if (instr.opcode == IROpCode::CALL) {
            hasCall = true;
            assert(instr.label == "add");
            break;
        }
    }
    assert(hasCall);
    
    std::cout << "✓ Function call test passed" << std::endl;
}

void testUnaryOperations() {
    std::cout << "Testing unary operations in IR..." << std::endl;
    
    std::string source = R"(
        int main() {
            int x = 5;
            return -5;
        }
    )";
    
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();
    
    IRGenerator irGen(ast);
    auto ir = irGen.generate();
    
    assert(ir.functions.size() == 1);
    
    // The unary negation is applied to a literal, check that SUB or NEG appears
    bool hasNegOrSub = false;
    for (const auto& instr : ir.functions[0].instructions) {
        if (instr.opcode == IROpCode::NEG || instr.opcode == IROpCode::SUB) {
            hasNegOrSub = true;
            break;
        }
    }
    assert(hasNegOrSub);
    
    std::cout << "✓ Unary operations test passed" << std::endl;
}

void testIRInstructionToString() {
    std::cout << "Testing IR instruction string conversion..." << std::endl;
    
    // Test various instruction types
    IRInstruction add(IROpCode::ADD);
    add.operands.push_back(IRValue(IRValue::Type::TEMP, "t0", 0));
    add.operands.push_back(IRValue(IRValue::Type::TEMP, "t1", 1));
    add.result = IRValue(IRValue::Type::TEMP, "t2", 2);
    
    std::string str = add.toString();
    assert(str.find("ADD") != std::string::npos);
    assert(str.find("t0") != std::string::npos);
    
    // Test label
    IRInstruction label(IROpCode::LABEL);
    label.label = "L0";
    std::string labelStr = label.toString();
    assert(labelStr.find("LABEL") != std::string::npos);
    
    std::cout << "✓ IR instruction string conversion test passed" << std::endl;
}

void testComplexExpression() {
    std::cout << "Testing complex expression in IR..." << std::endl;
    
    std::string source = R"(
        int main() {
            return 2 + 3 * 4;
        }
    )";
    
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();
    
    IRGenerator irGen(ast);
    auto ir = irGen.generate();
    
    assert(ir.functions.size() == 1);
    
    // Should have both ADD and MUL instructions
    bool hasMul = false;
    bool hasAdd = false;
    for (const auto& instr : ir.functions[0].instructions) {
        if (instr.opcode == IROpCode::MUL) hasMul = true;
        if (instr.opcode == IROpCode::ADD) hasAdd = true;
    }
    assert(hasMul);
    assert(hasAdd);
    
    std::cout << "✓ Complex expression test passed" << std::endl;
}

void testMultipleFunctions() {
    std::cout << "Testing multiple functions in IR..." << std::endl;
    
    std::string source = R"(
        int foo() { return 1; }
        int bar() { return 2; }
        int main() { return 3; }
    )";
    
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();
    
    IRGenerator irGen(ast);
    auto ir = irGen.generate();
    
    assert(ir.functions.size() == 3);
    assert(ir.functions[0].name == "foo");
    assert(ir.functions[1].name == "bar");
    assert(ir.functions[2].name == "main");
    
    std::cout << "✓ Multiple functions test passed" << std::endl;
}

int main() {
    std::cout << "=== IR GENERATOR TESTS ===" << std::endl << std::endl;
    
    try {
        testBasicIRGeneration();
        testBinaryOperations();
        testVariableDeclaration();
        testIfStatement();
        testWhileLoop();
        testForLoop();
        testFunctionCall();
        testUnaryOperations();
        testIRInstructionToString();
        testComplexExpression();
        testMultipleFunctions();
        
        std::cout << "\n=== ALL TESTS PASSED ===" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n!!! TEST FAILED !!!" << std::endl;
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
