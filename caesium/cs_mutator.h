#ifndef _CS_MUTATOR_H_
#define _CS_MUTATOR_H_

#include <pthread.h>

#include "cs_runtime.h"
#include "cs_assembler.h"
#include "cs_value.h"
#include "cs_nursery.h"

typedef struct CsMutator {

  bool started;
  CsRuntime* cs;
  pthread_t thread;
  pthread_mutex_t gc_cv_mut; // Used to wait for gc to complete
  void* (*entry_point)(struct CsMutator*, void*);
  void* data;

  // Current CsValue epoch
  uint32_t epoch;

  bool error;
  CsValue error_register;

  CsList* nursery;
  CsList* freelist;

} CsMutator;

typedef struct CsClosure {

  struct CsClosure* parent;
  CsByteFunction* cur_func;
  size_t pc;
  uintptr_t* codes;
  size_t ncodes;
  CsValue* params;
  CsValue* upvals;
  CsValue* stacks;

} CsClosure;

CsValue cs_mutator_copy_string(
  CsMutator* mut,
  const char* u8str,
  uint32_t hash,
  size_t size,
  size_t length);

CsValue cs_mutator_new_string_formatted(
  CsMutator* mut,
  const char* format,
  ...);

CsValue cs_mutator_new_real(CsMutator* mut, double real);

CsValue cs_mutator_new_class(
  CsMutator* mut,
  const char* name,
  CsHash* dict,
  CsArray* bases);

CsValue cs_mutator_new_builtin(CsMutator* mut, CsBuiltin builtin);

CsValue cs_mutator_new_instance(CsMutator* mut, CsValue klass);

CsMutator* cs_mutator_new(CsRuntime* cs);

void cs_mutator_free(CsMutator* mut);

void cs_mutator_start(
  CsMutator* mut,
  void* (*entry_point)(struct CsMutator*, void*),
  void* data);

/**
 * Execute a bytecode chunk (default mutator entry point)
 * @param mut   a mutator
 * @param chunk a bytechunk
 * @return thread return value
 */
void* cs_mutator_exec(CsMutator* mut, CsByteChunk* chunk);

void cs_mutator_raise(CsMutator* mut, CsValue error);

int cs_mutator_collect(CsMutator* mut);

CsValue cs_mutator_easy_error(
  CsMutator* mut,
  CsValue klass,
  const char* format,
  ...);

CsValue cs_mutator_value_as_string(CsMutator* mut, CsValue value);

/**
 * Locates a member in a value.
 * This searches the value's local dictionary, class
 * dictionary, and all inherited class dictionaries.
 * @param  mut      a mutator
 * @param  instance an instance
 * @param  key      field name
 * @param  key_sz   field name size
 * @return          matched value or NULL if not found
 */
CsValue cs_mutator_member_find(
  CsMutator* mut,
  CsValue instance,
  const char* key,
  size_t key_sz);

#endif /* _CS_MUTATOR_H_ */
