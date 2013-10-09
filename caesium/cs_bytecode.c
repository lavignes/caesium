#include "cs_bytecode.h"

void cs_bytechunk_free(CsByteChunk* chunk) {
  cs_bytefunction_free(chunk->entry);
  cs_free_object(chunk);
}

static bool free_const(void* konst_ptr, void* data) {
  CsByteConst* konst = konst_ptr;
  if (konst->type == CS_CONST_TYPE_STRING) {
    cs_free_object(konst->string);
  }
  cs_free_object(konst);
  return false;
}

static bool free_func(void* func_ptr, void* data) {
  cs_bytefunction_free((CsByteFunction*) func_ptr);
  return false;
}

void cs_bytefunction_free(CsByteFunction* func) {
  cs_array_free(func->codes);
  cs_array_traverse(func->funcs, free_func, NULL);
  cs_array_free(func->funcs);
  cs_array_traverse(func->consts, free_const, NULL);
  cs_array_free(func->consts);
  cs_free_object(func);
}
