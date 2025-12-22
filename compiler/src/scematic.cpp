#include "scematic.h"
#include <iostream>

bool SemanticAnalyzer::errors = false;

// Scope implementation
void Scope::declare(const std::string& name, const Symbol& symbol) {
    if (symbols.find(name) != symbols.end()) {
        throw std::runtime_error("Symbol '" + name + "' already declared in current scope");
    }
    symbols[name] = symbol;
}

Symbol* Scope::lookup(const std::string& name) {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        return &it->second;
    }
    
    if (parent != nullptr) {
        return parent->lookup(name);
    }
    
    return nullptr;
}

Symbol* Scope::lookupLocal(const std::string& name) {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        return &it->second;
    }
    return nullptr;
}

// SemanticAnalyzer implementation
SemanticAnalyzer::SemanticAnalyzer(const ProgramPtr& ast)
    : ast(ast), currentScope(nullptr), globalScope(nullptr), 
      currentFunctionReturnType("void") {}

void SemanticAnalyzer::analyze() {
    globalScope = new Scope();
    currentScope = globalScope;
    
    try {
        analyzeProgram(ast);
    } catch (const std::exception& e) {
        reportError(e.what());
    }
}

void SemanticAnalyzer::analyzeProgram(const ProgramPtr& program) {
    if (!program) return;
    
    // First pass: collect function declarations
    for (const auto& func : program->functions) {
        Symbol funcSymbol(func->name, func->returnType, true, true);
        try {
            currentScope->declare(func->name, funcSymbol);
        } catch (const std::exception& e) {
            reportError(e.what());
        }
    }
    
    // Second pass: analyze function bodies
    for (const auto& func : program->functions) {
        analyzeFunction(func);
    }
}

void SemanticAnalyzer::analyzeFunction(const FunctionDeclPtr& func) {
    if (!func) return;
    
    currentFunctionReturnType = func->returnType;
    enterScope();
    
    // Add parameters to scope
    for (const auto& param : func->parameters) {
        Symbol paramSymbol(param.second, param.first, false, true);
        try {
            currentScope->declare(param.second, paramSymbol);
        } catch (const std::exception& e) {
            reportError(e.what());
        }
    }
    
    // Analyze function body
    analyzeStatement(func->body);
    
    exitScope();
}

void SemanticAnalyzer::analyzeStatement(const StatementPtr& stmt) {
    if (!stmt) return;
    
    if (auto block = std::dynamic_pointer_cast<BlockStatement>(stmt)) {
        analyzeBlockStatement(block);
    } else if (auto ret = std::dynamic_pointer_cast<ReturnStatement>(stmt)) {
        analyzeReturnStatement(ret);
    } else if (auto ifStmt = std::dynamic_pointer_cast<IfStatement>(stmt)) {
        analyzeIfStatement(ifStmt);
    } else if (auto whileStmt = std::dynamic_pointer_cast<WhileStatement>(stmt)) {
        analyzeWhileStatement(whileStmt);
    } else if (auto forStmt = std::dynamic_pointer_cast<ForStatement>(stmt)) {
        analyzeForStatement(forStmt);
    } else if (auto varDecl = std::dynamic_pointer_cast<VariableDecl>(stmt)) {
        analyzeVariableDecl(varDecl);
    } else if (auto exprStmt = std::dynamic_pointer_cast<ExpressionStatement>(stmt)) {
        analyzeExpressionStatement(exprStmt);
    }
}

void SemanticAnalyzer::analyzeBlockStatement(const std::shared_ptr<BlockStatement>& block) {
    if (!block) return;
    
    for (const auto& stmt : block->statements) {
        analyzeStatement(stmt);
    }
}

void SemanticAnalyzer::analyzeReturnStatement(const std::shared_ptr<ReturnStatement>& ret) {
    if (!ret) return;
    
    if (ret->expression) {
        std::string exprType = analyzeExpression(ret->expression);
        
        if (!isCompatibleType(exprType, currentFunctionReturnType)) {
            reportError("Return type mismatch: expected " + currentFunctionReturnType + 
                       ", got " + exprType);
        }
    }
}

