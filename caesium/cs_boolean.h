#ifndef _CS_BOOLEAN_H_
#define _CS_BOOLEAN_H_

#include "cs_mutator.h"
#include "cs_value.h"

extern CsValue CS_CLASS_TRUE;
extern CsValue CS_CLASS_FALSE;

CsValue cs_initclass_true(CsMutator* mut);
void cs_freeclass_true(CsValue klass);

CsValue cs_initclass_false(CsMutator* mut);
void cs_freeclass_false(CsValue klass);

#endif /* _CS_BOOLEAN_H_ */