#include "common.h" 
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main(int argc, char* argv[]){
    Chunk chunk;
    VM vm;
    int constantIndex = chunk.addConstant(1.2);
    chunk.writeChunk( OP_CONSTANT, 123);
    chunk.writeChunk( constantIndex, 123);

    constantIndex = chunk.addConstant(3.4);
    chunk.writeChunk(OP_CONSTANT, 123);
    chunk.writeChunk(constantIndex,123);
    chunk.writeChunk(OP_ADD, 123);

    constantIndex = chunk.addConstant(5.6);
    chunk.writeChunk(OP_CONSTANT, 123);
    chunk.writeChunk(constantIndex, 123);

    chunk.writeChunk(OP_DIVIDE, 123);

    chunk.writeChunk(OP_NEGATE, 123);
    chunk.writeChunk(OP_RETURN, 123);
    disassembleChunk(chunk, "test chunk");
    vm.interpret(chunk);
    return 0;
}