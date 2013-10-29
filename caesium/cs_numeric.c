#include "cs_object.h"
#include "cs_numeric.h"
#include "cs_error.h"

CsValue CS_CLASS_INT;
CsValue CS_CLASS_REAL;

CsValue cs_initclass_int(CsMutator* mut) {
  CsHash* dict = cs_hash_new();
  CsArray* bases = cs_array_new();

  cs_array_insert(bases, -1, CS_CLASS_OBJECT);
  CS_CLASS_INT = cs_mutator_new_class(mut, "Integer", dict, bases);
  return CS_CLASS_INT;
}

void cs_freeclass_int(CsValue klass) {
  cs_hash_free(klass->dict);
  cs_array_free(klass->bases);
}

CsValue cs_initclass_real(CsMutator* mut) {
  CsHash* dict = cs_hash_new();
  CsArray* bases = cs_array_new();

  cs_array_insert(bases, -1, CS_CLASS_OBJECT);
  CS_CLASS_REAL = cs_mutator_new_class(mut, "Real", dict, bases);
  return CS_CLASS_REAL;
}

void cs_freeclass_real(CsValue klass) {
  cs_hash_free(klass->dict);
  cs_array_free(klass->bases);
}

CsValue cs_int_add(CsMutator* mut, CsValue x, CsValue y) {
  if (cs_value_isint(y))
    return cs_value_fromint(cs_value_toint(x) + cs_value_toint(y));

  switch (y->type) {
    case CS_VALUE_REAL:
      return cs_mutator_new_real(mut,
        ((double) cs_value_toint(x)) + cs_value_toreal(y));

    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut, CS_CLASS_TYPEERROR, 
        "invalid operands for add"));
      return NULL;
  }
}

CsValue cs_real_add(CsMutator* mut, CsValue x, CsValue y) {
  if (cs_value_isint(y))
    return cs_mutator_new_real(mut, 
      cs_value_toreal(x) + (double) cs_value_toint(y));

  switch (y->type) {
    case CS_VALUE_REAL:
      return cs_mutator_new_real(mut, cs_value_toreal(x) + cs_value_toreal(y));

    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut, CS_CLASS_TYPEERROR, 
        "invalid operands for add"));
      return NULL;
  }
}
