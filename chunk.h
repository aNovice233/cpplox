#pragma once
#include <vector>
#include "common.h"
#include <iostream>
#include <iomanip>

typedef double Value;

typedef enum{
    OP_CONSTANT,
    OP_NEGATE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_RETURN,  //从当前函数返回
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
    uint8_t getInstruction(int offset) const;

    void printValue(int offset) const;
};