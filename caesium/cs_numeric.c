#include <math.h>

#include "cs_object.h"
#include "cs_numeric.h"
#include "cs_error.h"

CsValue CS_CLASS_INT;
CsValue CS_CLASS_REAL;

int cs_int_add(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  if (cs_value_isint(OTHER)) {
    RET = cs_value_fromint(cs_value_toint(SELF) + cs_value_toint(OTHER));
    return 1;
  }
  switch (OTHER->type) {
    case CS_VALUE_REAL:
      RETS = cs_mutator_new_real(mut,
        ((double) cs_value_toint(SELF)) + cs_value_toreal(OTHER));
      return 1;
      
    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_TYPEERROR, "invalid operands for Integer.__add"));
      return NULL;
  }
}

CsValue cs_int_sub(CsMutator* mut, CsValue self, CsValue other) {
  if (cs_value_isint(other))
    return cs_value_fromint(cs_value_toint(self) - cs_value_toint(other));

  switch (other->type) {
    case CS_VALUE_REAL:
      return cs_mutator_new_real(mut,
        ((double) cs_value_toint(self)) - cs_value_toreal(other));

    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_TYPEERROR, "invalid operands for Integer.__sub"));
      return NULL;
  }
}

CsValue cs_int_mul(CsMutator* mut, CsValue self, CsValue other) {
  if (cs_value_isint(other))
    return cs_value_fromint(cs_value_toint(self) * cs_value_toint(other));

  switch (other->type) {
    case CS_VALUE_REAL:
      return cs_mutator_new_real(mut,
        ((double) cs_value_toint(self)) * cs_value_toreal(other));

    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_TYPEERROR, "invalid operands for Integer.__mul"));
      return NULL;
  }
}

CsValue cs_int_div(CsMutator* mut, CsValue self, CsValue other) {
  if (cs_value_isint(other))
    return cs_value_fromint(cs_value_toint(self) / cs_value_toint(other));

  switch (other->type) {
    case CS_VALUE_REAL:
      return cs_mutator_new_real(mut,
        ((double) cs_value_toint(self)) / cs_value_toreal(other));

    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_TYPEERROR, "invalid operands for Integer.__div"));
      return NULL;
  }
}

CsValue cs_int_mod(CsMutator* mut, CsValue self, CsValue other) {
  if (cs_value_isint(other))
    return cs_value_fromint(cs_value_toint(self) % cs_value_toint(other));

  switch (other->type) {
    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_TYPEERROR, "invalid operands for Integer.__mod"));
      return NULL;
  }
}

CsValue cs_int_pow(CsMutator* mut, CsValue self, CsValue other) {
  if (cs_value_isint(other))
    return cs_value_fromint(powl(cs_value_toint(self), cs_value_toint(other)));

  switch (other->type) {
    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_TYPEERROR, "invalid operands for Integer.__pow"));
      return NULL;
  }
}

CsValue cs_int_neg(CsMutator* mut, CsValue self) {
    return cs_value_fromint(-cs_value_toint(self));
}

CsValue cs_int_and(CsMutator* mut, CsValue self, CsValue other) {
  if (cs_value_isint(other))
    return cs_value_fromint(cs_value_toint(self) & cs_value_toint(other));

  switch (other->type) {
    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_TYPEERROR, "invalid operands for Integer.__and"));
      return NULL;
  }
}

CsValue cs_int_or(CsMutator* mut, CsValue self, CsValue other) {
  if (cs_value_isint(other))
    return cs_value_fromint(cs_value_toint(self) | cs_value_toint(other));

  switch (other->type) {
    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_TYPEERROR, "invalid operands for Integer.__or"));
      return NULL;
  }
}

CsValue cs_int_xor(CsMutator* mut, CsValue self, CsValue other) {
  if (cs_value_isint(other))
    return cs_value_fromint(cs_value_toint(self) ^ cs_value_toint(other));

  switch (other->type) {
    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut,
        CS_CLASS_TYPEERROR, "invalid operands for Integer.__xor"));
      return NULL;
  }
}

CsValue cs_int_not(CsMutator* mut, CsValue self) {
    return cs_value_fromint(~cs_value_toint(self));
}

CsValue cs_initclass_int(CsMutator* mut) {
  CsHash* dict = cs_hash_new();
  CsArray* bases = cs_array_new();

  cs_hash_insert(dict, "__add", 5, cs_mutator_new_builtin2(mut, cs_int_add));
  cs_hash_insert(dict, "__sub", 5, cs_mutator_new_builtin2(mut, cs_int_sub));
  cs_hash_insert(dict, "__mul", 5, cs_mutator_new_builtin2(mut, cs_int_mul));
  cs_hash_insert(dict, "__div", 5, cs_mutator_new_builtin2(mut, cs_int_div));
  cs_hash_insert(dict, "__mod", 5, cs_mutator_new_builtin2(mut, cs_int_mod));
  cs_hash_insert(dict, "__pow", 5, cs_mutator_new_builtin2(mut, cs_int_pow));
  cs_hash_insert(dict, "__neg", 5, cs_mutator_new_builtin1(mut, cs_int_neg));
  cs_hash_insert(dict, "__and", 5, cs_mutator_new_builtin2(mut, cs_int_and));
  cs_hash_insert(dict, "__or", 4, cs_mutator_new_builtin2(mut, cs_int_or));
  cs_hash_insert(dict, "__xor", 5, cs_mutator_new_builtin2(mut, cs_int_xor));
  cs_hash_insert(dict, "__not", 5, cs_mutator_new_builtin1(mut, cs_int_not));

  cs_array_insert(bases, -1, CS_CLASS_OBJECT);
  CS_CLASS_INT = cs_mutator_new_class(mut, "Integer", dict, bases);
  return CS_CLASS_INT;
}

