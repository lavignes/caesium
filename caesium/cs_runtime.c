#include "cs_runtime.h"
#include "cs_mutator.h"
#include "cs_lexer.h"
#include "cs_assembler.h"

extern void setup_assembler();
extern void shutdown_assembler();

CsRuntime* cs_runtime_new() {
  CsRuntime* cs = cs_alloc_object(CsRuntime);
  if (cs_unlikely(cs == NULL))
    cs_exit(CS_REASON_NOMEM);
  if (mtx_init(&cs->globals_lock, mtx_plain) != thrd_success)
    cs_exit(CS_REASON_THRDFATAL);
  cs->globals = cs_hash_new();
  cs->mutators = cs_list_new();
  setup_assembler();
  return cs;
}

bool free_mutators(void* node, void* data) {
  cs_mutator_free((CsMutator*) node);
  return true;
}

void cs_runtime_free(CsRuntime* cs) {
  cs_list_traverse(cs->mutators, free_mutators, NULL);
  cs_list_free(cs->mutators);
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

struct entrydata {
  const char* u8str;
  size_t size;
};

/*
  Assembly-based mutator entry point
 */
static int entry(CsMutator* mut, void* data) {
  struct entrydata* edata = data;
  CsAssembler* assembler = cs_assembler_new();
  CsByteChunk* chunk =
    cs_assembler_assemble(assembler, edata->u8str, edata->size);
  cs_free_object((char*) edata->u8str);
  cs_assembler_free(assembler);
  cs_mutator_exec(mut, chunk);
  cs_bytechunk_free(chunk);
  return 0;
}

void cs_runtime_doassembly(CsRuntime* cs, const char* u8str, size_t size) {
  CsMutator* mut0 = cs_mutator_new(cs);
  cs_list_push_back(cs->mutators, mut0);
  struct entrydata edata = {u8str, size};
  cs_mutator_start(mut0, entry, &edata);
  if (thrd_join(mut0->thread, NULL) != thrd_success)
    cs_exit(CS_REASON_THRDFATAL);
}
