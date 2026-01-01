#include "ir.h" 
#include <sstream>
#include <stdexcept>

std::string IRValue::toString() const {
    switch (type) {
        case Type::TEMP:
            return "t" + std::to_string(id);
        case Type::GLOBAL:
            return "g_" + name;
        case Type::LOCAL:
            return "l_" + name;
        case Type::CONSTANT:
            return name;
        case Type::LABEL:
            return name;
        default:
            return "unknown";
    }
}

std::string IRInstruction::toString() const {
    std::ostringstream oss;
    oss << opCodeToString(opcode) << " ";
    
    if (opcode == IROpCode::LABEL) {
        oss << label << ":";
    } else {
        for (size_t i = 0; i < operands.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << operands[i].toString();
        }
        
        if (opcode != IROpCode::JMP && opcode != IROpCode::JZ && 
            opcode != IROpCode::JNZ && opcode != IROpCode::RET &&
            opcode != IROpCode::NOP) {
            oss << " -> " << result.toString();
        }
    }
    
    return oss.str();
}

std::string opCodeToString(IROpCode opcode) {
    switch (opcode) {
        case IROpCode::ADD: return "ADD";
        case IROpCode::SUB: return "SUB";
        case IROpCode::MUL: return "MUL";
        case IROpCode::DIV: return "DIV";
        case IROpCode::MOD: return "MOD";
        case IROpCode::NEG: return "NEG";
        case IROpCode::CONCAT: return "CONCAT";
        case IROpCode::AND: return "AND";
        case IROpCode::OR: return "OR";
        case IROpCode::NOT: return "NOT";
        case IROpCode::EQ: return "EQ";
        case IROpCode::NE: return "NE";
        case IROpCode::LT: return "LT";
        case IROpCode::GT: return "GT";
        case IROpCode::LE: return "LE";
        case IROpCode::GE: return "GE";
        case IROpCode::JMP: return "JMP";
        case IROpCode::JZ: return "JZ";
        case IROpCode::JNZ: return "JNZ";
        case IROpCode::CALL: return "CALL";
        case IROpCode::RET: return "RET";
        case IROpCode::LOAD: return "LOAD";
        case IROpCode::STORE: return "STORE";
        case IROpCode::LOAD_GLOBAL: return "LOAD_GLOBAL";
        case IROpCode::STORE_GLOBAL: return "STORE_GLOBAL";
        case IROpCode::LOAD_INT: return "LOAD_INT";
        case IROpCode::LOAD_FLOAT: return "LOAD_FLOAT";
        case IROpCode::LOAD_STRING: return "LOAD_STRING";
        case IROpCode::PRINT: return "PRINT";
        case IROpCode::INPUT: return "INPUT";
        case IROpCode::KEY_PRESSED: return "KEY_PRESSED";
        case IROpCode::SCREEN: return "SCREEN";
        case IROpCode::DRAW_PIXEL: return "DRAW_PIXEL";
        case IROpCode::DRAW_RECT: return "DRAW_RECT";
        case IROpCode::DRAW_LINE: return "DRAW_LINE";
        case IROpCode::DRAW_CIRCLE: return "DRAW_CIRCLE";
        case IROpCode::CLEAR_SCREEN: return "CLEAR_SCREEN";
        case IROpCode::PRESENT: return "PRESENT";
        case IROpCode::LABEL: return "LABEL";
        case IROpCode::NOP: return "NOP";
        default: return "UNKNOWN";
    }
}

IRGenerator::IRGenerator(const ProgramPtr& ast)
    : ast(ast), currentFunction(nullptr), tempCounter(0), labelCounter(0) {}

IRProgram IRGenerator::generate() {
    visitProgram(ast);
    return program;
}

void IRGenerator::visitProgram(const ProgramPtr& program) {
    for (const auto& func : program->functions) {
        visitFunction(func);
    }
}

void IRGenerator::visitFunction(const FunctionDeclPtr& func) {
    IRFunction irFunc;
    irFunc.name = func->name;
    irFunc.returnType = func->returnType;
    irFunc.parameters = func->parameters;
    
    program.functions.push_back(irFunc);
    currentFunction = &program.functions.back();
    
    // Clear symbol table for new function
    symbolTable.clear();
    tempCounter = 0;
    
    // Add parameters to symbol table
    for (const auto& param : func->parameters) {
        IRValue paramVal(IRValue::Type::LOCAL, param.second);
        symbolTable[param.second] = paramVal;
    }
    
    // Visit function body
    visitStatement(func->body);
}

