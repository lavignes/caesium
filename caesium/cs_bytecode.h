#ifndef _CS_BYTECODE_H
#define _CS_BYTECODE_H

typedef enum CsOpcode {
  CS_OPCODE_MOVE,    // move  A B   -> R[A] = R[B]
  CS_OPCODE_LOADK,   // loadk A B   -> R[A] = K[B]
  CS_OPCODE_LOADG,   // loadg A B   -> R[A] = G[K[B]]
  CS_OPCODE_STORG,   // storg A B   -> G[K[B]] = R[A]
  CS_OPCODE_LOADI,   // loadi A B C -> R[A] = R[B][RK[C]]
  CS_OPCODE_STORI,   // stori A B C -> R[B][RK[C]] = R[A]
  CS_OPCODE_ADD,     // add A B C   -> R[A] = RK[B] + RK[C]
  CS_OPCODE_SUB,     // sub A B C   -> R[A] = RK[B] - RK[C]
  CS_OPCODE_MUL,     // sub A B C   -> R[A] = RK[B] * RK[C]
  CS_OPCODE_DIV,     // div A B C   -> R[A] = RK[B] / RK[C]
  CS_OPCODE_MOD,     // mod A B C   -> R[A] = RK[B] % RK[C]
  CS_OPCODE_POW,     // pow A B C   -> R[A] = RK[B] ** RK[C]
  CS_OPCODE_NEG,     // neg A B     -> R[A] = - RK[B]
  CS_OPCODE_AND,     // and A B C   -> R[A] = RK[B] & RK[C]
  CS_OPCODE_OR,      // or  A B C   -> R[A] = RK[B] | RK[C]
  CS_OPCODE_XOR,     // xor A B C   -> R[A] = RK[B] ^ RK[C]
  CS_OPCODE_NOT,     // not A B     -> R[A] = ~ RK[B]
  CS_OPCODE_SHL,     // shl A B C   -> R[A] = RK[B] << RK[C]
  CS_OPCODE_SHR,     // shr A B C   -> R[A] = RK[B] >> RK[C]
  CS_OPCODE_JMP,     // jmp sIMM    -> PC += sIMM
  CS_OPCODE_LT,      // lt  A B C   -> R[A] = RK[B] <  RK[C]
  CS_OPCODE_LE,      // le  A B C   -> R[A] = RK[B] <= RK[C]
  CS_OPCODE_EQ,      // eq  A B C   -> R[A] = RK[B] == RK[C]
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
 *   32[  B:9 C:9  A:8 opcode:6 ]0
 *   32[   IMM:18  A:8 opcode:6 ]0
 *   32[  sIMM:18  A:8 opcode:6 ]0
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