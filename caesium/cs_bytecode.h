#ifndef _CS_BYTECODE_H
#define _CS_BYTECODE_H

typedef enum CsOpcode {
  CS_OPCODE_MOVE,
  CS_OPCODE_LOADK,
  CS_OPCODE_LOADG,
  CS_OPCODE_STORG,
  CS_OPCODE_ADD,
  CS_OPCODE_SUB,
  CS_OPCODE_MUL,
  CS_OPCODE_DIV,
  CS_OPCODE_MOD,
  CS_OPCODE_POW,
  CS_OPCODE_NEG,
  CS_OPCODE_AND,
  CS_OPCODE_OR,
  CS_OPCODE_XOR,
  CS_OPCODE_NOT,
  CS_OPCODE_SHL,
  CS_OPCODE_SHR,
  CS_OPCODE_JMP,
  CS_OPCODE_EQ,
  CS_OPCODE_LT,
  CS_OPCODE_LE,
  CS_OPCODE_RETURN,
} CsOpcode;

#define CS_BYTECODE_OPCODE_MASK 0x0000003F;
#define CS_BYTECODE_A_MASK      0x00003FC0;
#define CS_BYTECODE_C_MASK      0x007FC000;
#define CS_BYTECODE_B_MASK      0xFF800000;
#define CS_BYTECODE_IMM_MASK    0xFFFFC000;

/**
 * a bytecode instruction
 * format:
 *   32[  b:9 c:9  a:8 opcode:6 ]0
 *   32[   imm:18  a:8 opcode:6 ]0
 *   32[  simm:18  a:8 opcode:6 ]0
 */
typedef uint32_t CsByteCode;

#define cs_bytecode_get_opcode(bytecode) (bytecode & CS_BYTECODE_OPCODE_MASK)
#define cs_bytecode_get_a(bytecode)   ((bytecode & CS_BYTECODE_A_MASK) >> 6)
#define cs_bytecode_get_b(bytecode)   ((bytecode & CS_BYTECODE_C_MASK) >> 14)
#define cs_bytecode_get_c(bytecode)   ((bytecode & CS_BYTECODE_B_MASK) >> 23)
#define cs_bytecode_get_imm(bytecode) ((bytecode & CS_BYTECODE_IMM_MASK) >> 14)

#define cs_bytecode_set_opcode(bytecode, opcode) \
  ((bytecode & ~CS_BYTECODE_OPCODE_MASK) | opcode)

#define cs_bytecode_set_a(bytecode, rd) \
  ((bytecode & ~CS_BYTECODE_A_MASK) | (rd << 6))

#define cs_bytecode_set_c(bytecode, rt) \
  ((bytecode & ~CS_BYTECODE_C_MASK) | (rt << 14))

#define cs_bytecode_set_b(bytecode, rs) \
  ((bytecode & ~CS_BYTECODE_B_MASK) | (rd << 23))

#define cs_bytecode_set_imm(bytecode, imm) \
  ((bytecode & ~CS_BYTECODE_IMM_MASK) | (imm << 14))

typedef enum CsByteConstType {
  CS_CONST_TYPE_NIL,
  CS_CONST_TYPE_BOOL,
  CS_CONST_TYPE_INT,
  CS_CONST_TYPE_REAL,
  CS_CONST_TYPE_STRING,
} CsByteConstType;

/**
 * a bytecode const
 */
typedef struct CsByteConst {
  CsByteConstType type;
  void* k;
} CsByteConst;

/**
 * a bytecode function
 */
typedef struct CsByteFunction {
  size_t nupvals;
  size_t nparams;
  size_t nstacks;
  size_t ncodes;
  CsByteCode* codes;
  size_t nconsts;
  CsByteConst* consts;
} CsByteFunction;

/**
 * a bytecode chunk
 */
typedef struct CsByteChunk {

  CsByteFunction entry;

} CsByteChunk;

#endif /* _CS_BYTECODE_H */
