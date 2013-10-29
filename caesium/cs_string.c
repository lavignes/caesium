#include "cs_string.h"
#include "cs_object.h"

CsValue CS_CLASS_STRING;

CsValue cs_initclass_string(CsMutator* mut) {
  CsHash* dict = cs_hash_new();
  CsArray* bases = cs_array_new();

  cs_array_insert(bases, -1, CS_CLASS_OBJECT);
  CS_CLASS_STRING = cs_mutator_new_class(mut, "String", dict, bases);
  return CS_CLASS_STRING;
}

void cs_freeclass_string(CsValue klass) {
  cs_hash_free(klass->dict);
  cs_array_free(klass->bases);
}
