//
// Created by ethereal on 2023/4/2.
//

#include <iostream>
#include "compiler.h"
#include "value.h"
#include "object.h"
#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

ParseRule Compiler::m_rules[] = {
        [TOKEN_LEFT_PAREN]    = {(ParseFn)&Compiler::grouping,  NULL,   PREC_NONE},
        [TOKEN_RIGHT_PAREN]   = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_LEFT_BRACE]    = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_RIGHT_BRACE]   = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_COMMA]         = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_DOT]           = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_MINUS]         = {(ParseFn)&Compiler::unary,     (ParseFn)&Compiler::binary, PREC_TERM},
        [TOKEN_PLUS]          = {NULL,                        (ParseFn)&Compiler::binary, PREC_TERM},
        [TOKEN_SEMICOLON]     = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_SLASH]         = {NULL,                        (ParseFn)&Compiler::binary, PREC_FACTOR},
        [TOKEN_STAR]          = {NULL,                        (ParseFn)&Compiler::binary, PREC_FACTOR},
        [TOKEN_BANG]          = {(ParseFn)&Compiler::unary,     NULL,   PREC_NONE},
        [TOKEN_BANG_EQUAL]    = {NULL,                        (ParseFn)&Compiler::binary, PREC_EQUALITY},
        [TOKEN_EQUAL]         = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_EQUAL_EQUAL]   = {NULL,                        (ParseFn)&Compiler::binary, PREC_EQUALITY},
        [TOKEN_GREATER]       = {NULL,                        (ParseFn)&Compiler::binary, PREC_COMPARISON},
        [TOKEN_GREATER_EQUAL] = {NULL,                        (ParseFn)&Compiler::binary, PREC_COMPARISON},
        [TOKEN_LESS]          = {NULL,                        (ParseFn)&Compiler::binary, PREC_COMPARISON},
        [TOKEN_LESS_EQUAL]    = {NULL,                        (ParseFn)&Compiler::binary, PREC_COMPARISON},
        [TOKEN_IDENTIFIER]    = {(ParseFn)&Compiler::variable,  NULL,   PREC_NONE},
        [TOKEN_STRING]        = {(ParseFn)&Compiler::string,    NULL,   PREC_NONE},
        [TOKEN_NUMBER]        = {(ParseFn)&Compiler::number,    NULL,   PREC_NONE},
        [TOKEN_AND]           = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_CLASS]         = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_ELSE]          = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_FALSE]         = {(ParseFn)&Compiler::literal,   NULL,   PREC_NONE},
        [TOKEN_FOR]           = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_FUN]           = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_IF]            = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_NIL]           = {(ParseFn)&Compiler::literal,   NULL,   PREC_NONE},
        [TOKEN_OR]            = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_PRINT]         = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_RETURN]        = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_SUPER]         = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_THIS]          = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_TRUE]          = {(ParseFn)&Compiler::literal,   NULL,   PREC_NONE},
        [TOKEN_VAR]           = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_WHILE]         = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_ERROR]         = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_EOF]           = {NULL,                        NULL,   PREC_NONE},
};

Compiler::Compiler(const std::string& source, Chunk* chunk){
    m_hadError = false;
    m_panicMode = false;
    m_chunk = chunk;
    m_sc = new Scanner(source);
}

Compiler::~Compiler(){
    delete m_sc;
}

void Compiler::errorAt(Token* token, const char* message){
    if(m_panicMode) return;
    m_panicMode = true; //设置之后字节码不会被执行
    std::cerr<<"[line "<<token->line<<"] error";

    if(token->type == TOKEN_EOF){
        std::cerr<<" at end";
    }else if(token->type == TOKEN_ERROR){

    }else{
        std::cerr<<" at "<<token->start;
    }

    std::cerr<<message<<std::endl;
    m_hadError = true;
}

void Compiler::errorAtCurrent(const char* message){
    errorAt(&m_current, message);
}

void Compiler::error(const char* message){
    errorAt(&m_previous, message);
}

void Compiler::advance(){
    m_previous = m_current;
    for(;;){
        m_current = m_sc->scanToken();
        if(m_current.type != TOKEN_ERROR)break;

        errorAtCurrent(m_current.start);
    }
}

bool Compiler::match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
}

bool Compiler::check(TokenType type) {
    return m_current.type == type;
}

void Compiler::synchronize() {
    m_panicMode = false;

    while (m_current.type != TOKEN_EOF) {
        if (m_previous.type == TOKEN_SEMICOLON) return;
        switch (m_current.type) {
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;

            default:
                ; // Do nothing.
        }

        advance();
    }
}

void Compiler::consume(TokenType type, const char* message){
    if(m_current.type == type){
        advance();
        return;
    }
    errorAtCurrent(message);
}

void Compiler::emitByte(uint8_t byte){
    m_chunk->writeChunk(byte, m_previous.line);
}

void Compiler::emitBytes(uint8_t byte1, uint8_t byte2){
    emitByte(byte1);
    emitByte(byte2);
}

void Compiler::emitConstant(Value value){
    emitBytes(OP_CONSTANT, m_chunk->addConstant(value));
}

