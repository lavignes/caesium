#include "cs_runtime.h"
#include "cs_mutator.h"
#include "cs_lexer.h"

CsRuntime* cs_runtime_new() {
  CsRuntime* cs = cs_alloc_object(CsRuntime);
  if (cs == NULL)
    cs_exit(CS_REASON_NOMEM);
  cs->mutators = cs_list_new();
  return cs;
}

bool free_mutators(void* node, void* data) {
  cs_mutator_free((CsMutator*) node);
  return true;
}

void cs_runtime_free(CsRuntime* cs) {
  cs_list_traverse(cs->mutators, free_mutators, NULL);
  cs_list_free(cs->mutators);
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
  if (file_buffer == NULL)
    cs_exit(CS_REASON_NOMEM);
  fread(file_buffer, file_size, 1, input_file);
  fclose(input_file);
  cs_runtime_dostring(cs, file_buffer);
}

int entry(CsMutator* mut, void* data) {
  // const char* u8str = data;
  CsLexer* lex = cs_lexer_new();
  cs_lexer_free(lex);

  return 0;
} 

void cs_runtime_dostring(CsRuntime* cs, const char* u8str) {
  CsMutator* mut0 = cs_mutator_new(cs);
  cs_list_push_back(cs->mutators, mut0);
  cs_mutator_start(mut0, entry, (void*) u8str);
  if (thrd_join(mut0->thread, NULL) != thrd_success)
    cs_exit(CS_REASON_THRDFATAL);
  free((void*) u8str);
}