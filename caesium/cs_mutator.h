#ifndef _CS_MUTATOR_H_
#define _CS_MUTATOR_H_

#include <tinycthread.h>

#include "cs_runtime.h"
#include "cs_assembler.h"
#include "cs_value.h"

typedef struct CsMutator {

  bool started;
  CsRuntime* cs;
  thrd_t thread;
  int (*entry_point)(struct CsMutator*, void*);
  void* data;

  CsList* error_stack; // error handling stack
  CsValue error_register;

  CsList* stack;

} CsMutator;

typedef struct CsStackFrame {

  CsByteFunction* cur_func;
  size_t pc;
  CsValue* params;
  CsValue* upvals;
  CsValue* stacks;

} CsStackFrame;

CsMutator* cs_mutator_new(CsRuntime* cs);

void cs_mutator_free(CsMutator* mut);

void cs_mutator_start(
  CsMutator* mut,
  int (*entry_point)(struct CsMutator*, void*),
  void* data);

void cs_mutator_exec(CsMutator* mut, CsByteChunk* chunk);

#endif /* _CS_MUTATOR_H_ */
