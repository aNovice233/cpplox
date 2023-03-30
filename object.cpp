#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE(type, count) \
    (type*)reallocate(NULL, 0, sizeof(type) * (count))

Obj::Obj(){
    m_type = OBJ;
    m_next = nullptr;
}

Obj::~Obj(){
    m_next = nullptr;
}

ObjString::ObjString(){
    m_length = 0;
    m_type = OBJ_STRING;
    m_string = '\0';
}

ObjString::~ObjString(){
    m_length = 0;
}

ObjString::ObjString(const char* chars, int length){
    m_type = OBJ_STRING;
    m_string = chars;
    m_string+='\0';
    m_length = length+1;
    m_next = vm.getObjects();
}

static void freeObject(Obj* object) {
    switch (object->m_type) {
        case OBJ_STRING: {
        ObjString* string = (ObjString*)object;
            delete string;
        break;
        }
    }
}

void freeObjects() {
    Obj* object = vm.getObjects();
    while (object != NULL) {
        Obj* next = object->m_next;
        freeObject(object);
        object = next;
    }
}

void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_STRING:
            std::cout<< AS_CSTRING(value);
        break;
    }
}

Obj* allocateObj(ObjType type){
    Obj* object = nullptr;
    switch(type){
        case OBJ_STRING: object = new ObjString;
    }
    object->m_next = vm.getObjects();
    vm.changeObjects(object);
    return object;
}

ObjString* makeString(std::string s, int length){
    ObjString* p = (ObjString*)allocateObj(OBJ_STRING);
    p->m_length = length;
    p->m_string = s;
    return p;   
}

//堆上创建一个ObjString对象并返回其指针
ObjString* copyString(const char* chars, int length) {
    ObjString* p = (ObjString*)allocateObj(OBJ_STRING);
    p->m_length = length;
    p->m_string = *chars++;
    while(--length){
        p->m_string+=*chars++;
    }
    return p;
}