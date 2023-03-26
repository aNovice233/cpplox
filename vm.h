#pragma once
#include "chunk.h"
#include <stack>

typedef enum{
    INTERPRET_OK,
    INTEPRET_COMPILE_ERROR,
    INTEPRET_RUNTIME_ERROR
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

    InterpretResult interpret(const Chunk &chunk);
};