#ifndef _CS_RUNTIME_H_
#define _CS_RUNTIME_H_

#include "cs_list.h"

typedef struct CsRuntime {

  CsList* mutators;

} CsRuntime;

CsRuntime* cs_runtime_new();

void cs_runtime_free(CsRuntime* cs);

void cs_runtime_dofile(CsRuntime* cs, const char* filename);

/**
 * Assemble and run Caesium ATOM bytecode
 * @param cs    Caesium runtime handle
 * @param u8str utf-8 encoded assembly file contents
 */
void cs_runtime_doassembly(CsRuntime* cs, const char* u8str);

void cs_runtime_dostring(CsRuntime* cs, const char* u8str);

#endif /* _CS_RUNTIME_H_ */