#include <math.h>

#include "cs_object.h"
#include "cs_numeric.h"
#include "cs_error.h"

CsValue CS_CLASS_INT;
CsValue CS_CLASS_REAL;

static int cs_int_new(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  RET = cs_value_fromint(0);
  return 1;
}

int cs_int_add(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  if (cs_value_isint(OTHER)) {
    RET = cs_value_fromint(cs_value_toint(SELF) + cs_value_toint(OTHER));
    return 1;
  }
  switch (OTHER->type) {
    case CS_VALUE_REAL:
      RET = cs_mutator_new_real(mut,
        ((double) cs_value_toint(SELF)) + cs_value_toreal(OTHER));
      return 1;
      
    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_TYPEERROR, "invalid operands for Integer.__add"));
      return 0;
  }
}

int cs_int_sub(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  if (cs_value_isint(OTHER)) {
    RET = cs_value_fromint(cs_value_toint(SELF) - cs_value_toint(OTHER));
    return 1;
  }
  switch (OTHER->type) {
    case CS_VALUE_REAL:
      RET = cs_mutator_new_real(mut,
        ((double) cs_value_toint(SELF)) - cs_value_toreal(OTHER));
      return 1;
      
    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_TYPEERROR, "invalid operands for Integer.__sub"));
      return 0;
  }
}

int cs_int_mul(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  if (cs_value_isint(OTHER)) {
    RET = cs_value_fromint(cs_value_toint(SELF) * cs_value_toint(OTHER));
    return 1;
  }
  switch (OTHER->type) {
    case CS_VALUE_REAL:
      RET = cs_mutator_new_real(mut,
        ((double) cs_value_toint(SELF)) * cs_value_toreal(OTHER));
      return 1;
      
    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_TYPEERROR, "invalid operands for Integer.__mul"));
      return 0;
  }
}

int cs_int_div(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  if (cs_value_isint(OTHER)) {
    RET = cs_value_fromint(cs_value_toint(SELF) / cs_value_toint(OTHER));
    return 1;
  }
  switch (OTHER->type) {
    case CS_VALUE_REAL:
      RET = cs_mutator_new_real(mut,
        ((double) cs_value_toint(SELF)) / cs_value_toreal(OTHER));
      return 1;
      
    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_TYPEERROR, "invalid operands for Integer.__div"));
      return 0;
  }
}

int cs_int_mod(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  if (cs_value_isint(OTHER)) {
    RET = cs_value_fromint(cs_value_toint(SELF) % cs_value_toint(OTHER));
    return 1;
  }
  switch (OTHER->type) {
    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_TYPEERROR, "invalid operands for Integer.__mod"));
      return 0;
  }
}

int cs_int_pow(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  if (cs_value_isint(OTHER)) {
    RET = cs_value_fromint(powl(cs_value_toint(SELF), cs_value_toint(OTHER)));
    return 1;
  }
  switch (OTHER->type) {
    default:
      RET = cs_mutator_new_real(mut,
        pow(((double) cs_value_toint(SELF)), cs_value_toreal(OTHER)));
      return 1;

      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_TYPEERROR, "invalid operands for Integer.__pow"));
      return 0;
  }
}

int cs_int_neg(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  RET = cs_value_fromint(-cs_value_toint(SELF));
  return 1;
}

int cs_int_and(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  if (cs_value_isint(OTHER)) {
    RET = cs_value_fromint(cs_value_toint(SELF) & cs_value_toint(OTHER));
    return 1;
  }
  switch (OTHER->type) {
    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_TYPEERROR, "invalid operands for Integer.__and"));
      return 0;
  }
}

int cs_int_or(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  if (cs_value_isint(OTHER)) {
    RET = cs_value_fromint(cs_value_toint(SELF) | cs_value_toint(OTHER));
    return 1;
  }
  switch (OTHER->type) {
    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_TYPEERROR, "invalid operands for Integer.__or"));
      return 0;
  }
}

