#include "cs_object.h"

CsValue CS_CLASS_OBJECT;

static int __bool(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  RET = CS_TRUE;
  return 1;
}

static int __new(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  RET = cs_mutator_new_instance(mut, SELF);
  return 1;
}

CsValue cs_initclass_object(CsMutator* mut) {
  CsHash* dict = cs_hash_new();
  CsArray* bases = cs_array_new();

  cs_hash_insert(dict, "__new", 5, cs_mutator_new_builtin(mut, __new));
  cs_hash_insert(dict, "__bool", 6, cs_mutator_new_builtin(mut, __bool));

  CS_CLASS_OBJECT = cs_mutator_new_class(mut, "Object", dict, bases);
  return CS_CLASS_OBJECT;
}

void cs_freeclass_object(CsValue klass) {
  cs_hash_free(klass->dict);
  cs_array_free(klass->bases);
}