#ifndef _CS_NUMERIC_H_
#define _CS_NUMERIC_H_

#include "cs_mutator.h"
#include "cs_value.h"

extern CsValue CS_CLASS_INT;
extern CsValue CS_CLASS_REAL;

CsValue cs_initclass_int(CsMutator* mut);
void cs_freeclass_int(CsValue klass);

CsValue cs_initclass_real(CsMutator* mut);
void cs_freeclass_real(CsValue klass);

CsValue cs_int_add(CsMutator* mut, CsValue x, CsValue y);

CsValue cs_real_add(CsMutator* mut, CsValue x, CsValue y);

#endif /* _CS_NUMERIC_H_ */
