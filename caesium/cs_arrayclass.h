#ifndef _CS_ARRAYCLASS_H_
#define _CS_ARRAYCLASS_H_

#include "cs_mutator.h"
#include "cs_value.h"

extern CsValue CS_CLASS_ARRAY;

CsValue cs_initclass_array(CsMutator* mut);
void cs_freeclass_array(CsValue klass);

#endif /* _CS_ARRAYCLASS_H_ */
