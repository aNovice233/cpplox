#pragma once
#include <vector>
#include <iostream>
#include <iomanip>
#include "common.h"
#include "value.h"



typedef enum{
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_PRINT,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_RETURN,  
} OpCode;

class Chunk{
    std::vector<uint8_t>    m_code;         //操作码数组
    std::vector<Value>      m_constants;    //常量数组
    std::vector<int>        m_lines;        //代码行数

public:
    Chunk();
    ~Chunk();
    void writeChunk(uint8_t byte, int line);  
    int addConstant(Value value);   //添加常数

    int getCount() const;
    int getLine(int offset) const;
    Value getConstant(int offset) const;
    uint8_t* getFirstCode() const;
    uint8_t getCode(int offset) const;
    uint8_t getInstruction(int offset) const;

    void changeCode(int offset, uint8_t content);
};