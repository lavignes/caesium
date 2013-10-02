#ifndef _CS_ASSEMBLER_H_
#define _CS_ASSEMBLER_H_

#include "cs_bytecode.h"
#include "cs_list.h"

typedef enum CsAsmState {
  CS_ASM_STATE_INIT,     // initial parser state
  CS_ASM_STATE_COMMENT,
  CS_ASM_STATE_PSEUDO,
  CS_ASM_STATE_OP,
} CsAsmState;

typedef struct CsAssembler {
  size_t filesize; // size of file in characters
  size_t offset;   // offset in characters
  size_t line;
  size_t col;
  const char* file;
  const char* pos;
  CsList* statestack;
} CsAssembler;

typedef enum CsPseudoOp {
  CS_PSEUDO_MAIN,
  CS_PSEUDO_CONST,
} CsPseudoOp;

CsAssembler* cs_assembler_new();

CsByteChunk* cs_assembler_assemble(CsAssembler* assembler, const char* u8str);

void cs_assembler_free(CsAssembler* assembler);


#endif /* _CS_ASSEMBLER_H_ */
