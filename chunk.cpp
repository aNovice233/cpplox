#include <stdlib.h>
#include "chunk.h"
#include "value.h"

Chunk::Chunk(){
    
}

Chunk::~Chunk(){
    m_code.clear();
    m_constants.clear();
    m_lines.clear();
}

void Chunk::writeChunk(uint8_t byte, int line){
    m_code.push_back(byte);
    m_lines.push_back(line);
}

int Chunk::addConstant(Value value){
    m_constants.push_back(value);
    return m_constants.size()-1;
}

int Chunk::getCount() const{
    return m_code.size();
}

int Chunk::getLine(int offset) const{
    return m_lines[offset];
}

Value Chunk::getConstant(int offset) const{
    return m_constants[offset];
}

uint8_t* Chunk::getFirstCode() const{
    if (m_code.size() > 0) {
        return const_cast<uint8_t*>(&m_code[0]);
    } else {
        return nullptr; // 空 vector 的情况
    }
}

uint8_t Chunk::getCode(int offset) const{
    return m_code[offset];
}

uint8_t Chunk::getInstruction(int offset) const{
    return m_code[offset];
}