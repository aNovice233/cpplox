//
// Created by ethereal on 2023/4/2.
//

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
    // if(!m_strings.empty()){
    //     for (auto it = m_strings.begin(); it != m_strings.end(); ++it) {
    //         delete it->second.as.obj; // 销毁 ObjString 对象
    //     }
    // }
    // if(!m_globals.empty()){
    //     for (auto it = m_globals.begin(); it != m_globals.end(); ++it) {
    //         delete it->second.as.obj; // 销毁 ObjString 对象
    //     }
    // }

    m_strings.clear();
    m_globals.clear();
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
#define READ_STRING() AS_STRING(READ_CONSTANT())
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
            case OP_POP: m_stack.pop(); break;
            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                m_stack.push(getStack(slot)); 
                break;
            }
            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                setStack(slot, peek(0));
                break;
            }
            case OP_GET_GLOBAL: {
                ObjString* name = READ_STRING();
                Value value;
                if (!m_globals.count(name->m_string)) {
                    runtimeError("Undefined variable '%s'.", name->m_string);
                    return INTERPRET_RUNTIME_ERROR;
                }
                value = m_globals.find(name->m_string)->second;
                m_stack.push(value);
                break;
            }
            case OP_DEFINE_GLOBAL: {
                ObjString* name = READ_STRING();
                m_globals.emplace(name->m_string, peek(0));
                m_stack.pop();
                break;
            }
            case OP_SET_GLOBAL: {
                ObjString* name = READ_STRING();
                if (!m_globals.count(name->m_string)) {
                    runtimeError("Undefined variable '%s'.", name->m_string);
                    return INTERPRET_RUNTIME_ERROR;
                }
                m_globals.erase(name->m_string);
                m_globals.emplace(name->m_string, peek(0));
                break;
            }
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
            case OP_PRINT: {
                printValue(m_stack.top());
                m_stack.pop();
                std::cout<< std::endl;
                break;
            }
            case OP_RETURN: {
                // Exit interpreter.
                return INTERPRET_OK;
            }
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
}

int VM::countString(const char* s){
    std::string str = s;
    return m_strings.count(s);
}

void VM::insertString(const char* s, Value value){
    std::string str = s;
    m_strings.emplace(str, value);
}

Value VM::getString(const char* s){
    std::string str = s;
    auto it = m_strings.find(s);
    return it->second;
}

Value VM::getStack(uint8_t index){
    Value* stackBottom;
    int size = m_stack.size();
    Value* stackTop;
    stackTop = &m_stack.top();
    stackBottom = &stackTop[-size+1];
    return stackBottom[index];
}

void VM::setStack(uint8_t index, Value value){
    Value* stackBottom;
    int size = m_stack.size();
    Value* stackTop;
    stackTop = &m_stack.top();
    stackBottom = &stackTop[-size+1];
    stackBottom[index] = value;
}

void VM::changeObjects(Obj* object){
    m_objects = object;
}

Obj* VM::getObjects(){
    return m_objects;
}

InterpretResult VM::interpret(const std::string& source){
    Chunk chunk;
    Compiler compiler(source, &chunk);
    //创建空的chunk，传给编译器，编译器来填充
    if(!compiler.compile()){
        return INTERPRET_COMPILE_ERROR;
    }

    m_chunk = &chunk;
    m_ip = m_chunk->getFirstCode();

    InterpretResult result = run();
    resetStack();
    return result;
}