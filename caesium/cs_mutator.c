#include "cs_mutator.h"

int mut_main(void* data) {
  CsMutator* mut = data;
  return mut->entry_point(mut, mut->data);
}

CsMutator* cs_mutator_new(CsRuntime* cs) {
  CsMutator* mut = cs_alloc_object(CsMutator);
  if (mut == NULL)
    cs_exit(CS_REASON_NOMEM);
  mut->started = false;
  mut->cs = cs;
  return mut;
}

void cs_mutator_free(CsMutator* mut) {
  cs_free_object(mut);
}

void cs_mutator_start(
  CsMutator* mut,
  int (*entry_point)(struct CsMutator*, void*),
  void* data)
{
  mut->entry_point = entry_point;
  mut->data = data;
  if (thrd_create(&mut->thread, mut_main, mut) != thrd_success)
    cs_exit(CS_REASON_THRDFATAL);
}