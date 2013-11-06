#include <sys/sysinfo.h>

#include "cs_runtime.h"
#include "cs_mutator.h"
#include "cs_lexer.h"
#include "cs_assembler.h"
#include "cs_value.h"

#include "cs_object.h"
#include "cs_boolean.h"
#include "cs_numeric.h"
#include "cs_string.h"
#include "cs_arrayclass.h"
#include "cs_error.h"

extern void setup_assembler();
extern void shutdown_assembler();

CsRuntime* cs_runtime_new() {
  // We hold these truths to be self-evident
  cs_assert(sizeof(CsValueStruct) == CS_VALUE_SIZE);
  cs_assert(sizeof(CsNurseryPage) == CS_NURSERY_PAGE_SIZE);

  CsRuntime* cs = cs_alloc_object(CsRuntime);
  if (cs_unlikely(cs == NULL))
    cs_exit(CS_REASON_NOMEM);
  if (pthread_mutex_init(&cs->globals_lock, NULL))
    cs_exit(CS_REASON_THRDFATAL);
  if (pthread_mutex_init(&cs->gc_lock, NULL))
    cs_exit(CS_REASON_THRDFATAL);
  if (sem_init(&cs->gc_sync, 0, 0) == -1)
    cs_exit(CS_REASON_THRDFATAL);
  if (pthread_cond_init(&cs->gc_done, NULL))
    cs_exit(CS_REASON_THRDFATAL);
  cs->globals = cs_hash_new();
  cs->mutators = cs_list_new();
  cs->nprocs = get_nprocs();
  setup_assembler();
  return cs;
}

bool free_mutators(void* mut, void* data) {
  cs_mutator_free(mut);
  return false;
}

void cs_runtime_free(CsRuntime* cs) {
  cs_list_traverse(cs->mutators, free_mutators, NULL);
  cs_list_free(cs->mutators);
  cs_hash_free(cs->globals);
  shutdown_assembler();
  cs_free_object(cs);
}

void cs_runtime_dofile(CsRuntime* cs, const char* filename) {
  FILE* input_file = fopen(filename, "r");
  size_t file_size;
  char* file_buffer;
  if (input_file == NULL)
    cs_exit(CS_REASON_BADFILE);
  fseek(input_file, 0, SEEK_END);
  file_size = ftell(input_file);
  rewind(input_file);
  
  file_buffer = malloc(file_size * sizeof(char));
  if (cs_unlikely(file_buffer == NULL))
    cs_exit(CS_REASON_NOMEM);
  fread(file_buffer, file_size, 1, input_file);
  fclose(input_file);
  //cs_runtime_dostring(cs, file_buffer);
  cs_runtime_doassembly(cs, file_buffer, file_size);
  cs_free_object(file_buffer);
}

void cs_runtime_dostring(CsRuntime* cs, const char* u8str) {
  cs_exit(CS_REASON_UNIMPLEMENTED);
  // CsMutator* mut0 = cs_mutator_new(cs);
  // cs_list_push_back(cs->mutators, mut0);
  // cs_mutator_start(mut0, entry, (void*) u8str);
  // if (thrd_join(mut0->thread, NULL) != thrd_success)
  //   cs_exit(CS_REASON_THRDFATAL);
  // free((void*) u8str);
}

static int print(CsMutator* mut,
  int argc, CsValue* args, int retc, CsValue* rets) {
  int i;
  CsValue value;
  for (i = 0; i < argc; i++) {
    value = cs_mutator_value_as_string(mut, args[i]);
    printf("%s\n", cs_value_tostring(value));
  }
  return 0;
}

static void load_builtins(CsRuntime* cs, CsMutator* mut) {
  cs_hash_insert(cs->globals, "print", 5, cs_mutator_new_builtin(mut, print));
}