void IRGenerator::visitStatement(const StatementPtr& stmt) {
    if (!stmt) return;

    if (auto block = std::dynamic_pointer_cast<BlockStatement>(stmt)) {
        visitBlockStatement(block);
    } else if (auto ret = std::dynamic_pointer_cast<ReturnStatement>(stmt)) {
        visitReturnStatement(ret);
    } else if (auto ifStmt = std::dynamic_pointer_cast<IfStatement>(stmt)) {
        visitIfStatement(ifStmt);
    } else if (auto whileStmt = std::dynamic_pointer_cast<WhileStatement>(stmt)) {
        visitWhileStatement(whileStmt);
    } else if (auto forStmt = std::dynamic_pointer_cast<ForStatement>(stmt)) {
        visitForStatement(forStmt);
    } else if (auto varDecl = std::dynamic_pointer_cast<VariableDecl>(stmt)) {
        visitVariableDecl(varDecl);
    } else if (auto exprStmt = std::dynamic_pointer_cast<ExpressionStatement>(stmt)) {
        visitExpressionStatement(exprStmt);
    } else if (auto printStmt = std::dynamic_pointer_cast<PrintStatement>(stmt)) {
        IRValue val = visitExpression(printStmt->expression);
        IRInstruction instr(IROpCode::PRINT);
        instr.operands.push_back(val);
        emitInstruction(instr);
    }
}

void IRGenerator::visitBlockStatement(const std::shared_ptr<BlockStatement>& block) {
    for (const auto& stmt : block->statements) {
        visitStatement(stmt);
    }
}

void IRGenerator::visitReturnStatement(const std::shared_ptr<ReturnStatement>& ret) {
    if (ret->expression) {
        IRValue val = visitExpression(ret->expression);
        
        IRInstruction instr(IROpCode::RET);
        instr.operands.push_back(val);
        emitInstruction(instr);
    } else {
        IRInstruction instr(IROpCode::RET);
        emitInstruction(instr);
    }
}

void IRGenerator::visitIfStatement(const std::shared_ptr<IfStatement>& ifStmt) {
    IRValue cond = visitExpression(ifStmt->condition);
    
    std::string thenLabel = createLabel();
    std::string elseLabel = createLabel();
    std::string endLabel = createLabel();
    
    // Jump to else if condition is false
    IRInstruction jz(IROpCode::JZ);
    jz.operands.push_back(cond);
    jz.label = elseLabel;
    emitInstruction(jz);
    
    // Then branch
    IRInstruction thenLabelInstr(IROpCode::LABEL);
    thenLabelInstr.label = thenLabel;
    emitInstruction(thenLabelInstr);
    
    visitStatement(ifStmt->thenBranch);
    
    IRInstruction jmpEnd(IROpCode::JMP);
    jmpEnd.label = endLabel;
    emitInstruction(jmpEnd);
    
    // Else branch
    IRInstruction elseLabelInstr(IROpCode::LABEL);
    elseLabelInstr.label = elseLabel;
    emitInstruction(elseLabelInstr);
    
    if (ifStmt->elseBranch) {
        visitStatement(ifStmt->elseBranch);
    }
    
    // End label
    IRInstruction endLabelInstr(IROpCode::LABEL);
    endLabelInstr.label = endLabel;
    emitInstruction(endLabelInstr);
}

void IRGenerator::visitWhileStatement(const std::shared_ptr<WhileStatement>& whileStmt) {
    std::string loopLabel = createLabel();
    std::string endLabel = createLabel();
    
    // Loop label
    IRInstruction loopLabelInstr(IROpCode::LABEL);
    loopLabelInstr.label = loopLabel;
    emitInstruction(loopLabelInstr);
    
    // Check condition
    IRValue cond = visitExpression(whileStmt->condition);
    
    IRInstruction jz(IROpCode::JZ);
    jz.operands.push_back(cond);
    jz.label = endLabel;
    emitInstruction(jz);
    
    // Loop body
    visitStatement(whileStmt->body);
    
    // Jump back to loop
    IRInstruction jmp(IROpCode::JMP);
    jmp.label = loopLabel;
    emitInstruction(jmp);
    
    // End label
    IRInstruction endLabelInstr(IROpCode::LABEL);
    endLabelInstr.label = endLabel;
    emitInstruction(endLabelInstr);
}

