#pragma once
#include "chunk.h"
#include <stack>

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

    void changeObjects(Obj* object);
    Obj* getObjects();
    InterpretResult interpret(const std::string& source);
};

extern class VM vm;