static void create_classes(CsRuntime* cs, CsMutator* mut) {
  // create object class
  cs_initclass_object(mut);
  cs_hash_insert(cs->globals,
    CS_CLASS_OBJECT->classname,
    strlen(CS_CLASS_OBJECT->classname),
    CS_CLASS_OBJECT);

  // create true class
  cs_initclass_true(mut);
  cs_hash_insert(cs->globals,
    CS_CLASS_TRUE->classname,
    strlen(CS_CLASS_TRUE->classname),
    CS_CLASS_TRUE);

  // create false class
  cs_initclass_false(mut);
  cs_hash_insert(cs->globals,
    CS_CLASS_FALSE->classname,
    strlen(CS_CLASS_FALSE->classname),
    CS_CLASS_FALSE);

  // create int class
  cs_initclass_int(mut);
  cs_hash_insert(cs->globals,
    CS_CLASS_INT->classname,
    strlen(CS_CLASS_INT->classname),
    CS_CLASS_INT);

  // create real class
  cs_initclass_real(mut);
  cs_hash_insert(cs->globals,
    CS_CLASS_REAL->classname,
    strlen(CS_CLASS_REAL->classname),
    CS_CLASS_REAL);

  // create string class
  cs_initclass_string(mut);
  cs_hash_insert(cs->globals,
    CS_CLASS_STRING->classname,
    strlen(CS_CLASS_STRING->classname),
    CS_CLASS_STRING);

  // create array class
  cs_initclass_array(mut);
  cs_hash_insert(cs->globals,
    CS_CLASS_ARRAY->classname,
    strlen(CS_CLASS_ARRAY->classname),
    CS_CLASS_ARRAY);

  // create error class
  cs_initclass_error(mut);
  cs_hash_insert(cs->globals,
    CS_CLASS_ERROR->classname,
    strlen(CS_CLASS_ERROR->classname),
    CS_CLASS_ERROR);

  // create nameerror class
  cs_initclass_nameerror(mut);
  cs_hash_insert(cs->globals,
    CS_CLASS_NAMEERROR->classname,
    strlen(CS_CLASS_NAMEERROR->classname),
    CS_CLASS_NAMEERROR);

  // create typeerror class
  cs_initclass_typeerror(mut);
  cs_hash_insert(cs->globals,
    CS_CLASS_TYPEERROR->classname,
    strlen(CS_CLASS_TYPEERROR->classname),
    CS_CLASS_TYPEERROR);

  // create indexerror class
  cs_initclass_indexerror(mut);
  cs_hash_insert(cs->globals,
    CS_CLASS_INDEXERROR->classname,
    strlen(CS_CLASS_INDEXERROR->classname),
    CS_CLASS_INDEXERROR);
}

static void cleanup_classes() {
  cs_freeclass_object(CS_CLASS_OBJECT);
  cs_freeclass_true(CS_CLASS_TRUE);
  cs_freeclass_false(CS_CLASS_FALSE);
  cs_freeclass_int(CS_CLASS_INT);
  cs_freeclass_real(CS_CLASS_REAL);
  cs_freeclass_string(CS_CLASS_STRING);
  cs_freeclass_array(CS_CLASS_ARRAY);
  cs_freeclass_error(CS_CLASS_ERROR);
  cs_freeclass_nameerror(CS_CLASS_NAMEERROR);
  cs_freeclass_typeerror(CS_CLASS_TYPEERROR);
  cs_freeclass_indexerror(CS_CLASS_INDEXERROR);
}

void cs_runtime_doassembly(CsRuntime* cs, const char* u8str, size_t size) {
  CsMutator* mut0 = cs_mutator_new(cs);
  // Increase the gc_sync semaphore
  sem_post(&cs->gc_sync);
  cs_list_push_back(cs->mutators, mut0);

  create_classes(cs, mut0);
  load_builtins(cs, mut0);

  CsAssembler* assembler = cs_assembler_new();
  CsByteChunk* chunk = cs_assembler_assemble(assembler, u8str, size);
  cs_assembler_free(assembler);

  cs_mutator_start(mut0, (void* (*)(CsMutator*, void*)) cs_mutator_exec, chunk);

  if (pthread_join(mut0->thread, NULL))
    cs_exit(CS_REASON_THRDFATAL);

  cleanup_classes();

  cs_bytechunk_free(chunk);
}