void IRGenerator::visitForStatement(const std::shared_ptr<ForStatement>& forStmt) {
    // Initialize
    if (forStmt->init) {
        visitStatement(forStmt->init);
    }
    
    std::string loopLabel = createLabel();
    std::string endLabel = createLabel();
    
    // Loop label
    IRInstruction loopLabelInstr(IROpCode::LABEL);
    loopLabelInstr.label = loopLabel;
    emitInstruction(loopLabelInstr);
    
    // Check condition
    if (forStmt->condition) {
        IRValue cond = visitExpression(forStmt->condition);
        
        IRInstruction jz(IROpCode::JZ);
        jz.operands.push_back(cond);
        jz.label = endLabel;
        emitInstruction(jz);
    }
    
    // Loop body
    visitStatement(forStmt->body);
    
    // Increment
    if (forStmt->increment) {
        visitExpression(forStmt->increment);
    }
    
    // Jump back to loop
    IRInstruction jmp(IROpCode::JMP);
    jmp.label = loopLabel;
    emitInstruction(jmp);
    
    // End label
    IRInstruction endLabelInstr(IROpCode::LABEL);
    endLabelInstr.label = endLabel;
    emitInstruction(endLabelInstr);
}

void IRGenerator::visitVariableDecl(const std::shared_ptr<VariableDecl>& varDecl) {
    IRValue var(IRValue::Type::LOCAL, varDecl->name);
    symbolTable[varDecl->name] = var;
    
    if (varDecl->initializer) {
        IRValue val = visitExpression(varDecl->initializer);
        
        IRInstruction instr(IROpCode::STORE);
        instr.operands.push_back(val);
        instr.result = var;
        emitInstruction(instr);
    }
}

void IRGenerator::visitExpressionStatement(const std::shared_ptr<ExpressionStatement>& exprStmt) {
    visitExpression(exprStmt->expression);
}

IRValue IRGenerator::visitExpression(const ExpressionPtr& expr) {
    if (!expr) return IRValue();

    if (auto binOp = std::dynamic_pointer_cast<BinaryOp>(expr)) {
        return visitBinaryOp(binOp);
    } else if (auto unaryOp = std::dynamic_pointer_cast<UnaryOp>(expr)) {
        return visitUnaryOp(unaryOp);
    } else if (auto lit = std::dynamic_pointer_cast<Literal>(expr)) {
        return visitLiteral(lit);
    } else if (auto id = std::dynamic_pointer_cast<Identifier>(expr)) {
        return visitIdentifier(id);
    } else if (auto call = std::dynamic_pointer_cast<FunctionCall>(expr)) {
        return visitFunctionCall(call);
    } else if (auto assign = std::dynamic_pointer_cast<Assignment>(expr)) {
        return visitAssignment(assign);
    } else if (auto access = std::dynamic_pointer_cast<ArrayAccess>(expr)) {
        return visitArrayAccess(access);
    } else if (auto inputCall = std::dynamic_pointer_cast<InputCall>(expr)) {
        IRValue result = createTemp();
        IRInstruction instr(IROpCode::INPUT);
        
        // Get the prompt text if provided
        if (inputCall->prompt) {
            auto promptLit = std::dynamic_pointer_cast<Literal>(inputCall->prompt);
            if (promptLit && promptLit->type == TokenType::STRING) {
                instr.prompt = promptLit->value;
            }
        }
        
        instr.result = result;
        emitInstruction(instr);
        return result;
    } else if (std::dynamic_pointer_cast<KeyPressedCall>(expr)) {
        IRValue result = createTemp();
        IRInstruction instr(IROpCode::KEY_PRESSED);
        instr.result = result;
        emitInstruction(instr);
        return result;
    }
    return IRValue();
}

