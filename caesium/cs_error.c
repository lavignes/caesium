#include "cs_error.h"
#include "cs_object.h"
#include "cs_unicode.h"

CsValue CS_CLASS_ERROR;
CsValue CS_CLASS_NAMEERROR;
CsValue CS_CLASS_TYPEERROR;

static CsValue error_as_string(CsMutator* mut, CsValue self) {
  CsValue what = cs_mutator_member_find(mut, self, "what", 4);
  CsValue str;
  if (what == NULL) {
    return cs_mutator_copy_string(mut, self->klass->classname, 0,
      strlen(self->klass->classname),
      cs_utf8_strnlen(self->klass->classname, -1));
  }
  str = cs_mutator_value_as_string(mut, what);
  return cs_mutator_new_string_formatted(mut,
    "%s: %s", self->klass->classname, cs_value_tostring(str));
}

CsValue cs_initclass_error(CsMutator* mut) {
  CsHash* dict = cs_hash_new();
  CsArray* bases = cs_array_new();

  CsValue __as_string = cs_mutator_new_builtin1(mut, error_as_string);
  cs_hash_insert(dict, "__as_string", 11, __as_string);

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