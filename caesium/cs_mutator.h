#ifndef _CS_MUTATOR_H_
#define _CS_MUTATOR_H_

#include <tinycthread.h>

#include "cs_runtime.h"
#include "cs_assembler.h"
#include "cs_value.h"
#include "cs_nursery.h"

typedef struct CsMutator {

  bool started;
  CsRuntime* cs;
  thrd_t thread;
  int (*entry_point)(struct CsMutator*, void*);
  void* data;

  bool error;
  CsValue error_register;

  CsList* stack;
  CsList* nursery;
  CsList* freelist;

} CsMutator;

typedef struct CsClosure {

  CsByteFunction* cur_func;
  size_t pc;
  uintptr_t* codes;
  size_t ncodes;
  CsValue* params;
  CsValue* upvals;
  CsValue* stacks;

} CsClosure;

CsValue cs_mutator_new_string(
  CsMutator* mut,
  const char* u8str,
  uint32_t hash,
  size_t size,
  size_t length);
  
CsValue cs_mutator_new_real(CsMutator* mut, double real);

CsValue cs_mutator_new_class(
  CsMutator* mut,
  const char* name,
  CsHash* dict,
  CsValue* bases);

CsValue cs_mutator_new_builtin0(CsMutator* mut, CsBuiltin0 builtin0);
CsValue cs_mutator_new_builtin1(CsMutator* mut, CsBuiltin1 builtin1);
CsValue cs_mutator_new_builtin2(CsMutator* mut, CsBuiltin2 builtin2);
CsValue cs_mutator_new_builtin3(CsMutator* mut, CsBuiltin3 builtin3);

CsValue cs_mutator_new_instance(CsMutator* mut, CsValue klass);

CsMutator* cs_mutator_new(CsRuntime* cs);

void cs_mutator_free(CsMutator* mut);

void cs_mutator_start(
  CsMutator* mut,
  int (*entry_point)(struct CsMutator*, void*),
  void* data);

/**
 * Execute a bytecode chunk (default mutator entry point)
 * @param mut   a mutator
 * @param chunk a bytechunk
 * @return thread return value
 */
int cs_mutator_exec(CsMutator* mut, CsByteChunk* chunk);

void cs_mutator_raise(CsMutator* mut, CsValue error);

#endif /* _CS_MUTATOR_H_ */
