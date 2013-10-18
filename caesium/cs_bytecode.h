#ifndef _CS_BYTECODE_H
#define _CS_BYTECODE_H

#include "cs_array.h"

typedef enum CsOpcode {
  CS_OPCODE_MOVE,    // move  A B   -> R[A] = R[B]
  CS_OPCODE_LOADK,   // loadk A B   -> R[A] = K[B]
  CS_OPCODE_LOADG,   // loadg A B   -> R[A] = G[K[B]]
  CS_OPCODE_STORG,   // storg A B   -> G[K[B]] = R[A]
  CS_OPCODE_LOADI,   // loadi A B C -> R[A] = R[B][RK[C]]
  CS_OPCODE_STORI,   // stori A B C -> R[B][RK[C]] = R[A]
  CS_OPCODE_LODUP,   // lodup A B   -> R[A] = U[B]
  CS_OPCODE_STRUP,   // strup A B   -> U[B] = R[A]
  CS_OPCODE_PUTS,    // puts A      -> printf(R[A])
  CS_OPCODE_NEW,     // new A B     -> R[A] = new R[B]
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
  CS_OPCODE_IF,      // if  A sIMM  -> if R[A] then PC++ else PC += sIMM
  CS_OPCODE_LT,      // lt  A B C   -> R[A] = RK[B] <  RK[C]
  CS_OPCODE_LE,      // le  A B C   -> R[A] = RK[B] <= RK[C]
  CS_OPCODE_EQ,      // eq  A B C   -> R[A] = RK[B] == RK[C]
  CS_OPCODE_CLOS,    // clos A IMM  -> R[A] = new F[IMM]
  CS_OPCODE_CPYUP,   // cpyup A B C -> R[A].UP[B] = R[C]
  CS_OPCODE_MOVUP,   // movup A B C -> R[A].UP[B] = UP[C]
  CS_OPCODE_CALL,    // call A B C  -> R[A], ... R[A+C-2] = R[A](R[A+B-1])
  CS_OPCODE_RET,     // ret A B     -> return R[A], ... R[A+B-2]
  CS_OPCODE_RAISE,   // raise A     -> e = R[A] goto RESQ
  CS_OPCODE_SPWN,    // spwn A IMM  -> R[A] = T.append(new thread(F[IMM]))
  CS_OPCODE_SEND,    // send A B    -> R[A].push_back(copymutable R[B])
  CS_OPCODE_RECV,    // recv A B    -> R[A] = R[B].pop_front()
} CsOpcode;

#define CS_BYTECODE_OPCODE_MASK 0x0000003F
#define CS_BYTECODE_A_MASK      0x00003FC0
#define CS_BYTECODE_C_MASK      0x007FC000
#define CS_BYTECODE_B_MASK      0xFF800000
#define CS_BYTECODE_IMM_MASK    0xFFFFC000

/**
 * a bytecode instruction
 * format:
 *   32[  B:9 C:9  A:8 opcode:6 ]0   TYPE1
 *   32[   IMM:18  A:8 opcode:6 ]0   TYPE2
 *   32[  sIMM:18  A:8 opcode:6 ]0   TYPE3
 */
typedef uint32_t CsByteCode;

#define cs_bytecode_get_opcode(bytecode) (bytecode & CS_BYTECODE_OPCODE_MASK)
#define cs_bytecode_get_a(bytecode)   ((bytecode & CS_BYTECODE_A_MASK) >> 6)
#define cs_bytecode_get_b(bytecode)   ((bytecode & CS_BYTECODE_B_MASK) >> 23)
#define cs_bytecode_get_c(bytecode)   ((bytecode & CS_BYTECODE_C_MASK) >> 14)
#define cs_bytecode_get_imm(bytecode) ((bytecode & CS_BYTECODE_IMM_MASK) >> 14)

#define cs_bytecode_set_opcode(bytecode, opcode) \
  ((bytecode & ~CS_BYTECODE_OPCODE_MASK) | opcode)

#define cs_bytecode_set_a(bytecode, a) \
  ((bytecode & ~CS_BYTECODE_A_MASK) | (a << 6))

#define cs_bytecode_set_c(bytecode, c) \
  ((bytecode & ~CS_BYTECODE_C_MASK) | (c << 14))

#define cs_bytecode_set_b(bytecode, b) \
  ((bytecode & ~CS_BYTECODE_B_MASK) | (b << 23))

#define cs_bytecode_set_imm(bytecode, imm) \
  ((bytecode & ~CS_BYTECODE_IMM_MASK) | (imm << 14))

#define CS_NSTACKS_MAX 250
#define CS_NLOCALS_MAX 200
#define CS_NUPVALS_MAX 60 

typedef enum CsByteConstType {
  CS_CONST_TYPE_NIL,
  CS_CONST_TYPE_TRUE,
  CS_CONST_TYPE_FALSE,
  CS_CONST_TYPE_INT,
  CS_CONST_TYPE_REAL,
  CS_CONST_TYPE_STRING,
} CsByteConstType;

/**
 * a bytecode const
 */
typedef struct CsByteConst {
  CsByteConstType type;
  union {
    struct {
      uint32_t hash;
      size_t size;
      size_t length;
      char* string;
    };
    uint32_t integer;
    double real;
  }; // <3
} CsByteConst;

/**
 * a bytecode function
 */
typedef struct CsByteFunction {
  size_t nparams;
  size_t nupvals;
  size_t nstacks;
  CsArray* funcs;
  CsArray* codes;
  CsArray* consts;
} CsByteFunction;

/**
 * a bytecode chunk
 */
typedef struct CsByteChunk {
  CsByteFunction* entry;
} CsByteChunk;

/**
 * These are helper structs used for debugging.
 * cast CsByteCode into these to have human readable forms
 */
#ifdef CS_DEBUG
typedef struct CsInstruction1 {
  enum CsOpcode opcode: 6;
  unsigned a: 8;
  unsigned c: 9;
  unsigned b: 9;
} CsInstruction1;

typedef struct CsInstruction2 {
  enum CsOpcode opcode: 6;
  unsigned a: 8;
  unsigned imm: 18;
} CsInstruction2;

typedef struct CsInstruction3 {
  enum CsOpcode opcode: 6;
  unsigned a: 8;
  int simm: 18;
} CsInstruction3;

CsInstruction1 CS_INSTRUCTION1;
CsInstruction2 CS_INSTRUCTION2;
CsInstruction3 CS_INSTRUCTION3;
#endif

uint32_t cs_bytecode_make_type1(CsOpcode opcode, int a, int b, int c);
uint32_t cs_bytecode_make_type2(CsOpcode opcode, int a, int imm);
uint32_t cs_bytecode_make_type3(CsOpcode opcode, int a, int simm);

const char* cs_bytechunk_serialize(CsByteChunk* chunk);

CsByteChunk* cs_bytechunk_deserialize(const char* data, size_t size);

void cs_bytechunk_free(CsByteChunk* chunk);

void cs_bytefunction_free(CsByteFunction* func);

#endif /* _CS_BYTECODE_H */
