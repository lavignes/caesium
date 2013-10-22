#include <malloc.h>

#include "cs_mutator.h"
#include "cs_numeric.h"

static int mut_main(void* data) {
  CsMutator* mut = data;
  return mut->entry_point(mut, mut->data);
}

void cs_mutator_raise(CsMutator* mut, CsValue error) {
  mut->error = true;
  mut->error_register = error;
}

static CsValue cs_mutator_new_value(CsMutator* mut) {
  CsNurseryPage* page;
  CsValue value = cs_list_pop_front(mut->freelist);
  if (value == NULL) {
    cs_error("Haven't finished expanding the pages yet...\n");
    cs_exit(CS_REASON_UNIMPLEMENTED);
  }
  // mark the value as used. this is a pretty cheap operation :)
  page = (CsNurseryPage*) cs_value_getpage(value);
  page->bitmaps[cs_value_getbits(value, page)] &= CS_NURSERY_USED;
  return value;
}

CsValue cs_mutator_new_string(
  CsMutator* mut,
  const char* u8str,
  uint32_t hash,
  size_t size,
  size_t length)
{
  CsValue value = cs_mutator_new_value(mut);
  value->type = CS_VALUE_STRING;
  value->hash = hash;
  value->string = cs_alloc_object(CsValueString);
  if (cs_unlikely(value->string == NULL))
    cs_exit(CS_REASON_NOMEM);
  value->string->u8str = u8str;
  value->string->size = size;
  value->string->length = length;
  return value;
}

CsValue cs_mutator_new_real(CsMutator* mut, double real) {
  CsValue value = cs_mutator_new_value(mut);
  value->type = CS_VALUE_REAL;
  value->real = real;
  return value;
}

CsMutator* cs_mutator_new(CsRuntime* cs) {
  int i;
  CsMutator* mut = cs_alloc_object(CsMutator);
  if (cs_unlikely(mut == NULL))
    cs_exit(CS_REASON_NOMEM);
  mut->started = false;
  mut->cs = cs;

  mut->error = false;
  mut->error_stack = cs_list_new();
  mut->error_register = CS_NIL;
  mut->stack = cs_list_new();

  mut->nursery = cs_list_new();
  mut->freelist = cs_list_new();

  // Allocate initial nursery
  CsNurseryPage* page = cs_nursery_new_page();
  // fill the freelist
  for (i = 0; i < CS_NURSERY_PAGE_MAX; i++) {
    cs_list_push_back(mut->freelist, &page->values[i]);
  }

  cs_list_push_back(mut->nursery, page);

  return mut;
}

static bool free_page(void* page, void* data) {
  cs_nursery_free_page(page);
  return false;
}