int cs_int_xor(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  if (cs_value_isint(OTHER)) {
    RET = cs_value_fromint(cs_value_toint(SELF) ^ cs_value_toint(OTHER));
    return 1;
  }
  switch (OTHER->type) {
    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_TYPEERROR, "invalid operands for Integer.__xor"));
      return 0;
  }
}

int cs_int_not(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  RET = cs_value_fromint(~cs_value_toint(SELF));
  return 1;
}

int cs_int_lt(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  if (cs_value_isint(OTHER)) {
    if (cs_value_toint(SELF) < cs_value_toint(OTHER))
      RET = CS_TRUE;
    else
      RET = CS_FALSE;
    return 1;
  }
  switch (OTHER->type) {
    case CS_VALUE_REAL:
      if (((double) cs_value_toint(SELF)) < cs_value_toreal(OTHER))
        RET = CS_TRUE;
      else
        RET = CS_FALSE;
      return 1;
      
    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_TYPEERROR, "invalid operands for Integer.__lt"));
      return 0;
  }
}

CsValue cs_initclass_int(CsMutator* mut) {
  CsHash* dict = cs_hash_new();
  CsArray* bases = cs_array_new();

  cs_hash_insert(dict, "__new", 5, cs_mutator_new_builtin(mut, cs_int_new));

  cs_hash_insert(dict, "__add", 5, cs_mutator_new_builtin(mut, cs_int_add));
  cs_hash_insert(dict, "__sub", 5, cs_mutator_new_builtin(mut, cs_int_sub));
  cs_hash_insert(dict, "__mul", 5, cs_mutator_new_builtin(mut, cs_int_mul));
  cs_hash_insert(dict, "__div", 5, cs_mutator_new_builtin(mut, cs_int_div));
  cs_hash_insert(dict, "__mod", 5, cs_mutator_new_builtin(mut, cs_int_mod));
  cs_hash_insert(dict, "__pow", 5, cs_mutator_new_builtin(mut, cs_int_pow));
  cs_hash_insert(dict, "__neg", 5, cs_mutator_new_builtin(mut, cs_int_neg));
  cs_hash_insert(dict, "__and", 5, cs_mutator_new_builtin(mut, cs_int_and));
  cs_hash_insert(dict, "__or", 4, cs_mutator_new_builtin(mut, cs_int_or));
  cs_hash_insert(dict, "__xor", 5, cs_mutator_new_builtin(mut, cs_int_xor));
  cs_hash_insert(dict, "__not", 5, cs_mutator_new_builtin(mut, cs_int_not));
  cs_hash_insert(dict, "__lt", 4, cs_mutator_new_builtin(mut, cs_int_lt));

  cs_array_insert(bases, -1, CS_CLASS_OBJECT);
  CS_CLASS_INT = cs_mutator_new_class(mut, "Integer", dict, bases);
  return CS_CLASS_INT;
}

void cs_freeclass_int(CsValue klass) {
  cs_hash_free(klass->dict);
  cs_array_free(klass->bases);
}

static int cs_real_new(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  RET = cs_mutator_new_real(mut, 0.0);
  return 1;
}

int cs_real_add(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  if (cs_value_isint(OTHER)) {
    RET = cs_mutator_new_real(mut, 
      cs_value_toreal(SELF) + (double) cs_value_toint(OTHER));
    return 1;
  }

  switch (OTHER->type) {
    case CS_VALUE_REAL:
      RET = cs_mutator_new_real(mut,
        cs_value_toreal(SELF) + cs_value_toreal(OTHER));
      return 1;

    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut, CS_CLASS_TYPEERROR, 
        "invalid operands for Real.__add"));
      return 0;
  }
}

int cs_real_sub(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  if (cs_value_isint(OTHER)) {
    RET = cs_mutator_new_real(mut, 
      cs_value_toreal(SELF) - (double) cs_value_toint(OTHER));
    return 1;
  }

  switch (OTHER->type) {
    case CS_VALUE_REAL:
      RET = cs_mutator_new_real(mut,
        cs_value_toreal(SELF) - cs_value_toreal(OTHER));
      return 1;

    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut, CS_CLASS_TYPEERROR, 
        "invalid operands for Real.__sub"));
      return 0;
  }
}

