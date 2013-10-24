#include <malloc.h>
#include <errno.h>

#include "cs_mutator.h"
#include "cs_numeric.h"
#include "cs_error.h"
#include "cs_unicode.h"

static void* mut_main(void* data) {
  CsMutator* mut = data;
  return mut->entry_point(mut, mut->data);
}

void cs_mutator_raise(CsMutator* mut, CsValue error) {
  mut->error = true;
  mut->error_register = error;
}

static CsValue cs_mutator_new_value(CsMutator* mut) {
  CsNurseryPage* page;
  CsValue value;
  int i;
  try_again: value = cs_list_pop_front(mut->freelist);
  if (value == NULL) {
    // Begin garbage collection
    
    // We have to acquire the gc lock, but another thread could have already
    // acquired it and is waiting for us to decrement our sem and let their
    // gc take place. In this case, we must let them have our semaphore.
    // Fortunately, if we were in the middle of something, it shouldn't matter
    // since they are not allowed to free our variables. But they could
    // mark them.
    if (pthread_mutex_trylock(&mut->cs->gc_lock) == EBUSY) {
      sem_post(&mut->cs->gc_sync);
      pthread_cond_wait(&mut->cs->gc_done, &mut->gc_cv_mut);
      sem_wait(&mut->cs->gc_sync);
      goto try_again;
    }

    // Now we have full control of the system.
    // We've (safely) stopped the world.
    cs_mutator_mark(mut);
    cs_mutator_sweep(mut);

    pthread_mutex_unlock(&mut->cs->gc_lock);
    pthread_cond_broadcast(&mut->cs->gc_done);

    value = cs_list_pop_front(mut->freelist);
    // Still didn't work?
    if (value == NULL) {
      // Allocate a new page
      page = cs_nursery_new_page();
      cs_list_push_back(mut->nursery, page);
      // fill the freelist
      for (i = 0; i < CS_NURSERY_PAGE_MAX; i++) {
        cs_list_push_back(mut->freelist, &page->values[i]);
      }
    }
  }
  // mark the value as used. this is a pretty cheap operation :)
  page = (CsNurseryPage*) cs_value_getpage(value);
  page->bitmaps[cs_value_getbits(value, page)] |= CS_NURSERY_USED;
  return value;
}

CsValue cs_mutator_copy_string(
  CsMutator* mut,
  const char* u8str,
  uint32_t hash,
  size_t size,
  size_t length) {

  CsValue value = cs_mutator_new_value(mut);
  value->type = CS_VALUE_STRING;
  value->hash = hash;
  value->string = cs_alloc_object(CsValueString);
  if (cs_unlikely(value->string == NULL))
    cs_exit(CS_REASON_NOMEM);
  value->string->u8str = malloc(size+1);
  if (cs_unlikely(value->string->u8str == NULL))
    cs_exit(CS_REASON_NOMEM);
  memcpy((char*) value->string->u8str, u8str, size);
  value->string->size = size;
  value->string->length = length;
  return value;
}

CsValue cs_mutator_new_string_formatted(
  CsMutator* mut,
  const char* format,
  ...)
{
  va_list args;
  va_start(args, format);
  int len;
  char* buffer = NULL;
  len = vasprintf(&buffer, format, args);
  if (cs_unlikely(len == -1))
    cs_exit(CS_REASON_NOMEM);
  CsValue value = cs_mutator_new_value(mut);
  value->type = CS_VALUE_STRING;
  value->hash = 0;
  value->string = cs_alloc_object(CsValueString);
  if (cs_unlikely(value->string == NULL))
    cs_exit(CS_REASON_NOMEM);
  value->string->u8str = buffer;
  value->string->size = len;
  value->string->length = cs_utf8_strnlen(buffer, -1);
  va_end(args);
  return value;
}

CsValue cs_mutator_new_real(CsMutator* mut, double real) {
  CsValue value = cs_mutator_new_value(mut);
  value->type = CS_VALUE_REAL;
  value->real = real;
  return value;
}

CsValue cs_mutator_new_class(
  CsMutator* mut,
  const char* name,
  CsHash* dict,
  CsArray* bases)
{
  CsValue value = cs_mutator_new_value(mut);
  value->type = CS_VALUE_CLASS;
  value->classname = name;
  value->dict = dict;
  value->bases = bases;
  return value;
}

CsValue cs_mutator_new_builtin0(CsMutator* mut, CsBuiltin0 builtin0) {
  CsValue value = cs_mutator_new_value(mut);
  value->type = CS_VALUE_BUILTIN;
  value->builtin0 = builtin0;
  return value;
}

CsValue cs_mutator_new_builtin1(CsMutator* mut, CsBuiltin1 builtin1) {
  CsValue value = cs_mutator_new_value(mut);
  value->type = CS_VALUE_BUILTIN;
  value->builtin1 = builtin1;
  return value;
}

