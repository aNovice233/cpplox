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

private:
    InterpretResult run();

public:
    VM();
    ~VM();

    InterpretResult interpret(const std::string& source);
};