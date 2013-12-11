#ifndef _CS_STRING_H_
#define _CS_STRING_H_

#include "cs_mutator.h"
#include "cs_value.h"

extern CsValue CS_CLASS_STRING;

int cs_string_add(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets);

int cs_string_mul(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets);

int cs_string_get(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets);

CsValue cs_initclass_string(CsMutator* mut);
void cs_freeclass_string(CsValue klass);

#endif /* _CS_STRING_H_ */
