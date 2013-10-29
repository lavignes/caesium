#include "cs_value.h"

static CsValueStruct _CS_NIL = {
  type: CS_VALUE_NIL,
};

static CsValueStruct _CS_TRUE = {
  type: CS_VALUE_TRUE,
};

static CsValueStruct _CS_FALSE = {
  type: CS_VALUE_FALSE,
};

CsValue CS_NIL = &_CS_NIL;
CsValue CS_TRUE = &_CS_TRUE;
CsValue CS_FALSE = &_CS_FALSE;

void cs_value_cleanup(CsValue value) {
  switch (value->type) {
    case CS_VALUE_STRING:
      free((char*) value->string->u8str);
      free(value->string);
      break;

    case CS_VALUE_INSTANCE:
      cs_hash_free(value->dict);
      break;

    default:
      break;
  }
}
