#include "cs_error.h"
#include "cs_object.h"

CsValue CS_CLASS_ERROR;

static CsValue error_bases[] = {NULL, NULL};

CsValue cs_initclass_error(CsMutator* mut) {
  CsHash* dict = cs_hash_new();
  error_bases[0] = CS_CLASS_OBJECT;
  CS_CLASS_ERROR = cs_mutator_new_class(mut, "Error", dict, error_bases);
  return CS_CLASS_ERROR;
}

void cs_freeclass_error(CsValue klass) {
  cs_hash_free(klass->dict);
}