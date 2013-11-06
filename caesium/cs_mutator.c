#include <malloc.h>
#include <errno.h>

#include "cs_mutator.h"
#include "cs_error.h"
#include "cs_numeric.h"
#include "cs_string.h"
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
    // Attempt garbage collection
    // collect will return 1 if your thread couldn't initiate collection.
    // This usually means another thread did it for you.
    if (cs_mutator_collect(mut))
      goto try_again;

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
      value = cs_list_pop_front(mut->freelist);
    }
  }
  page = (CsNurseryPage*) cs_value_getpage(value);
  page->bitmaps[cs_value_getbits(value, page)] = mut->epoch | CS_NURSERY_USED;
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

static CsValue cs_mutator_new_string_va_list(
  CsMutator* mut,
  const char* format,
  va_list args)
{
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

CsValue cs_mutator_new_builtin(CsMutator* mut, CsBuiltin builtin) {
  CsValue value = cs_mutator_new_value(mut);
  value->type = CS_VALUE_BUILTIN;
  value->builtin = builtin;
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

  mut->epoch = 0;

  mut->error = false;
  mut->error_register = CS_NIL;

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

  closure->parent = NULL;

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
  CsValue val[3];
  CsValue temp1;
  int a, b, c, simm;

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
          if (cs_unlikely(pair == NULL)) {
            closure->stacks[a] = CS_NIL;
            cs_mutator_raise(mut, cs_mutator_easy_error(mut,
              CS_CLASS_NAMEERROR, "name '%s' is not defined", konst->u8str));
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
          val[1] = load_rk_value(b);
          val[2] = load_rk_value(c);
          if (cs_value_isint(val[1])) {
            cs_int_add(mut, 2, &val[1], 1, &closure->stacks[a]);
          }
          switch (val[1]->type) {
            case CS_VALUE_REAL:
              cs_real_add(mut, 2, &val[1], 1, &closure->stacks[a]);
              break;

            case CS_VALUE_STRING:
              cs_string_add(mut, 2, &val[1], 1, &closure->stacks[a]);
              break;

            case CS_VALUE_INSTANCE:
              temp1 = cs_mutator_member_find(mut, val[1], "__add", 5);
              if (temp1) {
                if (temp1->type == CS_VALUE_BUILTIN)
                  temp1->builtin(mut, 2, &val[1], 1, &closure->stacks[a]);
                else {
                  closure->stacks[a] = CS_NIL;
                  cs_mutator_raise(mut, cs_mutator_easy_error(mut,
                    CS_CLASS_TYPEERROR, "%s.__add is not callable",
                    val[1]->klass->classname));
                } 
              }
              else goto add_error;
              break;

            default:
              add_error: closure->stacks[a] = CS_NIL;
              cs_mutator_raise(mut, cs_mutator_easy_error(mut,
                CS_CLASS_TYPEERROR, "invalid operands for Object.__add"));
              break;
          }
          break;

        case CS_OPCODE_ADD:
          a = cs_bytecode_get_a(code);
          b = cs_bytecode_get_b(code);
          c = cs_bytecode_get_c(code);
          // Read the args as konst or value
          val[1] = load_rk_value(b);
          val[2] = load_rk_value(c);
          if (cs_value_isint(val[1])) {
            cs_int_sub(mut, 2, &val[1], 1, &closure->stacks[a]);
          }
          switch (val[1]->type) {
            case CS_VALUE_REAL:
              cs_real_sub(mut, 2, &val[1], 1, &closure->stacks[a]);
              break;

            case CS_VALUE_INSTANCE:
              temp1 = cs_mutator_member_find(mut, val[1], "__sub", 5);
              if (temp1) {
                if (temp1->type == CS_VALUE_BUILTIN)
                  temp1->builtin(mut, 2, &val[1], 1, &closure->stacks[a]);
                else {
                  closure->stacks[a] = CS_NIL;
                  cs_mutator_raise(mut, cs_mutator_easy_error(mut,
                    CS_CLASS_TYPEERROR, "%s.__sub is not callable",
                    val[1]->klass->classname));
                } 
              }
              else goto sub_error;
              break;

            default:
              sub_error: closure->stacks[a] = CS_NIL;
              cs_mutator_raise(mut, cs_mutator_easy_error(mut,
                CS_CLASS_TYPEERROR, "invalid operands for Object.__sub"));
              break;
          }
          break;

        case CS_OPCODE_MUL:
          a = cs_bytecode_get_a(code);
          b = cs_bytecode_get_b(code);
          c = cs_bytecode_get_c(code);
          val[1] = load_rk_value(b);
          val[2] = load_rk_value(c);
          if (cs_value_isint(val[1])) {
            if ((temp1 = cs_int_mul(mut, val[1], val[2])) == NULL)
              closure->stacks[a] = CS_NIL;
            else closure->stacks[a] = temp1;
            break;
          }
          switch (val[1]->type) {
            case CS_VALUE_REAL:
              if ((temp1 = cs_real_mul(mut, val[1], val[2])) == NULL)
                closure->stacks[a] = CS_NIL;
              else closure->stacks[a] = temp1;
              break;

            case CS_VALUE_STRING:
              if ((temp1 = cs_string_mul(mut, val[1], val[2])) == NULL)
                closure->stacks[a] = CS_NIL;
              else closure->stacks[a] = temp1;
              break;

            case CS_VALUE_INSTANCE:
              temp1 = cs_mutator_member_find(mut, val[1], "__mul", 5);
              if (temp1) {
                if (temp1->type == CS_VALUE_BUILTIN)
                  closure->stacks[a] = temp1->builtin2(mut, val[1], val[2]);
                else {
                  closure->stacks[a] = CS_NIL;
                  cs_mutator_raise(mut, cs_mutator_easy_error(mut,
                    CS_CLASS_TYPEERROR, "%s.__mul is not callable",
                    val[1]->klass->classname));
                } 
              }
              else goto mul_error;
              break;

            default:
              mul_error: closure->stacks[a] = CS_NIL;
              cs_mutator_raise(mut, cs_mutator_easy_error(mut,
                CS_CLASS_TYPEERROR, "invalid operands for Object.__mul"));
              break;
          }
          break;

        case CS_OPCODE_DIV:
          a = cs_bytecode_get_a(code);
          b = cs_bytecode_get_b(code);
          c = cs_bytecode_get_c(code);
          val[1] = load_rk_value(b);
          val[2] = load_rk_value(c);
          if (cs_value_isint(val[1])) {
            if ((temp1 = cs_int_div(mut, val[1], val[2])) == NULL)
              closure->stacks[a] = CS_NIL;
            else closure->stacks[a] = temp1;
            break;
          }
          switch (val[1]->type) {
            case CS_VALUE_REAL:
              if ((temp1 = cs_real_div(mut, val[1], val[2])) == NULL)
                closure->stacks[a] = CS_NIL;
              else closure->stacks[a] = temp1;
              break;

            case CS_VALUE_INSTANCE:
              temp1 = cs_mutator_member_find(mut, val[1], "__div", 5);
              if (temp1) {
                if (temp1->type == CS_VALUE_BUILTIN)
                  closure->stacks[a] = temp1->builtin2(mut, val[1], val[2]);
                else {
                  closure->stacks[a] = CS_NIL;
                  cs_mutator_raise(mut, cs_mutator_easy_error(mut,
                    CS_CLASS_TYPEERROR, "%s.__div is not callable",
                    val[1]->klass->classname));
                } 
              }
              else goto div_error;
              break;

            default:
              div_error: closure->stacks[a] = CS_NIL;
              cs_mutator_raise(mut, cs_mutator_easy_error(mut,
                CS_CLASS_TYPEERROR, "invalid operands for Object.__div"));
              break;
          }
          break;

        case CS_OPCODE_MOD:
          a = cs_bytecode_get_a(code);
          b = cs_bytecode_get_b(code);
          c = cs_bytecode_get_c(code);
          val[1] = load_rk_value(b);
          val[2] = load_rk_value(c);
          if (cs_value_isint(val[1])) {
            if ((temp1 = cs_int_mod(mut, val[1], val[2])) == NULL)
              closure->stacks[a] = CS_NIL;
            else closure->stacks[a] = temp1;
            break;
          }
          switch (val[1]->type) {
            case CS_VALUE_INSTANCE:
              temp1 = cs_mutator_member_find(mut, val[1], "__mod", 5);
              if (temp1) {
                if (temp1->type == CS_VALUE_BUILTIN)
                  closure->stacks[a] = temp1->builtin2(mut, val[1], val[2]);
                else {
                  closure->stacks[a] = CS_NIL;
                  cs_mutator_raise(mut, cs_mutator_easy_error(mut,
                    CS_CLASS_TYPEERROR, "%s.__mod is not callable",
                    val[1]->klass->classname));
                } 
              }
              else goto mod_error;
              break;

            default:
              mod_error: closure->stacks[a] = CS_NIL;
              cs_mutator_raise(mut, cs_mutator_easy_error(mut,
                CS_CLASS_TYPEERROR, "invalid operands for Object.__mod"));
              break;
          }
          break;

        case CS_OPCODE_POW:
          a = cs_bytecode_get_a(code);
          b = cs_bytecode_get_b(code);
          c = cs_bytecode_get_c(code);
          val[1] = load_rk_value(b);
          val[2] = load_rk_value(c);
          if (cs_value_isint(val[1])) {
            if ((temp1 = cs_int_pow(mut, val[1], val[2])) == NULL)
              closure->stacks[a] = CS_NIL;
            else closure->stacks[a] = temp1;
            break;
          }
          switch (val[1]->type) {
            case CS_VALUE_REAL:
              if ((temp1 = cs_real_pow(mut, val[1], val[2])) == NULL)
                closure->stacks[a] = CS_NIL;
              else closure->stacks[a] = temp1;
              break;

            case CS_VALUE_INSTANCE:
              temp1 = cs_mutator_member_find(mut, val[1], "__pow", 5);
              if (temp1) {
                if (temp1->type == CS_VALUE_BUILTIN)
                  closure->stacks[a] = temp1->builtin2(mut, val[1], val[2]);
                else {
                  closure->stacks[a] = CS_NIL;
                  cs_mutator_raise(mut, cs_mutator_easy_error(mut,
                    CS_CLASS_TYPEERROR, "%s.__pow is not callable",
                    val[1]->klass->classname));
                } 
              }
              else goto pow_error;
              break;

            default:
              pow_error: closure->stacks[a] = CS_NIL;
              cs_mutator_raise(mut, cs_mutator_easy_error(mut,
                CS_CLASS_TYPEERROR, "invalid operands for Object.__pow"));
              break;
          }
          break;

        case CS_OPCODE_NEG:
          a = cs_bytecode_get_a(code);
          b = cs_bytecode_get_b(code);
          val[1] = load_rk_value(b);
          if (cs_value_isint(val[1])) {
            if ((temp1 = cs_int_neg(mut, val[1])) == NULL)
              closure->stacks[a] = CS_NIL;
            else closure->stacks[a] = temp1;
            break;
          }
          switch (val[1]->type) {
            case CS_VALUE_REAL:
              if ((temp1 = cs_real_neg(mut, val[1])) == NULL)
                closure->stacks[a] = CS_NIL;
              else closure->stacks[a] = temp1;
              break;

            case CS_VALUE_INSTANCE:
              temp1 = cs_mutator_member_find(mut, val[1], "__neg", 5);
              if (temp1) {
                if (temp1->type == CS_VALUE_BUILTIN)
                  closure->stacks[a] = temp1->builtin1(mut, val[1]);
                else {
                  closure->stacks[a] = CS_NIL;
                  cs_mutator_raise(mut, cs_mutator_easy_error(mut,
                    CS_CLASS_TYPEERROR, "%s.__neg is not callable",
                    val[1]->klass->classname));
                }
              }
              else goto neg_error;
              break;

            default:
              neg_error: closure->stacks[a] = CS_NIL;
              cs_mutator_raise(mut, cs_mutator_easy_error(mut,
                CS_CLASS_TYPEERROR, "invalid operands for Object.__neg"));
              break;
          }
          break;

        case CS_OPCODE_AND:
          a = cs_bytecode_get_a(code);
          b = cs_bytecode_get_b(code);
          c = cs_bytecode_get_c(code);
          val[1] = load_rk_value(b);
          val[2] = load_rk_value(c);
          if (cs_value_isint(val[1])) {
            if ((temp1 = cs_int_and(mut, val[1], val[2])) == NULL)
              closure->stacks[a] = CS_NIL;
            else closure->stacks[a] = temp1;
            break;
          }
          switch (val[1]->type) {
            case CS_VALUE_INSTANCE:
              temp1 = cs_mutator_member_find(mut, val[1], "__and", 5);
              if (temp1) {
                if (temp1->type == CS_VALUE_BUILTIN)
                  closure->stacks[a] = temp1->builtin2(mut, val[1], val[2]);
                else {
                  closure->stacks[a] = CS_NIL;
                  cs_mutator_raise(mut, cs_mutator_easy_error(mut,
                    CS_CLASS_TYPEERROR, "%s.__and is not callable",
                    val[1]->klass->classname));
                } 
              }
              else goto and_error;
              break;

            default:
              and_error: closure->stacks[a] = CS_NIL;
              cs_mutator_raise(mut, cs_mutator_easy_error(mut,
                CS_CLASS_TYPEERROR, "invalid operands for Object.__and"));
              break;
          }
          break;

        case CS_OPCODE_OR:
          a = cs_bytecode_get_a(code);
          b = cs_bytecode_get_b(code);
          c = cs_bytecode_get_c(code);
          val[1] = load_rk_value(b);
          val[2] = load_rk_value(c);
          if (cs_value_isint(val[1])) {
            if ((temp1 = cs_int_or(mut, val[1], val[2])) == NULL)
              closure->stacks[a] = CS_NIL;
            else closure->stacks[a] = temp1;
            break;
          }
          switch (val[1]->type) {
            case CS_VALUE_INSTANCE:
              temp1 = cs_mutator_member_find(mut, val[1], "__or", 4);
              if (temp1) {
                if (temp1->type == CS_VALUE_BUILTIN)
                  closure->stacks[a] = temp1->builtin2(mut, val[1], val[2]);
                else {
                  closure->stacks[a] = CS_NIL;
                  cs_mutator_raise(mut, cs_mutator_easy_error(mut,
                    CS_CLASS_TYPEERROR, "%s.__or is not callable",
                    val[1]->klass->classname));
                } 
              }
              else goto or_error;
              break;

            default:
              or_error: closure->stacks[a] = CS_NIL;
              cs_mutator_raise(mut, cs_mutator_easy_error(mut,
                CS_CLASS_TYPEERROR, "invalid operands for Object.__or"));
              break;
          }
          break;

        case CS_OPCODE_XOR:
          a = cs_bytecode_get_a(code);
          b = cs_bytecode_get_b(code);
          c = cs_bytecode_get_c(code);
          val[1] = load_rk_value(b);
          val[2] = load_rk_value(c);
          if (cs_value_isint(val[1])) {
            if ((temp1 = cs_int_xor(mut, val[1], val[2])) == NULL)
              closure->stacks[a] = CS_NIL;
            else closure->stacks[a] = temp1;
            break;
          }
          switch (val[1]->type) {
            case CS_VALUE_INSTANCE:
              temp1 = cs_mutator_member_find(mut, val[1], "__xor", 5);
              if (temp1) {
                if (temp1->type == CS_VALUE_BUILTIN)
                  closure->stacks[a] = temp1->builtin2(mut, val[1], val[2]);
                else {
                  closure->stacks[a] = CS_NIL;
                  cs_mutator_raise(mut, cs_mutator_easy_error(mut,
                    CS_CLASS_TYPEERROR, "%s.__xor is not callable",
                    val[1]->klass->classname));
                } 
              }
              else goto xor_error;
              break;

            default:
              xor_error: closure->stacks[a] = CS_NIL;
              cs_mutator_raise(mut, cs_mutator_easy_error(mut,
                CS_CLASS_TYPEERROR, "invalid operands for Object.__xor"));
              break;
          }
          break;

        case CS_OPCODE_NOT:
          a = cs_bytecode_get_a(code);
          b = cs_bytecode_get_b(code);
          val[1] = load_rk_value(b);
          if (cs_value_isint(val[1])) {
            if ((temp1 = cs_int_not(mut, val[1])) == NULL)
              closure->stacks[a] = CS_NIL;
            else closure->stacks[a] = temp1;
            break;
          }
          switch (val[1]->type) {
            case CS_VALUE_INSTANCE:
              temp1 = cs_mutator_member_find(mut, val[1], "__not", 5);
              if (temp1) {
                if (temp1->type == CS_VALUE_BUILTIN)
                  closure->stacks[a] = temp1->builtin1(mut, val[1]);
                else {
                  closure->stacks[a] = CS_NIL;
                  cs_mutator_raise(mut, cs_mutator_easy_error(mut,
                    CS_CLASS_TYPEERROR, "%s.__not is not callable",
                    val[1]->klass->classname));
                }
              }
              else goto not_error;
              break;

            default:
              not_error: closure->stacks[a] = CS_NIL;
              cs_mutator_raise(mut, cs_mutator_easy_error(mut,
                CS_CLASS_TYPEERROR, "invalid operands for Object.__not"));
              break;
          }
          break;

        case CS_OPCODE_JMP:
          simm = cs_bytecode_get_imm(code);
          closure->pc += simm-1;
          break;

        case CS_OPCODE_IF:
          a = cs_bytecode_get_a(code);
          val[0] = load_rk_value(a);
          simm = cs_bytecode_get_imm(code);
          if (cs_value_isint(val[0]))
            break;
          switch (val[0]->type) {
            case CS_VALUE_NIL:
            case CS_VALUE_FALSE:
              closure->pc += simm-1;
              break;

            case CS_VALUE_INSTANCE:
              // Object implements __bool
              temp1 = cs_mutator_member_find(mut, val[1], "__bool", 6);
              if (temp1->type == CS_VALUE_BUILTIN) {
                if (temp1->builtin1(mut, val[1]) == CS_FALSE)
                  closure->pc += simm-1;
              } else {
                cs_mutator_raise(mut, cs_mutator_easy_error(mut,
                  CS_CLASS_TYPEERROR, "%s.__bool is not callable",
                  val[1]->klass->classname));
              }
              break;

            default:
              // Everything else is true
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
      mut->epoch = (mut->epoch + 1) & CS_NURSERY_EPOCH;
      sem_post(&mut->cs->gc_sync);
      sem_wait(&mut->cs->gc_sync);
    }

    // implicit Return to calling function
    CsClosure* temp_clos = closure;
    closure = closure->parent;
    cs_free_object(temp_clos);
  }

  return NULL;
}

CsValue cs_mutator_value_as_string(CsMutator* mut, CsValue value) {
  CsValue temp1;
  if (cs_value_isint(value)) {
    return cs_mutator_new_string_formatted(mut,
      "%"PRIiPTR, cs_value_toint(value));
  }
  switch (value->type) {
    case CS_VALUE_NIL:
      return cs_mutator_copy_string(mut, "nil", 0, 3, 3);
      break;

    case CS_VALUE_TRUE:
      return cs_mutator_copy_string(mut, "True", 0, 4, 4);
      break;

    case CS_VALUE_FALSE:
      return cs_mutator_copy_string(mut, "False", 0, 5, 5);
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
      temp1 = cs_mutator_member_find(mut, value, "__as_string", 11);
      if (temp1) {
        if (temp1->type == CS_VALUE_BUILTIN)
          return temp1->builtin1(mut, value);
        else {
          cs_mutator_raise(mut, cs_mutator_easy_error(mut,
            CS_CLASS_TYPEERROR, "%s.__as_string is not callable",
            value->klass->classname));
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

static bool epoch_mark(CsMutator* mut, void* data) {
  return false;
}

int cs_mutator_collect(CsMutator* mut) {
  // We have to acquire the gc lock, but another thread could have already
  // acquired it and is waiting for us to decrement our sem and let their
  // gc take place. In this case, we must let them have our semaphore.
  if (pthread_mutex_trylock(&mut->cs->gc_lock) == EBUSY) {
    sem_post(&mut->cs->gc_sync);
    pthread_cond_wait(&mut->cs->gc_done, &mut->gc_cv_mut);
    sem_wait(&mut->cs->gc_sync);
    return 1;
  }

  // Now we have full control of the system.
  // We've (safely) stopped the world.
  cs_debug("Yo DAWG! I heard you like garbage collection.\n");

  // The epoch mark will trace all objects and mark those with
  // epochs different than their mutators
  cs_list_traverse(mut->cs->mutators, (bool (*)(void*,void*)) epoch_mark, NULL);

  // We need to scan the entire root set of each mutator.
  pthread_mutex_unlock(&mut->cs->gc_lock);
  pthread_cond_broadcast(&mut->cs->gc_done);
  return 0;
}

CsValue cs_mutator_easy_error(
  CsMutator* mut,
  CsValue klass,
  const char* format,
  ...)
{
  CsValue value;
  va_list args;
  va_start(args, format);
  value = cs_mutator_new_instance(mut, klass);
  cs_hash_insert(value->dict, "what", 4,
    cs_mutator_new_string_va_list(mut, format, args));
  va_end(args);
  return value;
}
