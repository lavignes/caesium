#ifndef _CS_ASSEMBLER_H_
#define _CS_ASSEMBLER_H_

#include "cs_bytecode.h"
#include "cs_list.h"
#include "cs_hash.h"

typedef enum CsAsmState {
  CS_ASM_STATE_INIT,     // initial parser state
  CS_ASM_STATE_BLOCK,
  CS_ASM_STATE_COMMENT,
  CS_ASM_STATE_LABEL,
  CS_ASM_STATE_PSEUDO,
  CS_ASM_STATE_OP,
  CS_ASM_STATE_ENTRY,
  CS_ASM_STATE_FUNC,
  CS_ASM_STATE_CONSTN,   // Const number
  CS_ASM_STATE_CONSTS,   // Const string
  CS_ASM_STATE_ARGS,
  CS_ASM_STATE_ARGS_READ,

  // STATES for opcodes
  CS_ASM_STATE_MOVE,
  CS_ASM_STATE_LOADK,
  CS_ASM_STATE_LOADG,
  CS_ASM_STATE_STORG,
  CS_ASM_STATE_LOADI,
  CS_ASM_STATE_STORI,
  CS_ASM_STATE_LODUP,
  CS_ASM_STATE_STRUP,
  CS_ASM_STATE_PUTS,
  CS_ASM_STATE_NEW,
  CS_ASM_STATE_ADD,
  CS_ASM_STATE_SUB,
  CS_ASM_STATE_MUL,
  CS_ASM_STATE_DIV,
  CS_ASM_STATE_MOD,
  CS_ASM_STATE_POW,
  CS_ASM_STATE_NEG,
  CS_ASM_STATE_AND,
  CS_ASM_STATE_OR,
  CS_ASM_STATE_XOR,
  CS_ASM_STATE_NOT,
  CS_ASM_STATE_SHL,
  CS_ASM_STATE_SHR,
  CS_ASM_STATE_JMP,
  CS_ASM_STATE_IF,
  CS_ASM_STATE_LT,
  CS_ASM_STATE_LE,
  CS_ASM_STATE_EQ,
  CS_ASM_STATE_CLOS,
  CS_ASM_STATE_MOVUP,
  CS_ASM_STATE_CPYUP,
  CS_ASM_STATE_CALL,
  CS_ASM_STATE_RET,
  CS_ASM_STATE_RAISE,
  CS_ASM_STATE_CATCH,
  CS_ASM_STATE_SPWN,
  CS_ASM_STATE_SEND,
  CS_ASM_STATE_RECV,
} CsAsmState;

typedef struct CsAssembler {
} CsAssembler;

typedef enum CsPseudoOp {
  CS_PSEUDO_ENTRY,
  CS_PSEUDO_FUNC,
  CS_PSEUDO_CONST,
  CS_PSEUDO_END,
  CS_PSEUDO_RESQ,
} CsPseudoOp;

CsAssembler* cs_assembler_new();

CsByteChunk* cs_assembler_assemble(
  CsAssembler* assembler,
  const char* u8str,
  size_t size);

void cs_assembler_free(CsAssembler* assembler);


#endif /* _CS_ASSEMBLER_H_ */
