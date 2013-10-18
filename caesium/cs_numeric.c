#include "cs_numeric.h"

CsValue cs_int_add(CsMutator* mut, CsValue x, CsValue y) {
  if (cs_value_isint(y))
    return cs_value_fromint(cs_value_toint(x) + cs_value_toint(y));

  switch (y->type) {
    case CS_VALUE_REAL:
      return cs_mutator_new_real(mut,
        ((double) cs_value_toint(x)) + cs_value_toreal(y));

    default:
      cs_error("int_add is not fully implemented dummy!\n");
      return CS_NIL;
  }
}

CsValue cs_real_add(CsMutator* mut, CsValue x, CsValue y) {
  if (cs_value_isint(y))
    return cs_mutator_new_real(mut, 
      cs_value_toreal(x) + (double) cs_value_toint(y));

  switch (y->type) {
    case CS_VALUE_REAL:
      return cs_mutator_new_real(mut, cs_value_toreal(x) + cs_value_toreal(y));

    default:
      cs_error("real_add is not fully implemented dummy!\n");
      return CS_NIL;
  }
}
