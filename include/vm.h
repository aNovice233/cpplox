#pragma once
#include "chunk.h"
#include <stack>
#include <unordered_map>
#include <unordered_set>

typedef enum{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
}InterpretResult;

class VM{
    Chunk *m_chunk;
    uint8_t *m_ip;
    std::stack<Value> m_stack;
    Obj*    m_objects;
    std::unordered_map<std::string, Value> m_globals;   //后期绑定（编译后分析）
    std::unordered_map<std::string, Value> m_strings;

private:
    InterpretResult run();
    void runtimeError(const char* format, ...);
    Value peek(int distance);//返回从栈顶起的第几个元素，0是第一个
    void resetStack();
    void printTop();
    void concatenate();

public:
    VM();
    ~VM();

    int countString(const char* s);
    void insertString(const char* s, Value value);
    Value getString(const char* s);
    void changeObjects(Obj* object);
    Value getStack(uint8_t index);
    void setStack(uint8_t index, Value value);
    Obj* getObjects();
    InterpretResult interpret(const std::string& source);
};

extern class VM vm;