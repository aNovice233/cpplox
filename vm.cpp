#include <stdarg.h>
#include "vm.h"
#include "debug.h"
#include "value.h"
#include "object.h"
#include "compiler.h"

VM::VM(){
    m_chunk = nullptr;
    m_ip = nullptr;
    m_objects = nullptr;
}

VM::~VM(){
    m_chunk = nullptr;
    m_ip = nullptr;
    while(!m_stack.empty())
        m_stack.pop();
    freeObjects();
}

void VM::resetStack(){
    while(!m_stack.empty())
        m_stack.pop();
}

void VM::printTop(){
    printValue(m_stack.top());
}

void VM::runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = m_ip - m_chunk->getFirstCode() - 1;
    int line = m_chunk->getLine(instruction);
    fprintf(stderr, "[line %d] in script\n", line);
    resetStack();
}

Value VM::peek(int distance){
    // if(distance == 0)return m_stack.top();
    // std::stack<Value> out;
    // do{
    //     out.push(m_stack.top());
    //     m_stack.pop();
    // }while(distance--);
    // Value v = out.top();
    // while(!out.empty()){
    //     m_stack.push(out.top());
    //     out.pop();
    // }
    // return v;
    Value* stackTop;
    stackTop = &m_stack.top();
    return stackTop[-distance];
}

static bool isFalsey(Value value){
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

void VM::concatenate() {
    ObjString* b = AS_STRING(m_stack.top());
    m_stack.pop();
    ObjString* a = AS_STRING(m_stack.top());
    m_stack.pop();
    std::string chars = a->m_string;
    chars+=b->m_string;
    ObjString* result = makeString(chars, chars.length());
    m_stack.push(OBJ_VAL(result));
}

InterpretResult VM::run(){
#define READ_BYTE() (*m_ip++)
#define READ_CONSTANT() (m_chunk->getConstant(READ_BYTE()))
#define BINARY_OP(valueType, op) \
        do{ \
            if(!IS_NUMBER(peek(0))|| !IS_NUMBER(peek(1))){ \
                runtimeError("Operands must be numbers."); \
                return INTERPRET_RUNTIME_ERROR; \
            } \
            double b = AS_NUMBER(m_stack.top());  \
            m_stack.pop();    \
            double a = AS_NUMBER(m_stack.top());   \
            m_stack.pop();    \
            m_stack.push(NUMBER_VAL(a op b));   \
        }while(false)

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    std::stack<Value> out;
    while(!m_stack.empty()){
        out.push(m_stack.top());
        m_stack.pop();
    }
    std::cout<<"           ";
    while(!out.empty()){
        std::cout<<"[ ";
        printValue(out.top());
        std::cout<<" ]";
        m_stack.push(out.top());
        out.pop();
    }
    std::cout<<std::endl;
    disassembleInstruction(*m_chunk,
                           (int)(m_ip - m_chunk->getFirstCode()));
#endif
        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT:{
                Value constant = READ_CONSTANT();
                m_stack.push(constant);
                printValue(constant);
                std::cout<<std::endl;
                break;
            }
            case OP_NIL: m_stack.push(NIL_VAL); break;
            case OP_TRUE: m_stack.push(BOOL_VAL(true)); break;
            case OP_FALSE: m_stack.push(BOOL_VAL(false)); break;
            case OP_EQUAL: {
                Value b = m_stack.top();
                m_stack.pop();
                Value a = m_stack.top();
                m_stack.pop();
                m_stack.push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }
            case OP_GREATER:  BINARY_OP(BOOL_VAL, >); break;
            case OP_LESS:     BINARY_OP(BOOL_VAL, <); break;
            case OP_ADD: {
                if (IS_OBJ(peek(0)) && IS_OBJ(peek(1))) {
                    if (IS_STRING(peek(0)) && IS_STRING(peek(1)))
                        concatenate();
                } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                    double b = AS_NUMBER(m_stack.top());
                    m_stack.pop();
                    double a = AS_NUMBER(m_stack.top());
                    m_stack.pop();
                    m_stack.push(NUMBER_VAL(a + b));
                } else {
                runtimeError(
                    "Operands must be two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE:   BINARY_OP(NUMBER_VAL, /); break;
            case OP_NOT:{
                Value b = BOOL_VAL(isFalsey(m_stack.top()));
                m_stack.pop();
                m_stack.push(b);
                break;
            }
            case OP_NEGATE: {
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                Value tmp = m_stack.top();
                m_stack.pop();
                m_stack.push(NUMBER_VAL(-AS_NUMBER(tmp)));
                break;
            }
            case OP_RETURN: {
                printTop();
                m_stack.pop();
                std::cout<<std::endl;
                return INTERPRET_OK;
            }
        }
    }

#undef READ_BYTE
#undef BINARY_OP
#undef READ_CONSTANT
}

void VM::changeObjects(Obj* object){
    m_objects = object;
}

Obj* VM::getObjects(){
    return m_objects;
}

InterpretResult VM::interpret(const std::string& source){
    Chunk chunk;
    Parser parser(source, &chunk);
    //创建空的chunk，传给编译器，编译器来填充
    std::cout<<std::endl<<"interpret source:"<<source<<std::endl;
    if(!parser.compile()){
        return INTERPRET_COMPILE_ERROR;
    }

    m_chunk = &chunk;
    m_ip = m_chunk->getFirstCode();

    InterpretResult result = run();
    resetStack();
    return result;
}