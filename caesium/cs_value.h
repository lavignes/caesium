#ifndef _CS_VALUE_H_
#define _CS_VALUE_H_

#include "cs_common.h"
#include "cs_hash.h"
#include "cs_array.h"

#define CS_VALUE_SIZE 32

typedef enum CsValueType {
  CS_VALUE_NIL,
  CS_VALUE_TRUE,
  CS_VALUE_FALSE,
  CS_VALUE_REAL,
  CS_VALUE_STRING,
  CS_VALUE_ARRAY,
  CS_VALUE_CLASS,
  CS_VALUE_INSTANCE,
  CS_VALUE_BUILTIN,
  CS_VALUE_CLOSURE,
} CsValueType;

typedef struct CsValueString {
  size_t size;
  size_t length;
  const char* u8str;
} CsValueString;

struct CsMutator;
struct CsClosure;
typedef struct CsValueStruct* CsValue;


/**
 *  Builtin function:
 *  argc => argument count
 *  args => argument list
 *  retc => expected returns count
 *  rets => returns list
 *  returns number of filled rets or 0 if exception occurrs
 */
typedef int (*CsBuiltin)(struct CsMutator*,
  int argc, CsValue* args, int retc, CsValue* rets);

#define SELF args[0]
#define OTHER args[1]
#define RET rets[0]

typedef struct CsValueStruct {
  union {
    struct {
      CsValueType type;
      uint32_t hash;
      union {
        struct {
          CsArray* bases;
          const char* classname;
        };
        CsValue klass;
      };
      union {
        CsBuiltin builtin;
        CsHash* dict;
        CsArray* array;
        double real;
        CsValueString* string;
        struct CsClosure* closure;
      };
    };
    uint8_t padding[CS_VALUE_SIZE];
  };
} CsValueStruct;


extern CsValue CS_NIL;
extern CsValue CS_TRUE;
extern CsValue CS_FALSE;
extern CsValue CS_EPSILON; // The empty string

// test whether a value is an integer
#define cs_value_isint(value) (((intptr_t) value) & 0x1)
#define cs_value_toint(value) (((intptr_t) value) >> 0x1)
#define cs_value_fromint(i) ((CsValue) ((((intptr_t) i) << 0x1) | 0x1))

#define cs_value_toreal(value) (value->real)
#define cs_value_toutf8(value) (value->string->u8str)
#define cs_value_tostring(value) (value->string)
#define cs_value_toarray(value) (value->array)

// Returns the address of a page given a value :)
#define cs_value_getpage(value) (((uintptr_t) value) & ~((uintptr_t) 0x3FFF))
// Returns offset into page buffer tp locate the bitmap for a value
#define cs_value_getbits(value, page) \
  ((((uintptr_t) value) - ((uintptr_t) page->values)) >> 0x5)

void cs_value_cleanup(CsValue value);

#endif /* _CS_VALUE_H_ */
