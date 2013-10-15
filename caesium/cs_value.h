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
  union {
    struct {
      CsValueType type;
      union {
        double real;
        struct {
          size_t size;
          const char* string;
        };
      };
    };
    uint8_t padding[32];
  };
} CsValueStruct;

typedef struct CsValueStruct* CsValue;

extern CsValue CS_TRUE;
extern CsValue CS_FALSE;
extern CsValue CS_NIL;

#define cs_value_get_type(value) (value->type)

// test whether a value is an integer
#define cs_value_isint(value) (((intptr_t) value) & 0x1)
#define cs_value_toint(value) (((intptr_t) value) >> 0x1)
#define cs_value_fromint(i) ((CsValue) ((((intptr_t) i) << 0x1) | 0x1))

#define cs_value_toreal(value) ((double) value->real)
#define cs_value_tostring(value) ((const char*) value->string)

#define cs_value_getpage(value) (((uintptr_t) value) & ~((uintptr_t) 0x3FFF))
#define cs_value_getbits(value, nursery) \
  ((((uintptr_t) value) - ((uintptr_t) nursery)) >> 0x5)

#endif /* _CS_VALUE_H_ */
