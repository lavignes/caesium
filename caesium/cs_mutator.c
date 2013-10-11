#include "cs_mutator.h"

static int mut_main(void* data) {
  CsMutator* mut = data;
  return mut->entry_point(mut, mut->data);
}

CsMutator* cs_mutator_new(CsRuntime* cs) {
  CsMutator* mut = cs_alloc_object(CsMutator);
  if (cs_unlikely(mut == NULL))
    cs_exit(CS_REASON_NOMEM);
  mut->started = false;
  mut->cs = cs;
  mut->error_stack = cs_list_new();
  mut->stack = cs_list_new();
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

void cs_mutator_exec(CsMutator* mut, CsByteChunk* chunk) {
  CsByteFunction* entry = chunk->entry;
  CsStackFrame* frame = malloc(sizeof(CsStackFrame) + sizeof(CsValue)
    * (entry->nparams + entry->nupvals + entry->nstacks));
  if (cs_unlikely(frame == NULL))
    cs_exit(CS_REASON_NOMEM);
  frame->params = (CsValue*) frame + sizeof(CsStackFrame);
  frame->upvals = (CsValue*) frame->params + sizeof(CsValue) * entry->nparams;
  frame->stacks = (CsValue*) frame->upvals + sizeof(CsValue) * entry->nupvals;

  frame->cur_func = entry;
  frame->pc = 0;
  CsByteConst* konst;
  int a, b;
  for (frame->pc = 0; frame->pc < frame->cur_func->codes->length; frame->pc++) {
    CsOpcode code = (CsOpcode) entry->codes->buckets[frame->pc];
    switch (code) {
      case CS_OPCODE_LOADK:
        a = cs_bytecode_get_a(code);
        b = cs_bytecode_get_b(code);
        frame->stacks[a] = malloc(sizeof(CsValueStruct));
        konst = entry->consts->buckets[b];
        frame->stacks[a]->type = CS_VALUE_STRING;
        frame->stacks[a]->size = konst->size;
        frame->stacks[a]->string = konst->string;
        break;
        
      case CS_OPCODE_PUTS:
        a = cs_bytecode_get_a(code);
        printf("%s\n", frame->stacks[a]->string);
        break;

      default:
        //cs_exit(CS_REASON_UNIMPLEMENTED);
        break;
    }
  }
}
