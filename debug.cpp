#include "debug.h"

void disassembleChunk(const Chunk &chunk, const char* name){
    std::cout << "== " << name << " ==" << std::endl;
    for(int offset=0; offset<chunk.getCount();)
        offset = disassembleInstruction(chunk, offset);
}

static int constantInstruction(const char *name, const Chunk &chunk, int offset){
    //constant 常数下标
    uint8_t constantIndex = chunk.getInstruction(offset + 1);
    printf("%-16s %4d ",name, constantIndex);
    printValue(chunk.getConstant(constantIndex));
    std::cout<<std::endl;
    return offset + 2;
}

static int simpleInstruction(const char* name, int offset) {
  std::cout<<name<<std::endl;
  return offset + 1;
}

static int byteInstruction(const char* name, const Chunk& chunk,
                           int offset) {
  uint8_t slot = chunk.getCode(offset + 1);
  printf("%-16s %4d\n", name, slot);
  return offset + 2; 
}

static int jumpInstruction(const char* name, int sign,
                           const Chunk& chunk, int offset) {
  uint16_t jump = (uint16_t)(chunk.getCode(offset + 1) << 8);
  jump |= chunk.getCode(offset + 2);
  printf("%-16s %4d -> %d\n", name, offset,
         offset + 3 + sign * jump);
  return offset + 3;
}

int disassembleInstruction(const Chunk &chunk, int offset){
    std::cout << std::setw(4) << std::setfill('0') << offset<<" ";
    //与上一条代码同一行，打印 |
    if(offset > 0 && chunk.getLine(offset) == chunk.getLine(offset-1)){
        std::cout<<"   | ";
    }else{
        std::cout<<std::setw(4)<<std::setfill('0')<<chunk.getLine(offset)<<" ";
    }
    uint8_t instruction = chunk.getInstruction(offset);
    switch (instruction)
    {
        case OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", chunk, offset);
        case OP_NIL:
            return simpleInstruction("OP_NIL", offset);
        case OP_TRUE:
            return simpleInstruction("OP_TRUE", offset);
        case OP_FALSE:
            return simpleInstruction("OP_FALSE", offset);
        case OP_POP:
            return simpleInstruction("OP_POP", offset);
        case OP_GET_LOCAL:
            return byteInstruction("OP_GET_LOCAL", chunk, offset);
        case OP_SET_LOCAL:
            return byteInstruction("OP_SET_LOCAL", chunk, offset);
        case OP_GET_GLOBAL:
            return constantInstruction("OP_GET_GLOBAL", chunk, offset);
        case OP_DEFINE_GLOBAL:
            return constantInstruction("OP_DEFINE_GLOBAL", chunk, offset);
        case OP_SET_GLOBAL:
            return constantInstruction("OP_SET_GLOBAL", chunk, offset);
        case OP_EQUAL:
            return simpleInstruction("OP_EQUAL", offset);
        case OP_GREATER:
            return simpleInstruction("OP_GREATER", offset);
        case OP_LESS:
            return simpleInstruction("OP_LESS", offset);
        case OP_ADD:
            return simpleInstruction("OP_ADD", offset);break;
        case OP_SUBTRACT:
            return simpleInstruction("OP_SUBTRACT", offset);
        case OP_MULTIPLY:
            return simpleInstruction("OP_MULTIPLY", offset);
        case OP_DIVIDE:
            return simpleInstruction("OP_DIVIDE", offset);
        case OP_NOT:
            return simpleInstruction("OP_NOT", offset);
        case OP_NEGATE:
            return simpleInstruction("OP_NEGATE", offset);
        case OP_PRINT:
            return simpleInstruction("OP_PRINT", offset);
        case OP_JUMP:
            return jumpInstruction("OP_JUMP", 1, chunk, offset);
        case OP_JUMP_IF_FALSE:
            return jumpInstruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
        case OP_LOOP:
            return jumpInstruction("OP_LOOP", -1, chunk, offset);
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        break;    
        default:
            std::cout<<"unknown opcode: "<< instruction;
        break;
    }
    return 1;
}
