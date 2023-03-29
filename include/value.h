#pragma once


typedef enum {
  VAL_BOOL,
  VAL_NIL, 
  VAL_NUMBER,
} ValueType;

typedef struct{
    ValueType type;
    union 
    {
        bool boolean;
        double number;
    }as;
}Value;

//检查值的类型
#define IS_BOOL(value)    ((value).type == VAL_BOOL)
#define IS_NIL(value)     ((value).type == VAL_NIL)
#define IS_NUMBER(value)  ((value).type == VAL_NUMBER)

//取对应的值
#define AS_BOOL(value)    ((value).as.boolean)
#define AS_NUMBER(value)  ((value).as.number)

//定义值的类型
#define BOOL_VAL(value)   ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL           ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value)   ((Value){VAL_NUMBER, {.number = value}})

bool valuesEqual(Value a, Value b);

void printValue(Value value);