int cs_real_mul(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  if (cs_value_isint(OTHER)) {
    RET = cs_mutator_new_real(mut, 
      cs_value_toreal(SELF) * (double) cs_value_toint(OTHER));
    return 1;
  }

  switch (OTHER->type) {
    case CS_VALUE_REAL:
      RET = cs_mutator_new_real(mut,
        cs_value_toreal(SELF) * cs_value_toreal(OTHER));
      return 1;

    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut, CS_CLASS_TYPEERROR, 
        "invalid operands for Real.__mul"));
      return 0;
  }
}

int cs_real_div(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  if (cs_value_isint(OTHER)) {
    RET = cs_mutator_new_real(mut, 
      cs_value_toreal(SELF) / (double) cs_value_toint(OTHER));
    return 1;
  }

  switch (OTHER->type) {
    case CS_VALUE_REAL:
      RET = cs_mutator_new_real(mut,
        cs_value_toreal(SELF) / cs_value_toreal(OTHER));
      return 1;

    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut, CS_CLASS_TYPEERROR, 
        "invalid operands for Real.__div"));
      return 0;
  }
}

int cs_real_pow(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  if (cs_value_isint(OTHER)) {
    RET = cs_mutator_new_real(mut, 
      pow(cs_value_toreal(SELF), (double) cs_value_toint(OTHER)));
    return 1;
  }

  switch (OTHER->type) {
    case CS_VALUE_REAL:
      RET = cs_mutator_new_real(mut,
        pow(cs_value_toreal(SELF), cs_value_toreal(OTHER)));
      return 1;

    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut, CS_CLASS_TYPEERROR, 
        "invalid operands for Real.__pow"));
      return 0;
  }
}

int cs_real_neg(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  RET = cs_mutator_new_real(mut, -cs_value_toreal(SELF));
  return 1;
}

int cs_real_lt(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  if (cs_value_isint(OTHER)) {
    if (cs_value_toreal(SELF) < (double) cs_value_toint(OTHER))
      RET = CS_TRUE;
    else
      RET = CS_FALSE;
    return 1;
  }

  switch (OTHER->type) {
    case CS_VALUE_REAL:
      if (cs_value_toreal(SELF) < cs_value_toreal(OTHER))
        RET = CS_TRUE;
      else
        RET = CS_FALSE;
      return 1;

    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut, CS_CLASS_TYPEERROR, 
        "invalid operands for Real.__lt"));
      return 0;
  }
}

CsValue cs_initclass_real(CsMutator* mut) {
  CsHash* dict = cs_hash_new();
  CsArray* bases = cs_array_new();

  cs_hash_insert(dict, "__new", 5, cs_mutator_new_builtin(mut, cs_real_new));

  cs_hash_insert(dict, "__add", 5, cs_mutator_new_builtin(mut, cs_real_add));
  cs_hash_insert(dict, "__sub", 5, cs_mutator_new_builtin(mut, cs_real_sub));
  cs_hash_insert(dict, "__mul", 5, cs_mutator_new_builtin(mut, cs_real_mul));
  cs_hash_insert(dict, "__div", 5, cs_mutator_new_builtin(mut, cs_real_div));
  cs_hash_insert(dict, "__pow", 5, cs_mutator_new_builtin(mut, cs_real_pow));
  cs_hash_insert(dict, "__neg", 5, cs_mutator_new_builtin(mut, cs_real_neg));

  cs_hash_insert(dict, "__lt", 4, cs_mutator_new_builtin(mut, cs_real_lt));

  cs_array_insert(bases, -1, CS_CLASS_OBJECT);
  CS_CLASS_REAL = cs_mutator_new_class(mut, "Real", dict, bases);
  return CS_CLASS_REAL;
}

void cs_freeclass_real(CsValue klass) {
  cs_hash_free(klass->dict);
  cs_array_free(klass->bases);
}