void SemanticAnalyzer::analyzeIfStatement(const std::shared_ptr<IfStatement>& ifStmt) {
    if (!ifStmt) return;
    
    // Analyze condition
    std::string condType = analyzeExpression(ifStmt->condition);
    
    // Condition should be boolean-like (int, string, etc.)
    
    // Analyze branches
    analyzeStatement(ifStmt->thenBranch);
    if (ifStmt->elseBranch) {
        analyzeStatement(ifStmt->elseBranch);
    }
}

void SemanticAnalyzer::analyzeWhileStatement(const std::shared_ptr<WhileStatement>& whileStmt) {
    if (!whileStmt) return;
    
    // Analyze condition
    std::string condType = analyzeExpression(whileStmt->condition);
    
    // Analyze body
    analyzeStatement(whileStmt->body);
}

void SemanticAnalyzer::analyzeForStatement(const std::shared_ptr<ForStatement>& forStmt) {
    if (!forStmt) return;
    
    enterScope();
    
    // Analyze init
    if (forStmt->init) {
        analyzeStatement(forStmt->init);
    }
    
    // Analyze condition
    if (forStmt->condition) {
        analyzeExpression(forStmt->condition);
    }
    
    // Analyze increment
    if (forStmt->increment) {
        analyzeExpression(forStmt->increment);
    }
    
    // Analyze body
    analyzeStatement(forStmt->body);
    
    exitScope();
}

void SemanticAnalyzer::analyzeVariableDecl(const std::shared_ptr<VariableDecl>& varDecl) {
    if (!varDecl) return;
    
    if (varDecl->initializer) {
        std::string exprType = analyzeExpression(varDecl->initializer);
        
        if (!isCompatibleType(exprType, varDecl->type)) {
            reportError("Variable initialization type mismatch: expected " + varDecl->type + 
                       ", got " + exprType);
        }
    }
    
    // Declare variable in current scope
    Symbol varSymbol(varDecl->name, varDecl->type, false, true);
    try {
        currentScope->declare(varDecl->name, varSymbol);
    } catch (const std::exception& e) {
        reportError(e.what());
    }
}

void SemanticAnalyzer::analyzeExpressionStatement(const std::shared_ptr<ExpressionStatement>& exprStmt) {
    if (!exprStmt) return;
    
    analyzeExpression(exprStmt->expression);
}

std::string SemanticAnalyzer::analyzeExpression(const ExpressionPtr& expr) {
    if (!expr) return "void";
    
    if (auto binOp = std::dynamic_pointer_cast<BinaryOp>(expr)) {
        return analyzeBinaryOp(binOp);
    } else if (auto unaryOp = std::dynamic_pointer_cast<UnaryOp>(expr)) {
        return analyzeUnaryOp(unaryOp);
    } else if (auto lit = std::dynamic_pointer_cast<Literal>(expr)) {
        return analyzeLiteral(lit);
    } else if (auto id = std::dynamic_pointer_cast<Identifier>(expr)) {
        return analyzeIdentifier(id);
    } else if (auto call = std::dynamic_pointer_cast<FunctionCall>(expr)) {
        return analyzeFunctionCall(call);
    } else if (auto assign = std::dynamic_pointer_cast<Assignment>(expr)) {
        return analyzeAssignment(assign);
    } else if (auto access = std::dynamic_pointer_cast<ArrayAccess>(expr)) {
        return analyzeArrayAccess(access);
    }
    
    return "void";
}

std::string SemanticAnalyzer::analyzeBinaryOp(const std::shared_ptr<BinaryOp>& binOp) {
    if (!binOp) return "void";
    
    std::string leftType = analyzeExpression(binOp->left);
    std::string rightType = analyzeExpression(binOp->right);
    
    // Type checking logic based on operator
    switch (binOp->op) {
        case TokenType::PLUS:
        case TokenType::MINUS:
        case TokenType::STAR:
        case TokenType::SLASH:
        case TokenType::PERCENT:
            return getCommonType(leftType, rightType);
        
        case TokenType::EQUAL:
        case TokenType::NOT_EQUAL:
        case TokenType::LESS:
        case TokenType::GREATER:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER_EQUAL:
            return "int";  // Comparison returns int (0 or 1)
        
        case TokenType::AND:
        case TokenType::OR:
            return "int";  // Logical returns int
        
        case TokenType::COMMA:
            return rightType;  // Comma operator returns right operand type
        
        default:
            return "void";
    }
}

