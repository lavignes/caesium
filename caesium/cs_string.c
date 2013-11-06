#include "cs_string.h"
#include "cs_error.h"
#include "cs_object.h"

CsValue CS_CLASS_STRING;

static int __new(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  RET = CS_EPSILON;
  return 1;
}

int cs_string_add(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  if (!cs_value_isint(OTHER) && OTHER->type == CS_VALUE_STRING) {
    RET = cs_mutator_new_string_formatted(mut, "%s%s", 
      cs_value_tostring(SELF), cs_value_tostring(OTHER));
    return 1;
  }
  
  cs_mutator_raise(mut, cs_mutator_easy_error(mut,
    CS_CLASS_TYPEERROR, "invalid operands for String.__add"));
  return 0;
}

int cs_string_mul(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  int i;
  CsValue value;
  if (cs_value_isint(OTHER)) {
    if (cs_value_toint(OTHER) <= 0) {
      RET = CS_NIL;
      return 1;
    } else if (cs_value_toint(OTHER) == 1) {
      RET = SELF;
      return 1;
    } else {
      value = cs_mutator_new_string_formatted(mut,
        "%s", cs_value_tostring(SELF));
      for (i = 1; i < cs_value_toint(OTHER); ++i) {
        value = cs_mutator_new_string_formatted(mut, "%s%s", 
        cs_value_tostring(value), cs_value_tostring(SELF));
      }
      RET = value;
      return 1;
    }
  }

  cs_mutator_raise(mut, cs_mutator_easy_error(mut,
    CS_CLASS_TYPEERROR, "invalid operands for String.__mul"));
  return 0;
}

CsValue cs_initclass_string(CsMutator* mut) {
  CsHash* dict = cs_hash_new();
  CsArray* bases = cs_array_new();

  cs_hash_insert(dict, "__new", 5, cs_mutator_new_builtin(mut, __new));

  cs_hash_insert(dict, "__add", 5, cs_mutator_new_builtin(mut, cs_string_add));
  cs_hash_insert(dict, "__mul", 5, cs_mutator_new_builtin(mut, cs_string_mul));

  cs_array_insert(bases, -1, CS_CLASS_OBJECT);
  CS_CLASS_STRING = cs_mutator_new_class(mut, "String", dict, bases);
  return CS_CLASS_STRING;
}

void cs_freeclass_string(CsValue klass) {
  cs_hash_free(klass->dict);
  cs_array_free(klass->bases);
}
