#include "cs_common.h"
#include "cs_assembler.h"
#include "cs_unicode.h"

CsAssembler* cs_assembler_new() {
  CsAssembler* assembler = cs_alloc_object(CsAssembler);
  if (assembler == NULL)
    cs_exit(CS_REASON_NOMEM);
  assembler->filesize = 0;
  assembler->offset = 0;
  assembler->file = NULL;
  assembler->pos = NULL;
  return assembler;
}

CsBytecode* cs_assembler_assemble(CsAssembler* assembler, const char* u8str) {
  assembler->filesize = cs_utf8_strnlen(u8str, -1);
  assembler->file = u8str;
  assembler->pos = u8str;
  assembler->offset = 0;
  while (assembler->offset < assembler->filesize) {
    printf("'%c'\n", *assembler->pos);
    assembler->pos = cs_utf8_next(assembler->pos, NULL);
    assembler->offset++;
  }

  return NULL;
}

void cs_assembler_free(CsAssembler* assembler) {
  cs_free_object(assembler);
}