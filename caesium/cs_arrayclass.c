#include "cs_arrayclass.h"
#include "cs_object.h"
#include "cs_error.h"

CsValue CS_CLASS_ARRAY;

static int __new(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  CsArray* array = cs_array_new();
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
      if (element == SELF)
        value = cs_mutator_new_string_formatted(mut, "%s[...], ",
          cs_value_tostring(value));
      else {
        element = cs_mutator_value_as_string(mut, element);
        if (element == NULL)
          return 0;
        value = cs_mutator_new_string_formatted(mut, "%s%s, ",
          cs_value_tostring(value), cs_value_tostring(element));
      }
    }
  }
  element = cs_value_toarray(SELF)->buckets[i];
  if (!cs_value_isint(element) && element->type == CS_VALUE_STRING)
    value = cs_mutator_new_string_formatted(mut, "%s'%s']",
      cs_value_tostring(value), cs_value_tostring(element));
  else {
    if (element == SELF)
      value = cs_mutator_new_string_formatted(mut, "%s[...]]",
        cs_value_tostring(value));
    else {
      element = cs_mutator_value_as_string(mut, element);
      if (element == NULL)
        return 0;
      value = cs_mutator_new_string_formatted(mut, "%s%s]",
        cs_value_tostring(value), cs_value_tostring(element));
    }
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

int cs_arrayclass_get(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  CsValue value;
  if (cs_value_isint(OTHER)) {
    if (cs_array_find(cs_value_toarray(SELF), cs_value_toint(OTHER),
      (void**) &value)) {
      RET = value;
      return 1;
    }
    else {
      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_INDEXERROR, "index [%"PRIiPTR"] out of range",
        cs_value_toint(OTHER)));
      return 0;
    }
  } else if (OTHER->type == CS_VALUE_STRING) {
    RET = cs_mutator_member_find(mut, CS_CLASS_ARRAY,
      cs_value_tostring(OTHER),
      OTHER->string->size);
    if (RET) {
      return 1;
    } else {
      RET = CS_NIL;
      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_NAMEERROR, "Array has no attribute '%s'",
          cs_value_tostring(OTHER)));
      return 0;
    }
  }

  cs_mutator_raise(mut, cs_mutator_easy_error(mut,
    CS_CLASS_TYPEERROR, "invalid operands for Array.__get"));
  return 0;
}

int cs_arrayclass_set(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  if (cs_value_isint(OTHER)) {
    if (cs_array_set(cs_value_toarray(SELF), cs_value_toint(OTHER), args[2])) {
      return 1;
    }
    else {
      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_INDEXERROR, "index [%"PRIiPTR"] out of range",
        cs_value_toint(OTHER)));
      return 0;
    }
  }

  cs_mutator_raise(mut, cs_mutator_easy_error(mut,
    CS_CLASS_TYPEERROR, "invalid operands for Array.__set"));
  return 0;
}

int cs_arrayclass_push_back(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  cs_array_insert(cs_value_toarray(SELF), -1, OTHER);
  RET = SELF;
  return 1;
}

int cs_arrayclass_insert(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  if (cs_value_isint(OTHER)) {
    if (cs_array_insert(cs_value_toarray(SELF), cs_value_toint(OTHER), args[2])) {
      RET = SELF;
      return 1;
    }
    else {
      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_INDEXERROR, "index [%"PRIiPTR"] out of range",
        cs_value_toint(OTHER)));
      return 0;
    }
  }
  cs_mutator_raise(mut, cs_mutator_easy_error(mut,
    CS_CLASS_TYPEERROR, "invalid operands for Array.insert"));
  return 0;
}

static int cs_arrayclass_remove(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  if (cs_value_isint(OTHER)) {
    if (cs_array_remove(cs_value_toarray(SELF), cs_value_toint(OTHER), (void**) &RET)) {
      RET = SELF;
      return 1;
    }
    else {
      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_INDEXERROR, "index [%"PRIiPTR"] out of range",
        cs_value_toint(OTHER)));
      return 0;
    }
  }
  cs_mutator_raise(mut, cs_mutator_easy_error(mut,
    CS_CLASS_TYPEERROR, "invalid operands for Array.remove"));
  return 0;
}

int cs_arrayclass_length(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  RET = cs_value_fromint((long) cs_value_toarray(SELF)->length);
  return 1;
}

CsValue cs_initclass_array(CsMutator* mut) {
  CsHash* dict = cs_hash_new();
  CsArray* bases = cs_array_new();

  cs_hash_insert(dict, "__as_string", 11,
    cs_mutator_new_builtin(mut, cs_arrayclass_as_string));

  cs_hash_insert(dict, "__new", 5, cs_mutator_new_builtin(mut, __new));

  cs_hash_insert(dict, "__add", 5,
    cs_mutator_new_builtin(mut, cs_arrayclass_add));

  cs_hash_insert(dict, "__get", 5,
    cs_mutator_new_builtin(mut, cs_arrayclass_get));
  cs_hash_insert(dict, "__set", 5,
    cs_mutator_new_builtin(mut, cs_arrayclass_set));

  cs_hash_insert(dict, "push_back", 9,
    cs_mutator_new_builtin(mut, cs_arrayclass_push_back));
  cs_hash_insert(dict, "insert", 6,
    cs_mutator_new_builtin(mut, cs_arrayclass_insert));
  cs_hash_insert(dict, "remove", 6,
    cs_mutator_new_builtin(mut, cs_arrayclass_remove));
  cs_hash_insert(dict, "length", 6,
    cs_mutator_new_builtin(mut, cs_arrayclass_length));

  cs_array_insert(bases, -1, CS_CLASS_OBJECT);
  CS_CLASS_ARRAY = cs_mutator_new_class(mut, "Array", dict, bases);
  return CS_CLASS_ARRAY;
}

void cs_freeclass_array(CsValue klass) {
  cs_hash_free(klass->dict);
  cs_array_free(klass->bases);
}
