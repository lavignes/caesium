#ifndef _CS_ARRAYCLASS_H_
#define _CS_ARRAYCLASS_H_

#include "cs_mutator.h"
#include "cs_value.h"

extern CsValue CS_CLASS_ARRAY;

int cs_arrayclass_as_string(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets);

int cs_arrayclass_add(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets);

CsValue cs_initclass_array(CsMutator* mut);
void cs_freeclass_array(CsValue klass);

#endif /* _CS_ARRAYCLASS_H_ */