std::string SemanticAnalyzer::analyzeUnaryOp(const std::shared_ptr<UnaryOp>& unaryOp) {
    if (!unaryOp) return "void";
    
    std::string operandType = analyzeExpression(unaryOp->operand);
    
    switch (unaryOp->op) {
        case TokenType::MINUS:
        case TokenType::NOT:
            return operandType;
        default:
            return "void";
    }
}

std::string SemanticAnalyzer::analyzeLiteral(const std::shared_ptr<Literal>& lit) {
    if (!lit) return "void";
    
    switch (lit->type) {
        case TokenType::INTEGER:
            return "int";
        case TokenType::FLOAT:
            return "float";
        case TokenType::STRING:
            return "string";
        default:
            return "void";
    }
}

std::string SemanticAnalyzer::analyzeIdentifier(const std::shared_ptr<Identifier>& id) {
    if (!id) return "void";
    
    Symbol* symbol = currentScope->lookup(id->name);
    if (!symbol) {
        reportError("Undefined identifier: " + id->name);
        return "void";
    }
    
    return symbol->type;
}

std::string SemanticAnalyzer::analyzeFunctionCall(const std::shared_ptr<FunctionCall>& call) {
    if (!call) return "void";
    
    Symbol* symbol = currentScope->lookup(call->name);
    if (!symbol) {
        reportError("Undefined function: " + call->name);
        return "void";
    }
    
    if (!symbol->isFunction) {
        reportError("'" + call->name + "' is not a function");
        return "void";
    }
    
    // Analyze arguments
    for (const auto& arg : call->arguments) {
        analyzeExpression(arg);
    }
    
    return symbol->type;
}

std::string SemanticAnalyzer::analyzeAssignment(const std::shared_ptr<Assignment>& assign) {
    if (!assign) return "void";
    
    Symbol* symbol = currentScope->lookup(assign->name);
    if (!symbol) {
        reportError("Undefined variable: " + assign->name);
        return "void";
    }
    
    std::string exprType = analyzeExpression(assign->value);
    
    if (!isCompatibleType(exprType, symbol->type)) {
        reportError("Assignment type mismatch: '" + assign->name + "' expects " + 
                   symbol->type + ", got " + exprType);
    }
    
    return symbol->type;
}

std::string SemanticAnalyzer::analyzeArrayAccess(const std::shared_ptr<ArrayAccess>& access) {
    if (!access) return "void";
    
    std::string arrayType = analyzeExpression(access->array);
    std::string indexType = analyzeExpression(access->index);
    
    // For now, assume array access returns the element type
    // This would need proper array type handling in the future
    return arrayType;
}

bool SemanticAnalyzer::isCompatibleType(const std::string& from, const std::string& to) {
    if (from == to) return true;
    
    // Allow implicit conversions
    if ((from == "int" || from == "float") && (to == "int" || to == "float")) {
        return true;
    }
    if ((from == "int" || from == "string") && (to == "int" || to == "string")) {
        return true;
    }
    if ((from == "bool" || from == "int") && (to == "bool" || to == "int")) {
        return true;
    }
    
    return false;
}

std::string SemanticAnalyzer::getCommonType(const std::string& type1, const std::string& type2) {
    if (type1 == type2) return type1;
    
    // Promote to float if either is float
    if (type1 == "float" || type2 == "float") {
        return "float";
    }
    
    // Default to left operand type
    return type1;
}

void SemanticAnalyzer::enterScope() {
    Scope* newScope = new Scope(currentScope);
    currentScope = newScope;
}

void SemanticAnalyzer::exitScope() {
    if (currentScope && currentScope != globalScope) {
        Scope* temp = currentScope;
        currentScope = currentScope->getParent();
        delete temp;
    }
}

void SemanticAnalyzer::reportError(const std::string& message) {
    std::cerr << "Semantic Error: " << message << std::endl;
    errors = true;
}

bool SemanticAnalyzer::hasErrors() {
    return errors;
}