IRValue IRGenerator::visitBinaryOp(const std::shared_ptr<BinaryOp>& binOp) {
    IRValue left = visitExpression(binOp->left);
    IRValue right = visitExpression(binOp->right);
    IRValue result = createTemp();
    
    // Use CONCAT for comma or || when concatenating values
    IROpCode opcode = IROpCode::NOP;
    if (binOp->op == TokenType::OR || binOp->op == TokenType::COMMA) {
        opcode = IROpCode::CONCAT;
    } else {
        opcode = tokenTypeToOpCode(binOp->op);
    }
    
    IRInstruction instr(opcode);
    instr.operands.push_back(left);
    instr.operands.push_back(right);
    instr.result = result;
    emitInstruction(instr);
    
    return result;
}

IRValue IRGenerator::visitUnaryOp(const std::shared_ptr<UnaryOp>& unaryOp) {
    IRValue operand = visitExpression(unaryOp->operand);
    IRValue result = createTemp();
    
    IROpCode opcode = tokenTypeToOpCode(unaryOp->op);
    
    IRInstruction instr(opcode);
    instr.operands.push_back(operand);
    instr.result = result;
    emitInstruction(instr);
    
    return result;
}

IRValue IRGenerator::visitLiteral(const std::shared_ptr<Literal>& lit) {
    IRValue val(IRValue::Type::CONSTANT, lit->value);
    
    IROpCode loadOp;
    if (lit->type == TokenType::INTEGER) {
        loadOp = IROpCode::LOAD_INT;
    } else if (lit->type == TokenType::FLOAT) {
        loadOp = IROpCode::LOAD_FLOAT;
    } else if (lit->type == TokenType::STRING) {
        loadOp = IROpCode::LOAD_STRING;
    } else {
        return val;
    }
    
    IRValue result = createTemp();
    
    IRInstruction instr(loadOp);
    instr.operands.push_back(val);
    instr.result = result;
    emitInstruction(instr);
    
    return result;
}

IRValue IRGenerator::visitIdentifier(const std::shared_ptr<Identifier>& id) {
    auto it = symbolTable.find(id->name);
    if (it != symbolTable.end()) {
        return it->second;
    }
    
    // Undefined variable - create as local
    IRValue var(IRValue::Type::LOCAL, id->name);
    symbolTable[id->name] = var;
    return var;
}

IRValue IRGenerator::visitFunctionCall(const std::shared_ptr<FunctionCall>& call) {
    IRValue result = createTemp();
    
    // Handle screen() function specially
    if (call->name == "screen") {
        IRInstruction instr(IROpCode::SCREEN);
        
        // Extract width, height, and title from arguments
        for (size_t i = 0; i < call->arguments.size(); i++) {
            IRValue argVal = visitExpression(call->arguments[i]);
            instr.operands.push_back(argVal);
        }
        
        instr.result = result;
        emitInstruction(instr);
        return result;
    }
    
    // Handle graphics drawing functions
    if (call->name == "clearScreen") {
        IRInstruction instr(IROpCode::CLEAR_SCREEN);
        for (const auto& arg : call->arguments) {
            IRValue argVal = visitExpression(arg);
            instr.operands.push_back(argVal);
        }
        instr.result = result;
        emitInstruction(instr);
        return result;
    }
    
    if (call->name == "drawPixel") {
        IRInstruction instr(IROpCode::DRAW_PIXEL);
        for (const auto& arg : call->arguments) {
            IRValue argVal = visitExpression(arg);
            instr.operands.push_back(argVal);
        }
        instr.result = result;
        emitInstruction(instr);
        return result;
    }
    
    if (call->name == "drawRect") {
        IRInstruction instr(IROpCode::DRAW_RECT);
        for (const auto& arg : call->arguments) {
            IRValue argVal = visitExpression(arg);
            instr.operands.push_back(argVal);
        }
        instr.result = result;
        emitInstruction(instr);
        return result;
    }
    
    if (call->name == "drawLine") {
        IRInstruction instr(IROpCode::DRAW_LINE);
        for (const auto& arg : call->arguments) {
            IRValue argVal = visitExpression(arg);
            instr.operands.push_back(argVal);
        }
        instr.result = result;
        emitInstruction(instr);
        return result;
    }
    
    if (call->name == "drawCircle") {
        IRInstruction instr(IROpCode::DRAW_CIRCLE);
        for (const auto& arg : call->arguments) {
            IRValue argVal = visitExpression(arg);
            instr.operands.push_back(argVal);
        }
        instr.result = result;
        emitInstruction(instr);
        return result;
    }
    
    if (call->name == "display") {
        IRInstruction instr(IROpCode::PRESENT);
        instr.result = result;
        emitInstruction(instr);
        return result;
    }
    
    if (call->name == "quit") {
        // For now, quit just exits the program
        // We can handle this in the interpreter
        IRInstruction instr(IROpCode::CALL);
        instr.label = "quit";
        instr.result = result;
        emitInstruction(instr);
        return result;
    }
    
    if (call->name == "isKeyDown") {
        // isKeyDown(keyCode) - returns 1 if key is pressed, 0 otherwise
        IRInstruction instr(IROpCode::CALL);
        instr.label = "isKeyDown";
        
        if (call->arguments.size() > 0) {
            IRValue keyVal = visitExpression(call->arguments[0]);
            instr.operands.push_back(keyVal);
        }
        
        instr.result = result;
        emitInstruction(instr);
        return result;
    }
    
    if (call->name == "updateInput") {
        // updateInput() - update keyboard state
        IRInstruction instr(IROpCode::CALL);
        instr.label = "updateInput";
        instr.result = result;
        emitInstruction(instr);
        return result;
    }
    
    IRInstruction instr(IROpCode::CALL);
    instr.label = call->name;
    
    for (const auto& arg : call->arguments) {
        IRValue argVal = visitExpression(arg);
        instr.operands.push_back(argVal);
    }
    
    instr.result = result;
    emitInstruction(instr);
    
    return result;
}

