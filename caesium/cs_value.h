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
  CS_VALUE_CLASS,
  CS_VALUE_INSTANCE,
  CS_VALUE_BUILTIN,
} CsValueType;

typedef struct CsValueString {
  size_t size;
  size_t length;
  const char* u8str;
} CsValueString;

struct CsMutator;
typedef struct CsValueStruct* CsValue;

typedef CsValue (*CsBuiltin0)(struct CsMutator*);
typedef CsValue (*CsBuiltin1)(struct CsMutator*,CsValue);
typedef CsValue (*CsBuiltin2)(struct CsMutator*,CsValue,CsValue);
typedef CsValue (*CsBuiltin3)(struct CsMutator*,CsValue,CsValue,CsValue);

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
        union {
          CsBuiltin0 builtin0;
          CsBuiltin1 builtin1;
          CsBuiltin2 builtin2;
          CsBuiltin3 builtin3;
        };
        CsHash* dict;
        double real;
        CsValueString* string;
      };
    };
    uint8_t padding[CS_VALUE_SIZE];
  };
} CsValueStruct;


extern CsValue CS_NIL;
extern CsValue CS_TRUE;
extern CsValue CS_FALSE;

// test whether a value is an integer
#define cs_value_isint(value) (((intptr_t) value) & 0x1)
#define cs_value_toint(value) (((intptr_t) value) >> 0x1)
#define cs_value_fromint(i) ((CsValue) ((((intptr_t) i) << 0x1) | 0x1))

#define cs_value_toreal(value) ((double) value->real)
#define cs_value_tostring(value) (value->string->u8str)

// Returns the address of a page given a value :)
#define cs_value_getpage(value) (((uintptr_t) value) & ~((uintptr_t) 0x3FFF))
// Returns offset into page buffer tp locate the bitmap for a value
#define cs_value_getbits(value, page) \
  ((((uintptr_t) value) - ((uintptr_t) page->values)) >> 0x5)

void cs_value_cleanup(CsValue value);

#endif /* _CS_VALUE_H_ */
