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

static CsStackFrame* create_stack_frame(CsByteFunction* func) {
  CsStackFrame* frame = malloc(sizeof(CsStackFrame) + sizeof(CsValue)
    * (func->nparams + func->nupvals + func->nstacks));
  if (cs_unlikely(frame == NULL))
    cs_exit(CS_REASON_NOMEM);

  frame->params = (CsValue*) frame + sizeof(CsStackFrame);
  frame->upvals = (CsValue*) frame->params + sizeof(CsValue) * func->nparams;
  frame->stacks = (CsValue*) frame->upvals + sizeof(CsValue) * func->nupvals;

  frame->cur_func = func;
  frame->pc = 0;

  frame->ncodes = func->codes->length;
  frame->codes = (uintptr_t*) func->codes->buckets;

  return frame;
}

void cs_mutator_exec(CsMutator* mut, CsByteChunk* chunk) {
  CsStackFrame* frame = create_stack_frame(chunk->entry);
  CsByteConst* konst;
  CsPair* pair;
  int a, b;

  for (frame->pc = 0; frame->pc < frame->ncodes; frame->pc++) {
    CsByteCode code = frame->codes[frame->pc];
    switch (code) {
      case CS_OPCODE_MOVE:
        a = cs_bytecode_get_a(code);
        b = cs_bytecode_get_b(code);
        frame->stacks[a] = frame->stacks[b];
        break;

      case CS_OPCODE_LOADK:
        a = cs_bytecode_get_a(code);
        b = cs_bytecode_get_b(code);
        frame->stacks[a] = malloc(sizeof(CsValueStruct));
        konst = frame->cur_func->consts->buckets[b];
        frame->stacks[a]->type = CS_VALUE_STRING;
        frame->stacks[a]->size = konst->size;
        frame->stacks[a]->string = konst->string;
        break;
        
      case CS_OPCODE_LOADG:
        a = cs_bytecode_get_a(code);
        b = cs_bytecode_get_b(code);
        konst = frame->cur_func->consts->buckets[b];
        mtx_lock(&mut->cs->globals_lock);
        pair = cs_hash_find(mut->cs->globals, konst->string, konst->size);
        mtx_unlock(&mut->cs->globals_lock);
        if (cs_likely(pair != NULL))
          frame->stacks[a] = pair->value;
        else
          // This should raise.... but just return NIL for now
          frame->stacks[a] = CS_NIL;
        break;

      case CS_OPCODE_STORG:
        a = cs_bytecode_get_a(code);
        b = cs_bytecode_get_b(code);
        konst = frame->cur_func->consts->buckets[b];
        mtx_lock(&mut->cs->globals_lock);
        cs_hash_insert(
          mut->cs->globals,
          konst->string,
          konst->size,
          frame->stacks[a]);
        mtx_unlock(&mut->cs->globals_lock);
        break;

      case CS_OPCODE_PUTS:
        a = cs_bytecode_get_a(code);
        printf("%s\n", frame->stacks[a]->string);
        break;

      default:
        cs_exit(CS_REASON_UNIMPLEMENTED);
        break;
    }
  }
}
