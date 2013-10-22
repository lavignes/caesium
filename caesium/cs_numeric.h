#ifndef _CS_NUMERIC_H_
#define _CS_NUMERIC_H_

#include "cs_mutator.h"
#include "cs_value.h"

extern CsValue CS_CLASS_NUMBER;

CsValue cs_initclass_number(CsMutator* mut);
void cs_freeclass_number(CsValue klass);

CsValue cs_int_add(CsMutator* mut, CsValue x, CsValue y);

CsValue cs_real_add(CsMutator* mut, CsValue x, CsValue y);

#endif /* _CS_NUMERIC_H_ */