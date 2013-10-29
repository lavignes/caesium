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
        CS_CLASS_TYPEERROR, "invalid operands for add"));
      return NULL;
  }
}

CsValue cs_initclass_string(CsMutator* mut) {
  CsHash* dict = cs_hash_new();
  CsArray* bases = cs_array_new();

  cs_hash_insert(dict, "__add", 5, cs_mutator_new_builtin2(mut, cs_string_add));

  cs_array_insert(bases, -1, CS_CLASS_OBJECT);
  CS_CLASS_STRING = cs_mutator_new_class(mut, "String", dict, bases);
  return CS_CLASS_STRING;
}

void cs_freeclass_string(CsValue klass) {
  cs_hash_free(klass->dict);
  cs_array_free(klass->bases);
}
