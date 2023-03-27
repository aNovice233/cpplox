#include "scanner.h"

static bool isDigit(char c){
    return c>='0' && c<='9';
}

static bool isAlpha(char c) {
  return (c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z') ||
          c == '_';
}

bool Scanner::isAtEnd(){
    return *m_current == '\0';
}

char Scanner::advance(){
    m_current++;
    return m_current[-1];
}

char Scanner::peek(){
    return *m_current;
}

char Scanner::peekNext(){
    if(isAtEnd())return '\0';
    return m_current[1];
}

bool Scanner::match(char expected){
    if(isAtEnd())return false;
    if(*m_current != expected) return false;
    m_current++;
    return true;
}

Token Scanner::makeToken(TokenType type){
    Token token;
    token.type = type;
    token.start = m_start;
    token.length = (int)(m_current - m_start);
    token.line = m_line;
    return token;
}

Token Scanner::errorToken(const char* message){
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = m_line;
    return token;
}

void Scanner::skipWhitespace(){
    for(;;){
        char c = peek();
        switch (c) {
        case ' ':
        case '\r':
        case '\t':
            advance();
            break;
        case '\n':
            m_line++;
            advance();
            break;
        case '/':
            if (peekNext() == '/') {
            // 注释可以当作空格来处理
            // 若第二个不是 / 则上一个 / 不进行消费
            while (peek() != '\n' && !isAtEnd()) advance();
            } else {
            return;
            }
            break;
        default:
            return;
        }
    }
}

TokenType Scanner::checkKeyword(int start, int length, const char* rest, TokenType type) {
  if (m_current - m_start == start + length && 
        memcmp(m_start + start, rest, length) == 0) {
    return type;
  }

  return TOKEN_IDENTIFIER;
}

TokenType Scanner::identifierType(){
    switch (m_start[0]) {
        case 'a': return checkKeyword(1, 2, "nd", TOKEN_AND);
        case 'c': return checkKeyword(1, 4, "lass", TOKEN_CLASS);
        case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
        case 'f':
            if (m_current - m_start > 1) {
                switch (m_start[1]) {
                case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE);
                case 'o': return checkKeyword(2, 1, "r", TOKEN_FOR);
                case 'u': return checkKeyword(2, 1, "n", TOKEN_FUN);
                }
            }
            break;
        case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
        case 'n': return checkKeyword(1, 2, "il", TOKEN_NIL);
        case 'o': return checkKeyword(1, 1, "r", TOKEN_OR);
        case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT);
        case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
        case 's': return checkKeyword(1, 4, "uper", TOKEN_SUPER);
        case 't':
            if (m_current - m_start > 1) {
                switch (m_start[1]) {
                case 'h': return checkKeyword(2, 2, "is", TOKEN_THIS);
                case 'r': return checkKeyword(2, 2, "ue", TOKEN_TRUE);
                }
            }
            break;
        case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
        case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
  }
    return TOKEN_IDENTIFIER;
}

Token Scanner::identifier(){
    while (isAlpha(peek()) || isDigit(peek())) advance();
    return makeToken(identifierType());
}

Token Scanner::number(){
    while(isDigit(peek()))advance();

    //小数部分
    if(peek() == '.' && isDigit(peekNext())){
        // 消费 ‘.’
        advance();
        while(isDigit(peek()))advance();
    }

    return makeToken(TOKEN_NUMBER);
}

Token Scanner::string(){
    while(peek() != '"' && !isAtEnd()){
        if(peek() == '\n') m_line++;
        advance();
    }

    if(isAtEnd()) return errorToken("Unterminated string.");

    advance();
    return makeToken(TOKEN_STRING);
}

Scanner::Scanner(const std::string source){
    m_start = &source[0];
    m_current = &source[0];
    m_line = 1;
}

Scanner::~Scanner(){
    m_current = nullptr;
    m_start = nullptr;
    m_line = -1;
}

Token Scanner::scanToken(){
    skipWhitespace();

    m_start = m_current;
    if(isAtEnd()) return makeToken(TOKEN_EOF);
    char c = advance();

    if (isAlpha(c)) return identifier();
    if (isDigit(c)) return number();
    switch(c){
        case '(': return makeToken(TOKEN_LEFT_PAREN);
        case ')': return makeToken(TOKEN_RIGHT_PAREN);
        case '{': return makeToken(TOKEN_LEFT_BRACE);
        case '}': return makeToken(TOKEN_RIGHT_BRACE);
        case ';': return makeToken(TOKEN_SEMICOLON);
        case ',': return makeToken(TOKEN_COMMA);
        case '.': return makeToken(TOKEN_DOT);
        case '-': return makeToken(TOKEN_MINUS);
        case '+': return makeToken(TOKEN_PLUS);
        case '/': return makeToken(TOKEN_SLASH);
        case '*': return makeToken(TOKEN_STAR);
        case '!':
            return makeToken(
          match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return makeToken(
            match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
            return makeToken(
            match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            return makeToken(
            match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '"': return string();
    }
    return errorToken("Unexpected character.");
}


