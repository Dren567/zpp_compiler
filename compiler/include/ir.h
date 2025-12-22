#ifndef IR_H
#define IR_H

#include "parser.h"
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

// IROpCode enum: Represents all intermediate representation operation codes
// These are the bytecode instructions that the interpreter executes
enum class IROpCode {
    // Arithmetic
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    NEG,
    CONCAT,  // String concatenation

    // Logical
    AND,
    OR,
    NOT,

    // Comparison
    EQ,
    NE,
    LT,
    GT,
    LE,
    GE,

    // Control flow
    JMP,
    JZ,      // Jump if zero
    JNZ,     // Jump if not zero
    CALL,
    RET,

    // Memory
    LOAD,
    STORE,
    LOAD_GLOBAL,
    STORE_GLOBAL,

    // Literals
    LOAD_INT,
    LOAD_FLOAT,
    LOAD_STRING,

    // Other
    PRINT,
    INPUT,
    KEY_PRESSED,
    SCREEN,
    // Graphics operations
    DRAW_PIXEL,
    DRAW_RECT,
    DRAW_LINE,
    DRAW_CIRCLE,
    CLEAR_SCREEN,
    PRESENT,
    LABEL,
    NOP
};

struct IRValue {
    enum class Type {
        TEMP,        // Temporary variable (t0, t1, ...)
        GLOBAL,      // Global variable
        LOCAL,       // Local variable
        CONSTANT,    // Literal constant
        LABEL        // Label
    };
    
    Type type;
    std::string name;
    int id;  // For temp variables, their ID
    
    IRValue(Type t = Type::TEMP, const std::string& n = "", int i = -1)
        : type(t), name(n), id(i) {}
    
    std::string toString() const;
};

struct IRInstruction {
    IROpCode opcode;
    std::vector<IRValue> operands;
    IRValue result;
    std::string label;  // For LABEL instructions
    std::string prompt; // For INPUT instructions
    
    IRInstruction(IROpCode op) : opcode(op) {}
    
    std::string toString() const;
};

struct IRFunction {
    std::string name;
    std::string returnType;
    std::vector<std::pair<std::string, std::string>> parameters;
    std::vector<IRInstruction> instructions;
};

struct IRProgram {
    std::vector<IRFunction> functions;
    std::unordered_map<std::string, std::string> globalVariables;
};

class IRGenerator {
public:
    explicit IRGenerator(const ProgramPtr& ast);
    
    IRProgram generate();
    
private:
    ProgramPtr ast;
    IRProgram program;
    IRFunction* currentFunction;
    int tempCounter;
    int labelCounter;
    std::unordered_map<std::string, IRValue> symbolTable;
    
    // Visitor methods for AST nodes
    void visitProgram(const ProgramPtr& program);
    void visitFunction(const FunctionDeclPtr& func);
    void visitStatement(const StatementPtr& stmt);
    void visitBlockStatement(const std::shared_ptr<BlockStatement>& block);
    void visitReturnStatement(const std::shared_ptr<ReturnStatement>& ret);
    void visitIfStatement(const std::shared_ptr<IfStatement>& ifStmt);
    void visitWhileStatement(const std::shared_ptr<WhileStatement>& whileStmt);
    void visitForStatement(const std::shared_ptr<ForStatement>& forStmt);
    void visitVariableDecl(const std::shared_ptr<VariableDecl>& varDecl);
    void visitExpressionStatement(const std::shared_ptr<ExpressionStatement>& exprStmt);
    
    IRValue visitExpression(const ExpressionPtr& expr);
    IRValue visitBinaryOp(const std::shared_ptr<BinaryOp>& binOp);
    IRValue visitUnaryOp(const std::shared_ptr<UnaryOp>& unaryOp);
    IRValue visitLiteral(const std::shared_ptr<Literal>& lit);
    IRValue visitIdentifier(const std::shared_ptr<Identifier>& id);
    IRValue visitFunctionCall(const std::shared_ptr<FunctionCall>& call);
    IRValue visitAssignment(const std::shared_ptr<Assignment>& assign);
    IRValue visitArrayAccess(const std::shared_ptr<ArrayAccess>& access);
    
    // Helper methods
    IRValue createTemp();
    std::string createLabel();
    void emitInstruction(const IRInstruction& instr);
    IROpCode tokenTypeToOpCode(TokenType type);
};

// Utility functions
std::string opCodeToString(IROpCode opcode);

#endif // IR_H