CsValue cs_mutator_new_builtin2(CsMutator* mut, CsBuiltin2 builtin2) {
  CsValue value = cs_mutator_new_value(mut);
  value->type = CS_VALUE_BUILTIN;
  value->builtin2 = builtin2;
  return value;
}

CsValue cs_mutator_new_builtin3(CsMutator* mut, CsBuiltin3 builtin3) {
  CsValue value = cs_mutator_new_value(mut);
  value->type = CS_VALUE_BUILTIN;
  value->builtin3 = builtin3;
  return value;
}

CsValue cs_mutator_new_instance(CsMutator* mut, CsValue klass) {
  CsValue value = cs_mutator_new_value(mut);
  value->type = CS_VALUE_INSTANCE;
  value->klass = klass;
  value->dict = cs_hash_new();
  return value;
}

CsMutator* cs_mutator_new(CsRuntime* cs) {
  int i;
  CsMutator* mut = cs_alloc_object(CsMutator);
  if (cs_unlikely(mut == NULL))
    cs_exit(CS_REASON_NOMEM);
  mut->started = false;
  mut->cs = cs;
  if (pthread_mutex_init(&mut->gc_cv_mut, NULL))
    cs_exit(CS_REASON_THRDFATAL);
  pthread_mutex_lock(&mut->gc_cv_mut); // We own this

  mut->error = false;
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
  cs_free_object(mut);
}

void cs_mutator_start(
  CsMutator* mut,
  void* (*entry_point)(struct CsMutator*, void*),
  void* data)
{
  mut->entry_point = entry_point;
  mut->data = data;
  if (pthread_create(&mut->thread, NULL, mut_main, mut))
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
      return cs_mutator_copy_string(mut, konst->u8str,
        konst->hash, konst->size, konst->length);
  }

  return CS_NIL;
}

#define load_rk_value(r) (r > 255)? \
  loadk(mut, closure->cur_func->consts->buckets[r - 256]) : closure->stacks[r]

void* cs_mutator_exec(CsMutator* mut, CsByteChunk* chunk) {
  CsByteConst* konst;
  CsPair* pair;
  CsValue bval, cval, temp1, temp2;
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
          pthread_mutex_lock(&mut->cs->globals_lock);
          pair = cs_hash_find(mut->cs->globals, konst->u8str, konst->size);
          pthread_mutex_unlock(&mut->cs->globals_lock);
          if (cs_unlikely(pair == NULL))
          {
            closure->stacks[a] = CS_NIL;
            temp1 = cs_mutator_new_instance(mut, CS_CLASS_NAMEERROR);
            temp2 = cs_mutator_new_string_formatted(mut,
              "name '%s' is not defined", konst->u8str);
            cs_hash_insert(temp1->dict, "what", 4, temp2);
            cs_mutator_raise(mut, temp1);
          }
          else
            closure->stacks[a] = pair->value;
          break;

        case CS_OPCODE_STORG:
          a = cs_bytecode_get_a(code);
          b = cs_bytecode_get_b(code);
          konst = closure->cur_func->consts->buckets[b];
          pthread_mutex_unlock(&mut->cs->globals_lock);
          cs_hash_insert(
            mut->cs->globals,
            konst->u8str,
            konst->size,
            closure->stacks[a]);
          pthread_mutex_unlock(&mut->cs->globals_lock);
          break;

        case CS_OPCODE_PUTS:
          a = cs_bytecode_get_a(code);
          // value_as_string can return NULL if an exception occurs
          temp1 = cs_mutator_value_as_string(mut, closure->stacks[a]);
          if (cs_likely(temp1 != NULL))
            printf("%s\n", cs_value_tostring(temp1));
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
              temp1 = cs_mutator_new_instance(mut, CS_CLASS_TYPEERROR);
              temp2 = cs_mutator_new_string_formatted(mut,
                "invalid operands for add");
              cs_hash_insert(temp1->dict, "what", 4, temp2);
              cs_mutator_raise(mut, temp1);
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

      // sync with gc
      // Essentially, we are giving the gc a chance to take control between
      // instructions.
      sem_post(&mut->cs->gc_sync);
      sem_wait(&mut->cs->gc_sync);
    }
    cs_free_object(closure);
    closure = cs_list_pop_back(mut->stack);

  }

  return NULL;
}

