#ifndef SCEMATIC_H
#define SCEMATIC_H

#include "parser.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

// Symbol struct: Represents a declared variable or function
struct Symbol {
    std::string name;      // Symbol name
    std::string type;      // Type (int, float, string, etc.)
    bool isFunction;       // Is this a function?
    bool isDeclared;       // Has this symbol been declared?
    
    Symbol(const std::string& n = "", const std::string& t = "", bool isFunc = false, bool decl = false)
        : name(n), type(t), isFunction(isFunc), isDeclared(decl) {}
};

// Scope class: Manages symbol table for a scope level
// Supports nested scopes (functions, blocks, etc.)
class Scope {
public:
    Scope(Scope* parent = nullptr) : parent(parent) {}
    
    void declare(const std::string& name, const Symbol& symbol);  // Add symbol to this scope
    Symbol* lookup(const std::string& name);                      // Look up symbol (searches parent scopes)
    Symbol* lookupLocal(const std::string& name);                 // Look up only in this scope
    Scope* getParent() const { return parent; }
    
private:
    std::unordered_map<std::string, Symbol> symbols;  // Symbols in this scope
    Scope* parent;                                     // Parent scope (for nested scopes)
};

// SemanticAnalyzer class: Second pass analysis for semantic checking
// Validates type compatibility, variable declarations, function calls, etc.
class SemanticAnalyzer {
public:
    // Constructor: Initialize analyzer with AST
    explicit SemanticAnalyzer(const ProgramPtr& ast);
    
    // analyze(): Main entry point - walk AST and check semantics
    void analyze();
    
    // Error reporting (static for easy access)
    static void reportError(const std::string& message);  // Report an error
    static bool hasErrors();                              // Check if any errors occurred
    
private:
    ProgramPtr ast;                           // Input: Abstract Syntax Tree
    Scope* currentScope;                      // Current scope being analyzed
    Scope* globalScope;                       // Global scope
    std::string currentFunctionReturnType;    // Return type of current function
    static bool errors;                       // Flag: any errors occurred?
    
    // AST traversal methods - validate nodes
    void analyzeProgram(const ProgramPtr& program);
    void analyzeFunction(const FunctionDeclPtr& func);
    void analyzeStatement(const StatementPtr& stmt);
    void analyzeBlockStatement(const std::shared_ptr<BlockStatement>& block);
    void analyzeReturnStatement(const std::shared_ptr<ReturnStatement>& ret);
    void analyzeIfStatement(const std::shared_ptr<IfStatement>& ifStmt);
    void analyzeWhileStatement(const std::shared_ptr<WhileStatement>& whileStmt);
    void analyzeForStatement(const std::shared_ptr<ForStatement>& forStmt);
    void analyzeVariableDecl(const std::shared_ptr<VariableDecl>& varDecl);
    void analyzeExpressionStatement(const std::shared_ptr<ExpressionStatement>& exprStmt);
    
    // Expression type checking - return type of expression
    std::string analyzeExpression(const ExpressionPtr& expr);
    std::string analyzeBinaryOp(const std::shared_ptr<BinaryOp>& binOp);
    std::string analyzeUnaryOp(const std::shared_ptr<UnaryOp>& unaryOp);
    std::string analyzeLiteral(const std::shared_ptr<Literal>& lit);
    std::string analyzeIdentifier(const std::shared_ptr<Identifier>& id);
    std::string analyzeFunctionCall(const std::shared_ptr<FunctionCall>& call);
    std::string analyzeAssignment(const std::shared_ptr<Assignment>& assign);
    std::string analyzeArrayAccess(const std::shared_ptr<ArrayAccess>& access);
    
    // Helper methods for type checking and scope management
    bool isCompatibleType(const std::string& from, const std::string& to);  // Can we convert from -> to?
    std::string getCommonType(const std::string& type1, const std::string& type2);  // Find common type
    void enterScope();   // Enter a new nested scope
    void exitScope();    // Exit to parent scope
};


#endif // SCEMATIC_H
