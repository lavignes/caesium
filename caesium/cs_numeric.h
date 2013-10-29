#ifndef _CS_NUMERIC_H_
#define _CS_NUMERIC_H_

#include "cs_mutator.h"
#include "cs_value.h"

extern CsValue CS_CLASS_INT;
extern CsValue CS_CLASS_REAL;

CsValue cs_int_add(CsMutator* mut, CsValue self, CsValue other);
CsValue cs_int_sub(CsMutator* mut, CsValue self, CsValue other);
CsValue cs_int_mul(CsMutator* mut, CsValue self, CsValue other);
CsValue cs_int_div(CsMutator* mut, CsValue self, CsValue other);
CsValue cs_int_mod(CsMutator* mut, CsValue self, CsValue other);
CsValue cs_int_pow(CsMutator* mut, CsValue self, CsValue other);
CsValue cs_int_neg(CsMutator* mut, CsValue self);
CsValue cs_int_and(CsMutator* mut, CsValue self, CsValue other);
CsValue cs_int_or(CsMutator* mut, CsValue self, CsValue other);
CsValue cs_int_xor(CsMutator* mut, CsValue self, CsValue other);
CsValue cs_int_not(CsMutator* mut, CsValue self);

CsValue cs_initclass_int(CsMutator* mut);
void cs_freeclass_int(CsValue klass);

CsValue cs_real_add(CsMutator* mut, CsValue self, CsValue other);
CsValue cs_real_sub(CsMutator* mut, CsValue self, CsValue other);
CsValue cs_real_mul(CsMutator* mut, CsValue self, CsValue other);
CsValue cs_real_div(CsMutator* mut, CsValue self, CsValue other);
CsValue cs_real_pow(CsMutator* mut, CsValue self, CsValue other);
CsValue cs_real_neg(CsMutator* mut, CsValue self);

CsValue cs_initclass_real(CsMutator* mut);
void cs_freeclass_real(CsValue klass);

#endif /* _CS_NUMERIC_H_ */