CsValue cs_mutator_value_as_string(CsMutator* mut, CsValue value) {
  CsValue str, temp1, temp2;
  if (cs_value_isint(value)) {
    return cs_mutator_new_string_formatted(mut,
      "%"PRIiPTR, cs_value_toint(value));
  }
  switch (value->type) {
    case CS_VALUE_NIL:
      return cs_mutator_copy_string(mut, "nil", 0, 3, 3);
      break;

    case CS_VALUE_TRUE:
      return cs_mutator_copy_string(mut, "true", 0, 4, 4);
      break;

    case CS_VALUE_FALSE:
      return cs_mutator_copy_string(mut, "false", 0, 5, 5);
      break;

    case CS_VALUE_REAL:
      return cs_mutator_new_string_formatted(mut,
        "%g", cs_value_toreal(value));
      break;

    case CS_VALUE_STRING:
      return value;
      break;

    case CS_VALUE_CLASS:
      return cs_mutator_new_string_formatted(mut,
        "<Class '%s' at %p>", value->classname, value);
      break;

    case CS_VALUE_INSTANCE:
      str = cs_mutator_member_find(mut, value, "__as_string", 11);
      if (str) {
        if (str->type == CS_VALUE_BUILTIN)
          return str->builtin1(mut, value);
        else {
          // __as_string is a key, but cannot be called
          // This is a classical error
          temp1 = cs_mutator_new_instance(mut, CS_CLASS_TYPEERROR);
          temp2 = cs_mutator_copy_string(mut,
            "__as_string is not callable", 0, 27, 27);
          cs_hash_insert(temp1->dict, "what", 4, temp2);
          cs_mutator_raise(mut, temp1);
          return NULL;
        } 
      }

      return cs_mutator_new_string_formatted(mut,
        "<Instance of '%s' at %p>", value->klass->classname, value);
      break;

    case CS_VALUE_BUILTIN:
      return cs_mutator_new_string_formatted(mut,
        "<Builtin at %p>", value);
      break;

    default:
      return cs_mutator_new_string_formatted(mut,
        "<Value at %p>", value);
      cs_exit(CS_REASON_UNIMPLEMENTED);
      break;
  }

  cs_assert(false);
  return NULL;
}

CsValue cs_mutator_member_find(
  CsMutator* mut,
  CsValue instance,
  const char* key,
  size_t key_sz)
{
  size_t i;
  CsArray* bases;
  CsValue ret;
  // Check in local dictionary
  CsPair* pair = cs_hash_find(instance->dict, key, key_sz);
  if (pair)
    return pair->value;
  if (instance->type == CS_VALUE_INSTANCE) {
    // Check class dictionary
    pair = cs_hash_find(instance->klass->dict, key, key_sz);
    if (pair)
      return pair->value;

    bases = instance->klass->bases;
  }
  else bases = instance->bases;
  // Recursively scan base classes for member
  for (i = 0; i < bases->length; i++) {
    ret = cs_mutator_member_find(mut, bases->buckets[i], key, key_sz);
    if (ret)
      return ret;
  }
  return NULL;
}

static void mark_value(CsMutator* mut, CsValue value) {

}

static void mark_hash(CsPair* pair, CsMutator* mut) {

}

void cs_mutator_mark(CsMutator* mut) {
  // We have to mark all globals,
  // then mark all by walking up the stack....
  // then we should be clear to sweep
  cs_hash_traverse(mut->cs->globals, (bool (*)(CsPair*,void*)) mark_hash, mut);
}

void cs_mutator_sweep(CsMutator* mut) {

}

/*
void cs_nursery_mark_page(CsNurseryPage* page) {
  int i;
  for (i = 0; i < CS_NURSERY_PAGE_MAX; i++) {
    if (page->bitmaps[i] & CS_NURSERY_USED) {
      if ((page->bitmaps[i] & CS_NURSERY_MARK) == false) {
        page->bitmaps[i] |= CS_NURSERY_MARK;
        switch (page->values[i]->type) {
          case CS_VALUE_INSTANCE:
            
          break;
        }
      }
    }
  }
}*/

// static bool mark_globals(CsPair* pair, CsMutator* mut) {
//   CsNurseryPage* page;
//   page = (CsNurseryPage*) cs_value_getpage(pair->value);
//   if (!(page->bitmaps[cs_value_getbits(pair->value, page)] & CS_NURSERY_MARK)) {
//     page->bitmaps[cs_value_getbits(pair->value, page)] |= CS_NURSERY_MARK;
//   }
//   mtx_unlock(&page->cs->global_lock);
//   return false;
// }

// static void mutator_mark(CsMutator* mut) {

//   // First mark the globals
//   mtx_lock(&mut->cs->globals_lock);
//   mtx_lock(&mut->cs->global_lock);

//   cs_hash_traverse(mut->cs->globals, (bool (*)(CsPair*,void*)) mark_hash, mut);
//   mtx_unlock(&mut->cs->globals_lock);
// }

// void cs_mutator_collect(CsMutator* mut) {

// }
