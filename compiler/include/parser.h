#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <vector>
#include <memory>
#include <stdexcept>

// Forward declarations
struct ASTNode;
struct Program;
struct FunctionDecl;
struct Statement;
struct Expression;
struct BlockStatement;
struct ReturnStatement;
struct IfStatement;
struct WhileStatement;
struct ForStatement;
struct ExpressionStatement;
struct VariableDecl;
struct BinaryOp;
struct UnaryOp;
struct FunctionCall;
struct Literal;
struct Identifier;

using ASTNodePtr = std::shared_ptr<ASTNode>;
using ProgramPtr = std::shared_ptr<Program>;
using FunctionDeclPtr = std::shared_ptr<FunctionDecl>;
using StatementPtr = std::shared_ptr<Statement>;
using ExpressionPtr = std::shared_ptr<Expression>;

// Base AST Node
struct ASTNode {
    virtual ~ASTNode() = default;
};

// Expressions
struct Expression : public ASTNode {
    virtual ~Expression() = default;
};

struct Literal : public Expression {
    TokenType type;
    std::string value;
    
    Literal(TokenType t, const std::string& v) : type(t), value(v) {}
};

struct Identifier : public Expression {
    std::string name;
    
    explicit Identifier(const std::string& n) : name(n) {}
};

struct BinaryOp : public Expression {
    ExpressionPtr left;
    TokenType op;
    ExpressionPtr right;
    
    BinaryOp(ExpressionPtr l, TokenType o, ExpressionPtr r)
        : left(l), op(o), right(r) {}
};

struct UnaryOp : public Expression {
    TokenType op;
    ExpressionPtr operand;
    
    UnaryOp(TokenType o, ExpressionPtr expr)
        : op(o), operand(expr) {}
};


struct FunctionCall : public Expression {
    std::string name;
    std::vector<ExpressionPtr> arguments;
    FunctionCall(const std::string& n) : name(n) {}
    FunctionCall(const std::string& n, const std::vector<ExpressionPtr>& args) : name(n), arguments(args) {}
};

struct InputCall : public Expression {
    ExpressionPtr prompt; // optional prompt expression (usually a string literal)
    InputCall() = default;
    explicit InputCall(ExpressionPtr p) : prompt(p) {}
};

struct KeyPressedCall : public Expression {
    ExpressionPtr prompt; // optional prompt expression
    KeyPressedCall() = default;
    explicit KeyPressedCall(ExpressionPtr p) : prompt(p) {}
};

struct ArrayAccess : public Expression {
    ExpressionPtr array;
    ExpressionPtr index;
    
    ArrayAccess(ExpressionPtr arr, ExpressionPtr idx)
        : array(arr), index(idx) {}
};

struct Assignment : public Expression {
    std::string name;
    ExpressionPtr value;
    
    Assignment(const std::string& n, ExpressionPtr v)
        : name(n), value(v) {}
};

// Statements
struct Statement : public ASTNode {
    virtual ~Statement() = default;
};

struct ExpressionStatement : public Statement {
    ExpressionPtr expression;
    
    explicit ExpressionStatement(ExpressionPtr expr) : expression(expr) {}
};

struct PrintStatement : public Statement {
    ExpressionPtr expression;
    explicit PrintStatement(ExpressionPtr expr) : expression(expr) {}
};

struct BlockStatement : public Statement {
    std::vector<StatementPtr> statements;
};

struct ReturnStatement : public Statement {
    ExpressionPtr expression;
    
    explicit ReturnStatement(ExpressionPtr expr = nullptr) : expression(expr) {}
};

struct IfStatement : public Statement {
    ExpressionPtr condition;
    StatementPtr thenBranch;
    StatementPtr elseBranch;
    
    IfStatement(ExpressionPtr cond, StatementPtr then, StatementPtr els = nullptr)
        : condition(cond), thenBranch(then), elseBranch(els) {}
};

struct WhileStatement : public Statement {
    ExpressionPtr condition;
    StatementPtr body;
    
    WhileStatement(ExpressionPtr cond, StatementPtr b)
        : condition(cond), body(b) {}
};

struct ForStatement : public Statement {
    StatementPtr init;
    ExpressionPtr condition;
    ExpressionPtr increment;
    StatementPtr body;
    
    ForStatement(StatementPtr i, ExpressionPtr c, ExpressionPtr inc, StatementPtr b)
        : init(i), condition(c), increment(inc), body(b) {}
};

struct VariableDecl : public Statement {
    std::string name;
    std::string type;
    ExpressionPtr initializer;

    VariableDecl(const std::string& n, const std::string& t, ExpressionPtr init = nullptr)
        : name(n), type(t), initializer(init) {}
};

// Functions and Program
struct FunctionDecl : public ASTNode {
    std::string returnType;
    std::string name;
    std::vector<std::pair<std::string, std::string>> parameters; // type, name pairs
    StatementPtr body;
    
    FunctionDecl(const std::string& rt, const std::string& n)
        : returnType(rt), name(n) {}
};

struct Program : public ASTNode {
    std::vector<FunctionDeclPtr> functions;
};

// Parser class
class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    
    ProgramPtr parse();
    
private:
    std::vector<Token> tokens;
    size_t current;

    // Token management
    Token currentToken();
    Token peekToken(int offset = 1);
    Token advance();
    bool check(TokenType type);
    bool match(const std::vector<TokenType>& types);
    void consume(TokenType type, const std::string& message);

    // Parsing methods
    FunctionDeclPtr parseFunction();
    StatementPtr parseStatement();
    StatementPtr parseBlockStatement();
    StatementPtr parseReturnStatement();
    StatementPtr parseIfStatement();
    StatementPtr parseWhileStatement();
    StatementPtr parseForStatement();
    StatementPtr parseVariableDeclaration();
    StatementPtr parsePrintStatement();
    StatementPtr parseExpressionStatement();

    // Expression parsing with precedence climbing
    ExpressionPtr parseExpression();
    ExpressionPtr parseComma();
    ExpressionPtr parseAssignment();
    ExpressionPtr parseLogicalOr();
    ExpressionPtr parseLogicalAnd();
    ExpressionPtr parseEquality();
    ExpressionPtr parseComparison();
    ExpressionPtr parseAdditive();
    ExpressionPtr parseMultiplicative();
    ExpressionPtr parseUnary();
    ExpressionPtr parsePostfix();
    ExpressionPtr parsePrimary();

    // Utility
    bool isType(TokenType type);
    std::string tokenTypeToString(TokenType type);
};

#endif // PARSER_H
