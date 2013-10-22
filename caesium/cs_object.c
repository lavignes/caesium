#include "cs_object.h"

CsValue CS_CLASS_OBJECT;

static CsValue cs_method___test(CsMutator* mut, CsValue self) {
  return self;
}

CsValue cs_initclass_object(CsMutator* mut) {
  CsHash* dict = cs_hash_new();
  CsArray* bases = cs_array_new();
  CsValue __test = cs_mutator_new_builtin1(mut, cs_method___test);
  cs_hash_insert(dict, "__test", 6, __test);
  CS_CLASS_OBJECT = cs_mutator_new_class(mut, "Object", dict, bases);
  return CS_CLASS_OBJECT;
}

void cs_freeclass_object(CsValue klass) {
  cs_hash_free(klass->dict);
  cs_array_free(klass->bases);
}