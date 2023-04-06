#include <iostream>
#include <string.h>
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
        [TOKEN_AND]           = {NULL,                        (ParseFn)&Compiler::and_,   PREC_AND},
        [TOKEN_CLASS]         = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_ELSE]          = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_FALSE]         = {(ParseFn)&Compiler::literal,   NULL,   PREC_NONE},
        [TOKEN_FOR]           = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_FUN]           = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_IF]            = {NULL,                        NULL,   PREC_NONE},
        [TOKEN_NIL]           = {(ParseFn)&Compiler::literal,   NULL,   PREC_NONE},
        [TOKEN_OR]            = {NULL,                        (ParseFn)&Compiler::or_,    PREC_OR},
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

    m_localCount = 0;
    m_scopeDepth = 0;
}

Compiler::~Compiler(){
    delete m_sc;
}

void Compiler::errorAt(Token* token, const char* message){
    if(m_panicMode) return;     //出现一个错误后屏蔽后续的一连串错误
    m_panicMode = true; //设置之后字节码不会被执行

    std::cerr<<"[line "<<token->line<<"] error";

    if(token->type == TOKEN_EOF){
        std::cerr<<" at end";
    }else if(token->type == TOKEN_ERROR){

    }else{
        std::cerr << " at '" << std::string(token->start, token->length) << "'  ";
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

//跳过标识，直到语句边界
//如分号；或者查找能够开始一条语句标识，通常是控制流或声明语句的关键字之一
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

void Compiler::emitLoop(int loopStart){
    emitByte(OP_LOOP);

    // +2 是考虑到OP_LOOP指令自身操作数的大小，也需要跳过
    int offset = m_chunk->getCount() - loopStart + 2;
    if (offset > UINT16_MAX) error("Loop body too large.");

    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

int Compiler::emitJump(uint8_t instruction){
    emitByte(instruction);
    emitByte(0xff);
    emitByte(0xff);
    return m_chunk->getCount()-2;
}

void Compiler::emitConstant(Value value){
    emitBytes(OP_CONSTANT, m_chunk->addConstant(value));
}

void Compiler::emitReturn(){
    emitByte(OP_RETURN);
}

void Compiler::patchJump(int offset){
    int jump = m_chunk->getCount() - offset - 2;

    if(jump > UINT16_MAX){
        error("Too much code to jump over.");
    }

    m_chunk->changeCode(offset, (jump >> 8) & 0xff);
    m_chunk->changeCode(offset+1, jump & 0xff);
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

//左侧表达式的值已经被编译了，运行时，结果在栈顶。
//如果这个值为false，整个and结果为false，跳过右边操作数，栈顶就是
//表达式结果，否则丢弃左值，计算右操作数，作为and结果。
void Compiler::and_(bool canAssign){
    int endJump = emitJump(OP_JUMP_IF_FALSE);

    emitByte(OP_POP);
    parsePrecedence(PREC_AND);

    patchJump(endJump);
}

void Compiler::or_(bool canAssign){
    int elseJump = emitJump(OP_JUMP_IF_FALSE);
    int endJump = emitJump(OP_JUMP);

    patchJump(elseJump);
    emitByte(OP_POP);

    parsePrecedence(PREC_OR);
    patchJump(endJump);
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

bool Compiler::identifiersEqual(Token* a, Token* b){
    if(a->length != b->length) return false;
    return memcmp(a->start, b->start, a->length) == 0;
}

int Compiler::resolveLocal(Token* name){
    for (int i = m_localCount - 1; i >= 0; i--) {
        Local* local = &m_locals[i];
        if (identifiersEqual(name, &local->name)) {
            if (local->depth == -1) {
                error("Can't read local variable in its own initializer.");
            }
            return i;
        }
    }
    return -1;
}

void Compiler::expression(){
    parsePrecedence(PREC_ASSIGNMENT);
}

void Compiler::block(){
    while(!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)){
        declaration();
    }

    consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

void Compiler::beginScope(){
    m_scopeDepth++;
}

void Compiler::endScope(){
    m_scopeDepth--;
    while(m_localCount > 0 && m_locals[m_localCount - 1].depth > m_scopeDepth){
        emitByte(OP_POP);
        m_localCount--;
    }
}

void Compiler::addLocal(Token name){
    if(m_localCount == UINT8_COUNT){
        error("Too many local variables in function.");
        return;
    }
    Local* local = &m_locals[m_localCount++];
    local->name = name;
    local->depth = -1;  //已声明还未初始化
}

//声明局部变量
//全局变量是后期绑定的，编译器不会看到全局变量的声明
void Compiler::declareVariable(){
    if(m_scopeDepth == 0) return;
    Token* name = &m_previous;
    for(int i = m_localCount-1; i>=0; i--){
        Local* local = &m_locals[i];
        if(local->depth != -1 && local->depth < m_scopeDepth){
            break;
        }
        if (identifiersEqual(name, &local->name)) {
            error("Already a variable with this name in this scope.");
        }
    }
    addLocal(*name);
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
    uint8_t getOp, setOp;
    int arg = resolveLocal(&name);  //返回-1表明是全局变量，否则返回在m_Locals数组中的位置
    if (arg != -1) {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    } else {
        arg = identifierConstant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }
    //查找标识符后面的等号。如果找到了，我们就不会生成变量访问的代码，
    //我们会编译所赋的值，然后生成一个赋值指令。
    if (canAssign && match(TOKEN_EQUAL)) {
        expression();
        emitBytes(setOp, (uint8_t)arg);
    } else {
        emitBytes(getOp, (uint8_t)arg);
    }
}

void Compiler::variable(bool canAssign){
    namedVariable(m_previous, canAssign);
}

uint8_t Compiler::parseVariable(const char* errorMessage) {
    consume(TOKEN_IDENTIFIER, errorMessage);
    declareVariable();
    if (m_scopeDepth > 0) return 0;
    return identifierConstant(&m_previous);
}

void Compiler::markInitialized(){
    m_locals[m_localCount - 1].depth = m_scopeDepth;
}

void Compiler::defineVariable(uint8_t global) {
    if (m_scopeDepth > 0) { //局部变量不需要写入chunk，它的值就在栈顶
        markInitialized();  //例如var a = 1+2; 执行完之后栈顶就是3;
        return;                 
    }
    emitBytes(OP_DEFINE_GLOBAL, global);
}

void Compiler::expressionStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    emitByte(OP_POP);
}

void Compiler::forStatement() {
    beginScope();   //for应该在一个作用域内
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
    if (match(TOKEN_SEMICOLON)) {
        // 无初始化语句
    } else if (match(TOKEN_VAR)) {
        varDeclaration();
    } else {
        expressionStatement();
    }

    int loopStart = m_chunk->getCount();
    int exitJump = -1;
    if (!match(TOKEN_SEMICOLON)) {
        expression();
        consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

        // Jump out of the loop if the condition is false.
        exitJump = emitJump(OP_JUMP_IF_FALSE);
        emitByte(OP_POP); // Condition.
    }
    if (!match(TOKEN_RIGHT_PAREN)) {
        int bodyJump = emitJump(OP_JUMP);
        int incrementStart = m_chunk->getCount();
        expression();
        emitByte(OP_POP);
        consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

        emitLoop(loopStart);
        loopStart = incrementStart;
        patchJump(bodyJump);
    }

    statement();
    emitLoop(loopStart);

    if (exitJump != -1) {
        patchJump(exitJump);
        emitByte(OP_POP); // Condition.
    }

    endScope();
}

void Compiler::ifStatement(){
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after 'if'.");

    int thenJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);

    statement();

    int elseJump = emitJump(OP_JUMP);

    patchJump(thenJump);
    emitByte(OP_POP);
    if (match(TOKEN_ELSE)) statement();
    patchJump(elseJump);
}

void Compiler::printStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after value.");
    emitByte(OP_PRINT);
}

void Compiler::whileStatement(){
    int loopStart = m_chunk->getCount();

    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    int exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();    //循环体

    emitLoop(loopStart);

    patchJump(exitJump);
    emitByte(OP_POP);
}

void Compiler::declaration(){
    if (match(TOKEN_VAR)) {
        varDeclaration();
    } else {
        statement();
    }
    //恐慌模式下的错误恢复来减少它所报告的级联编译错误
    //语句边界作为同步点，退出恐慌模式
    if (m_panicMode) synchronize();
}

/*
statement      → exprStmt
               | forStmt
               | ifStmt
               | printStmt
               | returnStmt
               | whileStmt
               | block ;
*/
void Compiler::statement(){
    if (match(TOKEN_PRINT)) {
        printStatement();
    } else if (match(TOKEN_FOR)) {
        forStatement();
    } else if (match(TOKEN_IF)) {
        ifStatement();
    } else if (match(TOKEN_WHILE)) {
        whileStatement();    
    } else if (match(TOKEN_LEFT_BRACE)) {   // { block
        beginScope();
        block();
        endScope();
    } else {
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
