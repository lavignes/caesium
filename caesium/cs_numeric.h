#ifndef _CS_NUMERIC_H_
#define _CS_NUMERIC_H_

#include "cs_mutator.h"
#include "cs_value.h"

CsValue cs_int_add(CsMutator* mut, CsValue x, CsValue y);

CsValue cs_real_add(CsMutator* mut, CsValue x, CsValue y);

#endif /* _CS_NUMERIC_H_ */