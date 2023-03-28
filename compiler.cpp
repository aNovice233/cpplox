#include <iostream>
#include "compiler.h"
#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

ParseRule Parser::m_rules[] = {
  [TOKEN_LEFT_PAREN]    = {(ParseFn)&Parser::grouping, NULL,   PREC_NONE},
  [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, 
  [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_MINUS]         = {(ParseFn)&Parser::unary,    (ParseFn)&Parser::binary, PREC_TERM},
  [TOKEN_PLUS]          = {NULL,     (ParseFn)&Parser::binary, PREC_TERM},
  [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SLASH]         = {NULL,     (ParseFn)&Parser::binary, PREC_FACTOR},
  [TOKEN_STAR]          = {NULL,     (ParseFn)&Parser::binary, PREC_FACTOR},
  [TOKEN_BANG]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_GREATER]       = {NULL,     NULL,   PREC_NONE},
  [TOKEN_GREATER_EQUAL] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS_EQUAL]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_STRING]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {(ParseFn)&Parser::number,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NIL]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_TRUE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};

Parser::Parser(const std::string source, Chunk* chunk){
    m_hadError = false;
    m_panicMode = false;
    m_chunk = chunk;
    m_sc = new Scanner(source);
}

Parser::~Parser(){
    delete m_sc;
}

void Parser::errorAt(Token* token, const char* message){
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

void Parser::errorAtCurrent(const char* message){
    errorAt(&m_current, message);
}

void Parser::error(const char* message){
    errorAt(&m_previous, message);
}

void Parser::advance(){
    m_previous = m_current;
    for(;;){
        m_current = m_sc->scanToken();
        if(m_current.type != TOKEN_ERROR)break;

        errorAtCurrent(m_current.start);
    }
}

void Parser::consume(TokenType type, const char* message){
    if(m_current.type == type){
        advance();
        return;
    }
    errorAtCurrent(message);
}

void Parser::emitByte(uint8_t byte){
    m_chunk->writeChunk(byte, m_previous.line);
}

void Parser::emitBytes(uint8_t byte1, uint8_t byte2){
    emitByte(byte1);
    emitByte(byte2);
}

void Parser::emitConstant(Value value){
    emitBytes(OP_CONSTANT, m_chunk->addConstant(value));
}

void Parser::emitReturn(){
    emitByte(OP_RETURN);
}

void Parser::endCompiler(){
    emitReturn();
#ifdef DEBUG_PRINT_CODE
  if (!m_hadError) {
    disassembleChunk(*m_chunk, "code");
  }
#endif
}

void Parser::grouping(){
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression");
}

void Parser::unary(){
    TokenType operatorType = m_previous.type;

    parsePrecedence(PREC_UNARY);

    switch(operatorType){
        case TOKEN_MINUS: emitByte(OP_NEGATE); break;
        default: return;
    }
}

//解析中缀表达式时，左操作数已经编译过了（在栈内）
//右操作数先入栈，在写二元运算符的字节码
//二元运算符的右操作数优先级比自己高一级
void Parser::binary(){
    TokenType operatorType = m_previous.type;
    ParseRule* rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));

    switch (operatorType) {
        case TOKEN_PLUS:          emitByte(OP_ADD); break;
        case TOKEN_MINUS:         emitByte(OP_SUBTRACT); break;
        case TOKEN_STAR:          emitByte(OP_MULTIPLY); break;
        case TOKEN_SLASH:         emitByte(OP_DIVIDE); break;
        default: return; // Unreachable.
    }
}

ParseRule* Parser::getRule(TokenType type){
    return &m_rules[type];
}

void Parser::number(){
    double value = strtod(m_previous.start, NULL);
    emitConstant(value);
}

void Parser::parsePrecedence(Precedence precedence){
    advance();
    ParseFn prefixRule = getRule(m_previous.type)->prefix;
    if(prefixRule == NULL){
        error("Expect expression");
        return;
    }

    prefixRule();
    while(precedence <= getRule(m_current.type)->precedence){
        advance();
        ParseFn infixRule = getRule(m_previous.type)->infix;
        infixRule();
    }
}

void Parser::expression(){
    parsePrecedence(PREC_ASSIGNMENT);
}

bool Parser::compile() {
    advance();
    expression();
    //编译后处于源代码末尾，检查EOF
    consume(TOKEN_EOF, "Expect end of expression.");
    endCompiler();
    return !m_hadError;
}