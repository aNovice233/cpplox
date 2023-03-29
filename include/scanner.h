#pragma once
#include <string>
#include <string.h>

typedef enum {
    // Single-character tokens. 单字符词法
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS,
    TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR,
    // One or two character tokens. 一或两字符词法
    TOKEN_BANG, TOKEN_BANG_EQUAL,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL,
    // Literals. 字母量
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,
    // Keywords. 关键字
    TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE,
    TOKEN_FOR, TOKEN_FUN, TOKEN_IF, TOKEN_NIL, TOKEN_OR,
    TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
    TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE,

    TOKEN_ERROR, TOKEN_EOF
} TokenType;

typedef struct{
    TokenType type;
    const char* start;
    int length;
    int line;
}Token;

class Scanner{
    const char* m_start;      //指向的值不能变，可变更指向对象。
    const char* m_current;
    int m_line;

private:
    bool isAtEnd();
    Token makeToken(TokenType type);
    Token errorToken(const char* message);
    char advance();
    char peek();
    char peekNext();
    Token string();
    bool match(char expected);
    void skipWhitespace();
    Token number();
    Token identifier();     //利用字典树(状态机)来判断是否是标识符或关键字
    TokenType identifierType();     
    TokenType checkKeyword(int start, int length, const char* rest, TokenType type);

public:
    Scanner(const std::string& source);
    ~Scanner();

    Token scanToken();
};