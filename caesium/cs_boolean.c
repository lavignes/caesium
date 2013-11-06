#include "cs_boolean.h"
#include "cs_object.h"

CsValue CS_CLASS_TRUE;
CsValue CS_CLASS_FALSE;

static int __true(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  if (retc)
    RET = CS_TRUE;
  return 1;
}

CsValue cs_initclass_true(CsMutator* mut) {
  CsHash* dict = cs_hash_new();
  CsArray* bases = cs_array_new();

  cs_hash_insert(dict, "__bool", 6, cs_mutator_new_builtin(mut, __true));

  cs_array_insert(bases, -1, CS_CLASS_OBJECT);
  CS_CLASS_TRUE = cs_mutator_new_class(mut, "TrueClass", dict, bases);
  return CS_CLASS_TRUE;
}

void cs_freeclass_true(CsValue klass) {
  cs_hash_free(klass->dict);
  cs_array_free(klass->bases);
}

static int __false(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  if (retc)
    RET = CS_TRUE;
  return 1;
}

CsValue cs_initclass_false(CsMutator* mut) {
  CsHash* dict = cs_hash_new();
  CsArray* bases = cs_array_new();

  cs_hash_insert(dict, "__bool", 6, cs_mutator_new_builtin(mut, __false));

  cs_array_insert(bases, -1, CS_CLASS_OBJECT);
  CS_CLASS_FALSE = cs_mutator_new_class(mut, "FalseClass", dict, bases);
  return CS_CLASS_FALSE;
}

void cs_freeclass_false(CsValue klass) {
  cs_hash_free(klass->dict);
  cs_array_free(klass->bases);
}
