#ifndef _CS_ERROR_H_
#define _CS_ERROR_H_

#include "cs_mutator.h"
#include "cs_value.h"

extern CsValue CS_CLASS_ERROR;
extern CsValue CS_CLASS_NAMEERROR;
extern CsValue CS_CLASS_TYPEERROR;
extern CsValue CS_CLASS_INDEXERROR;

CsValue cs_initclass_error(CsMutator* mut);
void cs_freeclass_error(CsValue klass);

CsValue cs_initclass_nameerror(CsMutator* mut);
void cs_freeclass_nameerror(CsValue klass);

CsValue cs_initclass_typeerror(CsMutator* mut);
void cs_freeclass_typeerror(CsValue klass);

CsValue cs_initclass_indexerror(CsMutator* mut);
void cs_freeclass_indexerror(CsValue klass);

#endif /* _CS_ERROR_H_ */
