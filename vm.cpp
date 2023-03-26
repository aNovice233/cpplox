#include "vm.h"
#include "debug.h"

VM::VM(){
    m_chunk = nullptr;
    m_ip = nullptr;
}

VM::~VM(){
    m_chunk = nullptr;
    m_ip = nullptr;
    while(!m_stack.empty())
        m_stack.pop();
}

InterpretResult VM::run(){
#define READ_BYTE() (*m_ip++)
#define READ_CONSTANT() (m_chunk->getConstant(READ_BYTE()))
#define BINARY_OP(op) \
        do{ \
            Value b = m_stack.top();  \
            m_stack.pop();    \
            Value a = m_stack.top();   \
            m_stack.pop();    \
            m_stack.push(a op b);   \
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
        std::cout<<"[ "<<out.top()<<" ]";
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
                std::cout<<constant<<std::endl;
                break;
            }
            case OP_ADD:         BINARY_OP(+); break;
            case OP_SUBTRACT:    BINARY_OP(-); break;
            case OP_MULTIPLY:    BINARY_OP(*); break;
            case OP_DIVIDE:      BINARY_OP(/); break;
            case OP_NEGATE: {
                Value tmp = m_stack.top();
                m_stack.pop();
                m_stack.push(-tmp);
                break;
            }
            case OP_RETURN: {
                std::cout<<m_stack.top();
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

InterpretResult VM::interpret(const Chunk &chunk){
    m_chunk = new Chunk(chunk);
    m_ip = m_chunk->getFirstCode();
    return run();
}