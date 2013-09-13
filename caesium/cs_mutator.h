#ifndef _CS_MUTATOR_H_
#define _CS_MUTATOR_H_

#include <tinycthread.h>

#include "cs_runtime.h"

typedef struct CsMutator {

  bool started;
  CsRuntime* cs;
  thrd_t thread;
  int (*entry_point)(struct CsMutator*, void*);
  void* data;

} CsMutator;

CsMutator* cs_mutator_new(CsRuntime* cs);

void cs_mutator_free(CsMutator* mut);

void cs_mutator_start(
  CsMutator* mut,
  int (*entry_point)(struct CsMutator*, void*),
  void* data);

#endif /* _CS_MUTATOR_H_ */