void Compiler::emitReturn(){
    emitByte(OP_RETURN);
}

void Compiler::endCompiler(){
    emitReturn();
#ifdef DEBUG_PRINT_CODE
    if (!m_hadError) {
    disassembleChunk(*m_chunk, "code");
  }
#endif
}

void Compiler::grouping(bool canAssign){
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression");
}

void Compiler::unary(bool canAssign){
    TokenType operatorType = m_previous.type;

    parsePrecedence(PREC_UNARY);

    switch(operatorType){
        case TOKEN_BANG: emitByte(OP_NOT); break;
        case TOKEN_MINUS: emitByte(OP_NEGATE); break;
        default: return;
    }
}

//解析中缀表达式时，左操作数已经编译过了（在栈内）
//右操作数先入栈，在写二元运算符的字节码
//二元运算符的右操作数优先级比自己高一级
void Compiler::binary(bool canAssign){
    TokenType operatorType = m_previous.type;
    ParseRule* rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));

    switch (operatorType) {
        case TOKEN_BANG_EQUAL:    emitBytes(OP_EQUAL, OP_NOT); break;
        case TOKEN_EQUAL_EQUAL:   emitByte(OP_EQUAL); break;
        case TOKEN_GREATER:       emitByte(OP_GREATER); break;
        case TOKEN_GREATER_EQUAL: emitBytes(OP_LESS, OP_NOT); break;
        case TOKEN_LESS:          emitByte(OP_LESS); break;
        case TOKEN_LESS_EQUAL:    emitBytes(OP_GREATER, OP_NOT); break;
        case TOKEN_PLUS:          emitByte(OP_ADD); break;
        case TOKEN_MINUS:         emitByte(OP_SUBTRACT); break;
        case TOKEN_STAR:          emitByte(OP_MULTIPLY); break;
        case TOKEN_SLASH:         emitByte(OP_DIVIDE); break;
        default: return; // Unreachable.
    }
}

void Compiler::literal(bool canAssign){
    switch(m_previous.type){
        case TOKEN_FALSE: emitByte(OP_FALSE); break;
        case TOKEN_NIL: emitByte(OP_NIL); break;
        case TOKEN_TRUE: emitByte(OP_TRUE); break;
        default: return;
    }
}

ParseRule* Compiler::getRule(TokenType type){
    return &m_rules[type];
}

void Compiler::number(bool canAssign){
    double value = strtod(m_previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

void Compiler::string(bool canAssign) {
    emitConstant(OBJ_VAL(copyString(m_previous.start+1 ,
                                    m_previous.length - 2)));
}

void Compiler::parsePrecedence(Precedence precedence){
    advance();
    ParseFn prefixRule = getRule(m_previous.type)->prefix;
    if(prefixRule == NULL){
        error("Expect expression");
        return;
    }
    bool canAssign = (precedence <= PREC_ASSIGNMENT);
    (this->*prefixRule)(canAssign);
    while(precedence <= getRule(m_current.type)->precedence){
        advance();
        ParseFn infixRule = getRule(m_previous.type)->infix;
        (this->*infixRule)(canAssign);
    }
    if (canAssign && match(TOKEN_EQUAL)) {
        error("Invalid assignment target.");
    }
}

uint8_t Compiler::identifierConstant(Token* name) {
    return m_chunk->addConstant(OBJ_VAL(copyString(name->start,
                                                   name->length)));
}

void Compiler::expression(){
    parsePrecedence(PREC_ASSIGNMENT);
}

void Compiler::varDeclaration() {
    uint8_t global = parseVariable("Expect variable name.");

    if (match(TOKEN_EQUAL)) {
        expression();
    } else {
        emitByte(OP_NIL);
    }
    consume(TOKEN_SEMICOLON,
            "Expect ';' after variable declaration.");

    defineVariable(global);
}

void Compiler::namedVariable(Token name, bool canAssign) {
    uint8_t arg = identifierConstant(&name);
    if (canAssign && match(TOKEN_EQUAL)) {
        expression();
        emitBytes(OP_SET_GLOBAL, arg);
    } else {
        emitBytes(OP_GET_GLOBAL, arg);
    }
}

void Compiler::variable(bool canAssign){
    namedVariable(m_previous, canAssign);
}

uint8_t Compiler::parseVariable(const char* errorMessage) {
    consume(TOKEN_IDENTIFIER, errorMessage);
    return identifierConstant(&m_previous);
}

void Compiler::defineVariable(uint8_t global) {
    emitBytes(OP_DEFINE_GLOBAL, global);
}

void Compiler::expressionStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    emitByte(OP_POP);
}

void Compiler::printStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after value.");
    emitByte(OP_PRINT);
}

void Compiler::declaration(){
    if (match(TOKEN_VAR)) {
        varDeclaration();
    } else {
        statement();
    }
    if (m_panicMode) synchronize();
}

void Compiler::statement(){
    if (match(TOKEN_PRINT)) {
        printStatement();
    }else {
        expressionStatement();
    }
}

bool Compiler::compile() {
    advance();
    while (!match(TOKEN_EOF)) {
        declaration();
    }
    endCompiler();
    return !m_hadError;
}
