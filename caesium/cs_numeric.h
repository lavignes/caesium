#ifndef _CS_NUMERIC_H_
#define _CS_NUMERIC_H_

#include "cs_mutator.h"
#include "cs_value.h"

extern CsValue CS_CLASS_INT;
extern CsValue CS_CLASS_REAL;

int cs_int_add(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets);
int cs_int_sub(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets);
int cs_int_mul(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets);
int cs_int_div(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets);
int cs_int_mod(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets);
int cs_int_pow(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets);
int cs_int_neg(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets);
int cs_int_and(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets);
int cs_int_or(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets);
int cs_int_add(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets);
int cs_int_xor(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets);
int cs_int_not(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets);
int cs_int_lt(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets);

CsValue cs_initclass_int(CsMutator* mut);
void cs_freeclass_int(CsValue klass);

int cs_real_add(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets);
int cs_real_sub(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets);
int cs_real_mul(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets);
int cs_real_div(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets);
int cs_real_mod(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets);
int cs_real_pow(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets);
int cs_real_neg(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets);

CsValue cs_initclass_real(CsMutator* mut);
void cs_freeclass_real(CsValue klass);

#endif /* _CS_NUMERIC_H_ */
