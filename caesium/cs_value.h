#ifndef _CS_VALUE_H_
#define _CS_VALUE_H_

#include "cs_common.h"

typedef enum CsValueType {
  CS_VALUE_NIL,
  CS_VALUE_TRUE,
  CS_VALUE_FALSE,
  CS_VALUE_REAL,
  CS_VALUE_STRING,
} CsValueType;

typedef struct CsValueStruct {
  CsValueType type;
  union {
    double real;
    struct {
      size_t length;
      char* u8data;
    } string;
  };
} CsValueStruct;

typedef CsValueStruct* CsValue;

extern CsValue CS_TRUE;
extern CsValue CS_FALSE;
extern CsValue CS_NIL;

#define CS_VALUE_TYPE(value) value->type

// test whether a value is an integer
#define CS_VALUE_VALISINT(value) ((intptr_t) value) & 0x1)
#define CS_VALUE_VAL2INT(value) ((intptr_t) value) >> 0x1)
#define CS_VALUE_INT2VAL(i) ((CsValue) ((intptr_t) i) << 0x1))

#endif /* _CS_VALUE_H_ */