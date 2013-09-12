#ifndef _CS_RUNTIME_H_
#define _CS_RUNTIME_H_

#include "cs_list.h"

typedef struct CsRuntime {

  CsList* mutators;

} CsRuntime;

CsRuntime* cs_runtime_new();

void cs_runtime_free(CsRuntime* cs);

void cs_runtime_dofile(CsRuntime* cs, const char* filename);

void cs_runtime_dostring(CsRuntime* cs, const char* u8str);

#endif /* _CS_RUNTIME_H_ */