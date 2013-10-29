#include "cs_string.h"
#include "cs_error.h"
#include "cs_object.h"

CsValue CS_CLASS_STRING;

CsValue cs_string_add(CsMutator* mut, CsValue self, CsValue other) {
  switch (other->type) {
    case CS_VALUE_STRING:
      return cs_mutator_new_string_formatted(mut, "%s%s", 
        cs_value_tostring(self), cs_value_tostring(other));
      break;

    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_TYPEERROR, "invalid operands for String.__add"));
      return NULL;
  }
}

CsValue cs_string_mul(CsMutator* mut, CsValue self, CsValue other) {
  int i;
  CsValue value;
  if (cs_value_isint(other)) {
    if (cs_value_toint(other) <= 0) {
      return CS_NIL;
    } else if (cs_value_toint(other) == 1) {
      return self;
    } else {
      value = cs_mutator_new_string_formatted(mut,
        "%s", cs_value_tostring(self));
      for (i = 1; i < cs_value_toint(other); ++i) {
        value = cs_mutator_new_string_formatted(mut, "%s%s", 
        cs_value_tostring(value), cs_value_tostring(self));
      }
      return value;
    }
  }

  cs_mutator_raise(mut, cs_mutator_easy_error(mut,
    CS_CLASS_TYPEERROR, "invalid operands for String.__mul"));
  return NULL;
}

CsValue cs_initclass_string(CsMutator* mut) {
  CsHash* dict = cs_hash_new();
  CsArray* bases = cs_array_new();

  cs_hash_insert(dict, "__add", 5, cs_mutator_new_builtin2(mut, cs_string_add));
  cs_hash_insert(dict, "__mul", 5, cs_mutator_new_builtin2(mut, cs_string_mul));

  cs_array_insert(bases, -1, CS_CLASS_OBJECT);
  CS_CLASS_STRING = cs_mutator_new_class(mut, "String", dict, bases);
  return CS_CLASS_STRING;
}

void cs_freeclass_string(CsValue klass) {
  cs_hash_free(klass->dict);
  cs_array_free(klass->bases);
}
