#include "cs_error.h"
#include "cs_object.h"
#include "cs_unicode.h"

CsValue CS_CLASS_ERROR;
CsValue CS_CLASS_NAMEERROR;
CsValue CS_CLASS_TYPEERROR;
CsValue CS_CLASS_INDEXERROR;

static int error_as_string(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  CsValue what = cs_mutator_member_find(mut, SELF, "what", 4);
  CsValue str;
  if (what == NULL) {
    if (retc)
      RET = cs_mutator_copy_string(mut, SELF->klass->classname, 0,
      strlen(SELF->klass->classname),
      cs_utf8_strnlen(SELF->klass->classname, -1));
    return 1;
  }
  str = cs_mutator_value_as_string(mut, what);
  // Propagate the found exception
  if (cs_unlikely(str == NULL))
    return 0;
  RET = cs_mutator_new_string_formatted(mut,
    "%s: %s", SELF->klass->classname, cs_value_toutf8(str));
  return 1;
}

CsValue cs_initclass_error(CsMutator* mut) {
  CsHash* dict = cs_hash_new();
  CsArray* bases = cs_array_new();

  cs_hash_insert(dict, "__as_string", 11,
    cs_mutator_new_builtin(mut, error_as_string));

  cs_array_insert(bases, -1, CS_CLASS_OBJECT);
  CS_CLASS_ERROR = cs_mutator_new_class(mut, "Error", dict, bases);
  return CS_CLASS_ERROR;
}

void cs_freeclass_error(CsValue klass) {
  cs_hash_free(klass->dict);
  cs_array_free(klass->bases);
}

CsValue cs_initclass_nameerror(CsMutator* mut) {
  CsHash* dict = cs_hash_new();
  CsArray* bases = cs_array_new();

  cs_array_insert(bases, -1, CS_CLASS_ERROR);
  CS_CLASS_NAMEERROR =
    cs_mutator_new_class(mut, "NameError", dict, bases);
  return CS_CLASS_NAMEERROR;
}

void cs_freeclass_nameerror(CsValue klass) {
  cs_hash_free(klass->dict);
  cs_array_free(klass->bases);
}

CsValue cs_initclass_typeerror(CsMutator* mut) {
  CsHash* dict = cs_hash_new();
  CsArray* bases = cs_array_new();

  cs_array_insert(bases, -1, CS_CLASS_ERROR);
  CS_CLASS_TYPEERROR =
    cs_mutator_new_class(mut, "TypeError", dict, bases);
  return CS_CLASS_TYPEERROR;
}

void cs_freeclass_typeerror(CsValue klass) {
  cs_hash_free(klass->dict);
  cs_array_free(klass->bases);
}

CsValue cs_initclass_indexerror(CsMutator* mut) {
  CsHash* dict = cs_hash_new();
  CsArray* bases = cs_array_new();

  cs_array_insert(bases, -1, CS_CLASS_ERROR);
  CS_CLASS_INDEXERROR =
    cs_mutator_new_class(mut, "IndexError", dict, bases);
  return CS_CLASS_INDEXERROR;
}

void cs_freeclass_indexerror(CsValue klass) {
  cs_hash_free(klass->dict);
  cs_array_free(klass->bases);
}
