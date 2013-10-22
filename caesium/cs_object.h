#ifndef _CS_OBJECT_H_
#define _CS_OBJECT_H_

#include "cs_value.h"
#include "cs_mutator.h"

extern CsValue CS_CLASS_OBJECT;

CsValue cs_initclass_object(CsMutator* mut);
void cs_freeclass_object(CsValue klass);

#endif /* _CS_OBJECT_H_ */