void cs_freeclass_int(CsValue klass) {
  cs_hash_free(klass->dict);
  cs_array_free(klass->bases);
}

CsValue cs_real_add(CsMutator* mut, CsValue self, CsValue other) {
  if (cs_value_isint(other))
    return cs_mutator_new_real(mut, 
      cs_value_toreal(self) + (double) cs_value_toint(other));

  switch (other->type) {
    case CS_VALUE_REAL:
      return cs_mutator_new_real(mut,
        cs_value_toreal(self) + cs_value_toreal(other));

    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut, CS_CLASS_TYPEERROR, 
        "invalid operands for Real.__add"));
      return NULL;
  }
}

CsValue cs_real_sub(CsMutator* mut, CsValue self, CsValue other) {
  if (cs_value_isint(other))
    return cs_mutator_new_real(mut, 
      cs_value_toreal(self) - (double) cs_value_toint(other));

  switch (other->type) {
    case CS_VALUE_REAL:
      return cs_mutator_new_real(mut,
        cs_value_toreal(self) - cs_value_toreal(other));

    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut, CS_CLASS_TYPEERROR, 
        "invalid operands for Real.__sub"));
      return NULL;
  }
}

CsValue cs_real_mul(CsMutator* mut, CsValue self, CsValue other) {
  if (cs_value_isint(other))
    return cs_mutator_new_real(mut, 
      cs_value_toreal(self) * (double) cs_value_toint(other));

  switch (other->type) {
    case CS_VALUE_REAL:
      return cs_mutator_new_real(mut,
        cs_value_toreal(self) * cs_value_toreal(other));

    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut, CS_CLASS_TYPEERROR, 
        "invalid operands for Real.__mul"));
      return NULL;
  }
}

CsValue cs_real_div(CsMutator* mut, CsValue self, CsValue other) {
  if (cs_value_isint(other))
    return cs_mutator_new_real(mut, 
      cs_value_toreal(self) / (double) cs_value_toint(other));

  switch (other->type) {
    case CS_VALUE_REAL:
      return cs_mutator_new_real(mut,
        cs_value_toreal(self) / cs_value_toreal(other));

    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut, CS_CLASS_TYPEERROR, 
        "invalid operands for Real.__div"));
      return NULL;
  }
}

CsValue cs_real_pow(CsMutator* mut, CsValue self, CsValue other) {
  if (cs_value_isint(other))
    return cs_mutator_new_real(mut, 
      pow(cs_value_toreal(self), (double) cs_value_toint(other)));

  switch (other->type) {
    case CS_VALUE_REAL:
      return cs_mutator_new_real(mut,
        pow(cs_value_toreal(self), cs_value_toreal(other)));

    default:
      cs_mutator_raise(mut, cs_mutator_easy_error(mut, CS_CLASS_TYPEERROR, 
        "invalid operands for Real.__div"));
      return NULL;
  }
}

CsValue cs_real_neg(CsMutator* mut, CsValue self) {
  return cs_mutator_new_real(mut, -cs_value_toreal(self));
}

CsValue cs_initclass_real(CsMutator* mut) {
  CsHash* dict = cs_hash_new();
  CsArray* bases = cs_array_new();

  cs_hash_insert(dict, "__add", 5, cs_mutator_new_builtin2(mut, cs_real_add));
  cs_hash_insert(dict, "__sub", 5, cs_mutator_new_builtin2(mut, cs_real_sub));
  cs_hash_insert(dict, "__mul", 5, cs_mutator_new_builtin2(mut, cs_real_mul));
  cs_hash_insert(dict, "__div", 5, cs_mutator_new_builtin2(mut, cs_real_div));
  cs_hash_insert(dict, "__pow", 5, cs_mutator_new_builtin2(mut, cs_real_pow));
  cs_hash_insert(dict, "__neg", 5, cs_mutator_new_builtin1(mut, cs_real_neg));

  cs_array_insert(bases, -1, CS_CLASS_OBJECT);
  CS_CLASS_REAL = cs_mutator_new_class(mut, "Real", dict, bases);
  return CS_CLASS_REAL;
}

void cs_freeclass_real(CsValue klass) {
  cs_hash_free(klass->dict);
  cs_array_free(klass->bases);
}
