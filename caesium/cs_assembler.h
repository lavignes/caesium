#ifndef _CS_ASSEMBLER_H_
#define _CS_ASSEMBLER_H_

#include "cs_bytecode.h"
#include "cs_list.h"
#include "cs_hash.h"

typedef enum CsAsmState {
  CS_ASM_STATE_INIT,     // initial parser state
  CS_ASM_STATE_BLOCK,
  CS_ASM_STATE_COMMENT,
  CS_ASM_STATE_PSEUDO,
  CS_ASM_STATE_ENTRY,
  CS_ASM_STATE_CONSTN,   // Const number
  CS_ASM_STATE_CONSTS,   // Const string
  CS_ASM_STATE_ARGS,
  CS_ASM_STATE_ARGS_READ,
} CsAsmState;

typedef struct CsAssembler {
} CsAssembler;

typedef enum CsPseudoOp {
  CS_PSEUDO_ENTRY,
  CS_PSEUDO_CONST,
  CS_PSEUDO_END,
} CsPseudoOp;

CsAssembler* cs_assembler_new();

CsByteChunk* cs_assembler_assemble(
  CsAssembler* assembler,
  const char* u8str,
  size_t size);

void cs_assembler_free(CsAssembler* assembler);


#endif /* _CS_ASSEMBLER_H_ */
