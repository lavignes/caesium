#include "cs_bytecode.h"

void cs_bytechunk_free(CsByteChunk* chunk) {
  cs_bytefunction_free(chunk->entry);
  cs_free_object(chunk);
}

void cs_bytefunction_free(CsByteFunction* func) {
  cs_array_free(func->funcs);
  cs_array_free(func->codes);
  cs_array_free(func->consts);
  cs_free_object(func);
}

