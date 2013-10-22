#ifndef _CS_ERROR_H_
#define _CS_ERROR_H_

#include "cs_mutator.h"
#include "cs_value.h"

extern CsValue CS_CLASS_ERROR;

CsValue cs_initclass_error(CsMutator* mut);
void cs_freeclass_error(CsValue klass);

#endif /* _CS_ERROR_H_ */