void cs_mutator_free(CsMutator* mut) {
  cs_list_free(mut->stack);
  cs_list_traverse(mut->nursery, free_page, NULL);
  cs_list_free(mut->nursery);
  cs_list_free(mut->freelist);
  cs_list_free(mut->error_stack);
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

static CsClosure* create_stack_frame(CsByteFunction* func) {
  CsClosure* closure = malloc(sizeof(CsClosure) + sizeof(CsValue)
    * (func->nparams + func->nupvals + func->nstacks));
  if (cs_unlikely(closure == NULL))
    cs_exit(CS_REASON_NOMEM);

  // Cast closure to void* to prevent heap corruption
  closure->params = ((void*) closure) + sizeof(CsClosure);
  closure->upvals = &closure->params[func->nparams];
  closure->stacks = &closure->upvals[func->nupvals];

  closure->cur_func = func;
  closure->pc = 0;

  closure->ncodes = func->codes->length;
  closure->codes = (uintptr_t*) func->codes->buckets;

  return closure;
}

static CsValue loadk(CsMutator* mut, CsByteConst* konst) {
  switch (konst->type) {
    case CS_CONST_TYPE_NIL:
      return CS_NIL;

    case CS_CONST_TYPE_TRUE:
      return CS_TRUE;

    case CS_CONST_TYPE_FALSE:
      return CS_FALSE;

    case CS_CONST_TYPE_INT:
      return cs_value_fromint(konst->integer);
      break;

    case CS_CONST_TYPE_REAL:
      return cs_mutator_new_real(mut, konst->real);

    case CS_CONST_TYPE_STRING:
      return cs_mutator_new_string(mut, konst->u8str,
        konst->hash, konst->size, konst->length);
  }

  return CS_NIL;
}

#define load_rk_value(r) (r > 255)? \
  loadk(mut, closure->cur_func->consts->buckets[r - 256]) : closure->stacks[r]

int cs_mutator_exec(CsMutator* mut, CsByteChunk* chunk) {
  CsByteConst* konst;
  CsPair* pair;
  CsValue bval, cval;
  int a, b, c;

  CsClosure* closure = create_stack_frame(chunk->entry);

  while (cs_likely(closure)) {
    for (closure->pc = 0; closure->pc < closure->ncodes; closure->pc++) {
      CsByteCode code = closure->codes[closure->pc];
      switch (cs_bytecode_get_opcode(code)) {
        case CS_OPCODE_MOVE:
          a = cs_bytecode_get_a(code);
          b = cs_bytecode_get_b(code);
          if (b > 255) {
            b = b - 256;
            goto move_as_loadk;
          }
          closure->stacks[a] = closure->stacks[b];
          break;

        case CS_OPCODE_LOADK:
          a = cs_bytecode_get_a(code);
          b = cs_bytecode_get_b(code);
          move_as_loadk: closure->stacks[a] =
            loadk(mut, closure->cur_func->consts->buckets[b]);
          break;
          
        case CS_OPCODE_LOADG:
          a = cs_bytecode_get_a(code);
          b = cs_bytecode_get_b(code);
          konst = closure->cur_func->consts->buckets[b];
          mtx_lock(&mut->cs->globals_lock);
          pair = cs_hash_find(mut->cs->globals, konst->u8str, konst->size);
          mtx_unlock(&mut->cs->globals_lock);
          if (cs_likely(pair != NULL))
            closure->stacks[a] = pair->value;
          else
            // This should raise.... but just return NIL for now
            closure->stacks[a] = CS_NIL;
          break;

        case CS_OPCODE_STORG:
          a = cs_bytecode_get_a(code);
          b = cs_bytecode_get_b(code);
          konst = closure->cur_func->consts->buckets[b];
          mtx_lock(&mut->cs->globals_lock);
          cs_hash_insert(
            mut->cs->globals,
            konst->u8str,
            konst->size,
            closure->stacks[a]);
          mtx_unlock(&mut->cs->globals_lock);
          break;

        case CS_OPCODE_PUTS:
          a = cs_bytecode_get_a(code);
          if (cs_value_isint(closure->stacks[a])) {
            printf("%"PRIiPTR"\n", cs_value_toint(closure->stacks[a]));
            break;
          }
          switch (closure->stacks[a]->type) {
            case CS_VALUE_NIL:
              printf("nil\n");
              break;

            case CS_VALUE_TRUE:
              printf("true\n");
              break;

            case CS_VALUE_FALSE:
              printf("false\n");
              break;

            case CS_VALUE_REAL:
              printf("%g\n", cs_value_toreal(closure->stacks[a]));
              break;

            case CS_VALUE_STRING:
              printf("%s\n", cs_value_tostring(closure->stacks[a]));
              break;
          }
          break;

          case CS_OPCODE_ADD:
            a = cs_bytecode_get_a(code);
            b = cs_bytecode_get_b(code);
            c = cs_bytecode_get_c(code);
            // Read the args as konst or value
            bval = load_rk_value(b);
            cval = load_rk_value(c);
            if (cs_value_isint(bval)) {
              closure->stacks[a] = cs_int_add(mut, bval, cval);
              break;
            }
            switch (bval->type) {
              case CS_VALUE_REAL:
                closure->stacks[a] = cs_real_add(mut, bval, cval);
                break;

              default:
                closure->stacks[a] = CS_NIL;
                cs_mutator_raise(mut, cs_mutator_new_string(mut, "Trying to add an unaddable value!", 0, 33, 33));
                break;
            }
            break;

          case CS_OPCODE_RAISE:
            a = cs_bytecode_get_a(code);
            cs_mutator_raise(mut, closure->stacks[a]);
            break;

          case CS_OPCODE_CATCH:
            a = cs_bytecode_get_a(code);
            closure->stacks[a] = mut->error_register;
            mut->error_register = CS_NIL;
            break;

          case CS_OPCODE_RET:
            break;

        default:
          cs_exit(CS_REASON_UNIMPLEMENTED);
          break;
      }
      // Error was raised
      if (cs_unlikely(mut->error)) {
        // redirect to exception handler
        if (closure->cur_func->resq) {
          mut->error = false;
          closure->pc = -1; // This is needed
          closure->ncodes = closure->cur_func->resq->length;
          closure->codes = (uintptr_t*) closure->cur_func->resq->buckets;
        }
      }
    }
    cs_free_object(closure);
    closure = cs_list_pop_back(mut->stack);
  }
  return 0;
}