IRValue IRGenerator::visitAssignment(const std::shared_ptr<Assignment>& assign) {
    IRValue value = visitExpression(assign->value);
    
    auto it = symbolTable.find(assign->name);
    IRValue var;
    if (it != symbolTable.end()) {
        var = it->second;
    } else {
        var = IRValue(IRValue::Type::LOCAL, assign->name);
        symbolTable[assign->name] = var;
    }
    
    IRInstruction instr(IROpCode::STORE);
    instr.operands.push_back(value);
    instr.result = var;
    emitInstruction(instr);
    
    return var;
}

IRValue IRGenerator::visitArrayAccess(const std::shared_ptr<ArrayAccess>& access) {
    IRValue array = visitExpression(access->array);
    IRValue index = visitExpression(access->index);
    IRValue result = createTemp();
    
    IRInstruction instr(IROpCode::LOAD);
    instr.operands.push_back(array);
    instr.operands.push_back(index);
    instr.result = result;
    emitInstruction(instr);
    
    return result;
}

IRValue IRGenerator::createTemp() {
    return IRValue(IRValue::Type::TEMP, "t" + std::to_string(tempCounter), tempCounter++);
}

std::string IRGenerator::createLabel() {
    return "L" + std::to_string(labelCounter++);
}

void IRGenerator::emitInstruction(const IRInstruction& instr) {
    if (currentFunction) {
        currentFunction->instructions.push_back(instr);
    }
}

IROpCode IRGenerator::tokenTypeToOpCode(TokenType type) {
    switch (type) {
        case TokenType::PLUS: return IROpCode::ADD;
        case TokenType::MINUS: return IROpCode::SUB;
        case TokenType::STAR: return IROpCode::MUL;
        case TokenType::SLASH: return IROpCode::DIV;
        case TokenType::PERCENT: return IROpCode::MOD;
        case TokenType::AND: return IROpCode::AND;
        case TokenType::OR: return IROpCode::OR;
        case TokenType::COMMA: return IROpCode::CONCAT;
        case TokenType::NOT: return IROpCode::NOT;
        case TokenType::EQUAL: return IROpCode::EQ;
        case TokenType::NOT_EQUAL: return IROpCode::NE;
        case TokenType::LESS: return IROpCode::LT;
        case TokenType::GREATER: return IROpCode::GT;
        case TokenType::LESS_EQUAL: return IROpCode::LE;
        case TokenType::GREATER_EQUAL: return IROpCode::GE;
        default: return IROpCode::NOP;
    }
}
