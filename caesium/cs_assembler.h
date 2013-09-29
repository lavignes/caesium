#ifndef _CS_ASSEMBLER_H_
#define _CS_ASSEMBLER_H_

#include "cs_bytecode.h"

typedef struct CsAssembler {
  size_t filesize; // size of file in characters
  size_t offset;   // offset in characters
  const char* file;
  const char* pos;
} CsAssembler;

CsAssembler* cs_assembler_new();

CsBytecode* cs_assembler_assemble(CsAssembler* assembler, const char* u8str);

void cs_assembler_free(CsAssembler* assembler);


#endif /* _CS_ASSEMBLER_H_ */