#pragma once
#include <string>
#include <iostream>
#include "common.h"
#include "value.h"

//取obj的类型
#define OBJ_TYPE(value)        (AS_OBJ(value)->m_type)

//接收Value，因为虚拟机中都用的Value
#define IS_STRING(value)       AS_OBJ(value)->isObjType(OBJ_STRING)

//接收Value
#define AS_STRING(value)       ((ObjString*)AS_OBJ(value)) //返回ObjString指针
#define AS_CSTRING(value)      (((ObjString*)AS_OBJ(value))->m_string) //返回ObjString下的string

typedef enum{
    OBJ_STRING,
    OBJ
}ObjType;

class Obj{
public:
    ObjType m_type;
    Obj* m_next;

    Obj();
    virtual ~Obj();
    bool isObjType(ObjType type){
        if (m_type == type) return true;
        else return false;
    }
};

class ObjString: public Obj{
public:
    std::string m_string;
    int m_length;

    ObjString();
    ObjString(const char* chars, int length);
    virtual ~ObjString();
};

ObjString* copyString(const char* chars, int length);

void printObject(Value value);

Obj* allocateObj(ObjType type);

ObjString* makeString(std::string s, int length);

void freeObjects();