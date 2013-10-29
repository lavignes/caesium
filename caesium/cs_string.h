#ifndef _CS_STRING_H_
#define _CS_STRING_H_

#include "cs_mutator.h"
#include "cs_value.h"

extern CsValue CS_CLASS_STRING;

CsValue cs_initclass_string(CsMutator* mut);
void cs_freeclass_string(CsValue klass);

#endif /* _CS_STRING_H_ */
