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

using ParseFn = void(*)();

typedef struct {
  ParseFn prefix;   //以该类型标识为起点的前缀表达式的函数
  ParseFn infix;    //左操作数后跟该着的类型的中缀表达式的函数
  Precedence precedence;    //用该标识作为操作符的中缀表达式的优先级
} ParseRule;

class Parser{
    Token m_current;
    Token m_previous;
    bool m_hadError;
    bool m_panicMode;
    Scanner *m_sc;
    Chunk *m_chunk;
    static ParseRule m_rules[];  

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
    void emitReturn();
    void emitConstant(Value value);

    //解析前缀表达式
    void grouping();
    void unary();
    

    //解析中缀表达式
    void binary();

    void literal();

    ParseRule* getRule(TokenType type);
    void number(); //指向下面函数的指针
    void string();
    void parsePrecedence(Precedence precedence); //解析给定优先级和更高优先级的表达式
    uint8_t identifierConstant(Token* name);
    void expression();
    void varDeclaration();
    void namedVariable(Token name);
    void variable();
    uint8_t parseVariable(const char* errorMessage);
    void defineVariable(uint8_t global);
    void expressionStatement();
    void printStatement();
    void declaration();
    void statement();
    void endCompiler();

public:
    Parser(const std::string& source, Chunk* chunk);
    ~Parser();

    bool compile();
};

