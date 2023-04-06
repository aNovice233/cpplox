#pragma once
#include <string>
#include "common.h"
#include "chunk.h"
#include "scanner.h"

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_CALL,        // . ()
    PREC_PRIMARY
} Precedence; //优先级，从低到高排序

class Compiler;

typedef void (Compiler::*ParseFn)(bool);

typedef struct {
    ParseFn prefix;   //以该类型标识为起点的前缀表达式的函数
    ParseFn infix;    //左操作数后跟该着的类型的中缀表达式的函数
    Precedence precedence;    //用该标识作为操作符的中缀表达式的优先级
} ParseRule;

//0是全局作用域，1是第一个顶层块，2是它内部的块
typedef struct{
    Token name;
    int depth;      //声明局部变量的代码块深度
} Local;

class Compiler{
    Token m_current;
    Token m_previous;
    bool m_hadError;
    bool m_panicMode;
    Scanner *m_sc;
    Chunk *m_chunk;
    static ParseRule m_rules[];

    Local m_locals[UINT8_COUNT];
    int m_localCount;     //作用域中有多少局部变量
    int m_scopeDepth;     //作用域深度，正在编译的当前代码外围的代码块数量

private:
    void advance(); //取下一个token，判断是否出错
    void consume(TokenType type, const char* message); //验证当前token是否等于预期，是则取下一token
    void errorAtCurrent(const char* message);
    void error(const char* message);
    void errorAt(Token* token, const char* message);
    bool match(TokenType type);
    bool check(TokenType type);
    void synchronize();

    //发出字节码
    void emitByte(uint8_t byte);
    void emitBytes(uint8_t byte1, uint8_t byte2);
    void emitLoop(int loopStart);
    int  emitJump(uint8_t instruction);
    void emitConstant(Value value);
    void emitReturn();

    void patchJump(int offset);

    //解析表达式
    void grouping(bool canAssign);
    void unary(bool canAssign);
    void binary(bool canAssign);
    void literal(bool canAssign);
    void and_(bool canAssign);
    void or_(bool canAssign);

    ParseRule* getRule(TokenType type);
    void number(bool canAssign); //指向下面函数的指针
    void string(bool canAssign);
    void parsePrecedence(Precedence precedence); //解析给定优先级和更高优先级的表达式
    uint8_t identifierConstant(Token* name);    //chunk写入标识符的constant(Value类型)，返回下标
    bool identifiersEqual(Token* a, Token* b);
    int resolveLocal(Token* name);
    void expression();

    void block();
    void beginScope();
    void endScope();

    void addLocal(Token name);
    void declareVariable();  //声明局部变量
    void varDeclaration();   //变量声明解析
    void namedVariable(Token name, bool canAssign);   //变量访问，解析已定义的变量
    void variable(bool canAssign);
    uint8_t parseVariable(const char* errorMessage);
    void markInitialized();
    void defineVariable(uint8_t global);

    void expressionStatement();
    void forStatement();
    void ifStatement();
    void printStatement();
    void whileStatement();
    void declaration();
    void statement();

    void endCompiler();

public:
    Compiler(const std::string& source, Chunk* chunk);
    ~Compiler();

    bool compile();
};
