#include "cs_arrayclass.h"
#include "cs_object.h"
#include "cs_error.h"

CsValue CS_CLASS_ARRAY;

static int __new(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  CsArray* array = cs_array_new();
  cs_array_insert(array, -1, cs_value_fromint(42));
  RET = cs_mutator_new_array(mut, array);
  return 1;
}

// Includes a special case for strings.
// if array contains a string, put quotes around
int cs_arrayclass_as_string(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  size_t i;
  CsValue value, element;
  if (cs_value_toarray(SELF)->length == 0) {
    RET = cs_mutator_new_string_formatted(mut, "[]");
    return 1;
  }
  value = cs_mutator_copy_string(mut, "[", 0, 1, 1);
  for (i = 0; i < cs_value_toarray(SELF)->length-1; ++i) {
    element = cs_value_toarray(SELF)->buckets[i];
    if (!cs_value_isint(element) && element->type == CS_VALUE_STRING)
      value = cs_mutator_new_string_formatted(mut, "%s'%s', ",
        cs_value_tostring(value), cs_value_tostring(element));
    else {
      element = cs_mutator_value_as_string(mut, element);
      if (element == NULL)
        return 0;
      value = cs_mutator_new_string_formatted(mut, "%s%s, ",
        cs_value_tostring(value), cs_value_tostring(element));
    }
  }
  element = cs_value_toarray(SELF)->buckets[i];
  if (!cs_value_isint(element) && element->type == CS_VALUE_STRING)
    value = cs_mutator_new_string_formatted(mut, "%s'%s']",
      cs_value_tostring(value), cs_value_tostring(element));
  else {
    element = cs_mutator_value_as_string(mut, element);
    if (element == NULL)
      return 0;
    value = cs_mutator_new_string_formatted(mut, "%s%s]",
      cs_value_tostring(value), cs_value_tostring(element));
  }
  RET = value;
  return 1;
}

int cs_arrayclass_add(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  size_t i;
  CsValue temp;
  if (!cs_value_isint(OTHER) && OTHER->type == CS_VALUE_ARRAY) {
    temp = cs_mutator_new_array(mut, cs_array_copy(cs_value_toarray(SELF)));
    for (i = 0; i < cs_value_toarray(OTHER)->length; ++i) {
      cs_array_insert(cs_value_toarray(temp), -1,
        cs_value_toarray(OTHER)->buckets[i]);
    }
    RET = temp;
    return 1;
  }
  cs_mutator_raise(mut, cs_mutator_easy_error(mut,
    CS_CLASS_TYPEERROR, "invalid operands for Array.__add"));
  return 0;
}

CsValue cs_initclass_array(CsMutator* mut) {
  CsHash* dict = cs_hash_new();
  CsArray* bases = cs_array_new();

  cs_hash_insert(dict, "__as_string", 11,
    cs_mutator_new_builtin(mut, cs_arrayclass_as_string));

  cs_hash_insert(dict, "__new", 5, cs_mutator_new_builtin(mut, __new));

  cs_hash_insert(dict, "__add", 5,
    cs_mutator_new_builtin(mut, cs_arrayclass_add));

  cs_array_insert(bases, -1, CS_CLASS_OBJECT);
  CS_CLASS_ARRAY = cs_mutator_new_class(mut, "Array", dict, bases);
  return CS_CLASS_ARRAY;
}

void cs_freeclass_array(CsValue klass) {
  cs_hash_free(klass->dict);
  cs_array_free(klass->bases);
}
