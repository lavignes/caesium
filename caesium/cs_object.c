#include "cs_object.h"
#include "cs_error.h"

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

int __get(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  CsValue value;
  if (!cs_value_isint(OTHER) && OTHER->type == CS_VALUE_STRING) {
    value = cs_mutator_member_find(mut, SELF,
      cs_value_toutf8(OTHER), OTHER->string->size);
    if (value) {
      RET = value;
      return 1;
    }
    else {
      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_INDEXERROR, "index '%s' out of range",
        cs_value_toutf8(OTHER)));
      return 0;
    }
  }

  cs_mutator_raise(mut, cs_mutator_easy_error(mut,
    CS_CLASS_TYPEERROR, "invalid operands for Object.__get"));
  return 0;
}
static int __set(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  if (!cs_value_isint(OTHER) && OTHER->type == CS_VALUE_STRING) {
    cs_hash_insert(SELF->dict,
      cs_value_toutf8(OTHER),
      cs_value_tostring(OTHER)->size, args[2]);
    return 1;
  }
  cs_mutator_raise(mut, cs_mutator_easy_error(mut,
    CS_CLASS_TYPEERROR, "invalid operands for Object.__set"));
  return 0;
}

CsValue cs_initclass_object(CsMutator* mut) {
  CsHash* dict = cs_hash_new();
  CsArray* bases = cs_array_new();

  cs_hash_insert(dict, "__new", 5, cs_mutator_new_builtin(mut, __new));
  cs_hash_insert(dict, "__bool", 6, cs_mutator_new_builtin(mut, __bool));
  cs_hash_insert(dict, "__get", 5, cs_mutator_new_builtin(mut, __get));
  cs_hash_insert(dict, "__set", 5, cs_mutator_new_builtin(mut, __set));

  CS_CLASS_OBJECT = cs_mutator_new_class(mut, "Object", dict, bases);
  return CS_CLASS_OBJECT;
}

void cs_freeclass_object(CsValue klass) {
  cs_hash_free(klass->dict);
  cs_array_free(klass->bases);
}
