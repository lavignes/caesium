#include <xxhash.h>

#include "cs_common.h"
#include "cs_assembler.h"
#include "cs_unicode.h"
#include "cs_hash.h"
#include "cs_array.h"

static CsHash* assembler_pseudo_ops = NULL;
static CsHash* assembler_ops = NULL;

void setup_assembler() {
  assembler_pseudo_ops = cs_hash_new();
  assembler_ops = cs_hash_new();

  cs_hash_insert(assembler_pseudo_ops, "entry", 5, (void*) CS_PSEUDO_ENTRY);
  cs_hash_insert(assembler_pseudo_ops, "func", 4, (void*) CS_PSEUDO_FUNC);
  cs_hash_insert(assembler_pseudo_ops, "const", 5, (void*) CS_PSEUDO_CONST);
  cs_hash_insert(assembler_pseudo_ops, "end", 3, (void*) CS_PSEUDO_END);
  cs_hash_insert(assembler_pseudo_ops, "resq", 4, (void*) CS_PSEUDO_RESQ);

  cs_hash_insert(assembler_ops, "move", 4, (void*) CS_OPCODE_MOVE);
  cs_hash_insert(assembler_ops, "loadk", 5, (void*) CS_OPCODE_LOADK);
  cs_hash_insert(assembler_ops, "loadg", 5, (void*) CS_OPCODE_LOADG);
  cs_hash_insert(assembler_ops, "storg", 5, (void*) CS_OPCODE_STORG);
  cs_hash_insert(assembler_ops, "loadi", 5, (void*) CS_OPCODE_LOADI);
  cs_hash_insert(assembler_ops, "stori", 5, (void*) CS_OPCODE_STORI);
  cs_hash_insert(assembler_ops, "lodup", 5, (void*) CS_OPCODE_LODUP);
  cs_hash_insert(assembler_ops, "strup", 5, (void*) CS_OPCODE_STRUP);
  cs_hash_insert(assembler_ops, "puts", 4, (void*) CS_OPCODE_PUTS);
  cs_hash_insert(assembler_ops, "new", 3, (void*) CS_OPCODE_NEW);
  cs_hash_insert(assembler_ops, "add", 3, (void*) CS_OPCODE_ADD);
  cs_hash_insert(assembler_ops, "sub", 3, (void*) CS_OPCODE_SUB);
  cs_hash_insert(assembler_ops, "mul", 3, (void*) CS_OPCODE_MUL);
  cs_hash_insert(assembler_ops, "div", 3, (void*) CS_OPCODE_DIV);
  cs_hash_insert(assembler_ops, "mod", 3, (void*) CS_OPCODE_MOD);
  cs_hash_insert(assembler_ops, "pow", 3, (void*) CS_OPCODE_POW);
  cs_hash_insert(assembler_ops, "neg", 3, (void*) CS_OPCODE_NEG);
  cs_hash_insert(assembler_ops, "and", 3, (void*) CS_OPCODE_AND);
  cs_hash_insert(assembler_ops, "or", 2, (void*) CS_OPCODE_OR);
  cs_hash_insert(assembler_ops, "xor", 3, (void*) CS_OPCODE_XOR);
  cs_hash_insert(assembler_ops, "not", 3, (void*) CS_OPCODE_NOT);
  cs_hash_insert(assembler_ops, "shl", 3, (void*) CS_OPCODE_SHL);
  cs_hash_insert(assembler_ops, "shr", 3, (void*) CS_OPCODE_SHR);
  cs_hash_insert(assembler_ops, "jmp", 3, (void*) CS_OPCODE_JMP);
  cs_hash_insert(assembler_ops, "if", 2, (void*) CS_OPCODE_IF);
  cs_hash_insert(assembler_ops, "lt", 2, (void*) CS_OPCODE_LT);
  cs_hash_insert(assembler_ops, "le", 2, (void*) CS_OPCODE_LE);
  cs_hash_insert(assembler_ops, "eq", 2, (void*) CS_OPCODE_EQ);
  cs_hash_insert(assembler_ops, "clos", 4, (void*) CS_OPCODE_CLOS);
  cs_hash_insert(assembler_ops, "cpyup", 5, (void*) CS_OPCODE_CPYUP);
  cs_hash_insert(assembler_ops, "movup", 5, (void*) CS_OPCODE_MOVUP);
  cs_hash_insert(assembler_ops, "call", 4, (void*) CS_OPCODE_CALL);
  cs_hash_insert(assembler_ops, "ret", 3, (void*) CS_OPCODE_RET);
  cs_hash_insert(assembler_ops, "raise", 5, (void*) CS_OPCODE_RAISE);
  cs_hash_insert(assembler_ops, "catch", 5, (void*) CS_OPCODE_CATCH);
  cs_hash_insert(assembler_ops, "spwn", 4, (void*) CS_OPCODE_SPWN);
  cs_hash_insert(assembler_ops, "send", 4, (void*) CS_OPCODE_SEND);
  cs_hash_insert(assembler_ops, "recv", 4, (void*) CS_OPCODE_RECV);
}

void shutdown_assembler() {
  cs_hash_free(assembler_pseudo_ops);
  cs_hash_free(assembler_ops);
}

CsAssembler* cs_assembler_new() {
  CsAssembler* assembler = cs_alloc_object(CsAssembler);
  if (cs_unlikely(assembler == NULL))
    cs_exit(CS_REASON_NOMEM);
  return assembler;
}

CsByteChunk* cs_assembler_assemble(
  CsAssembler* assembler,
  const char* u8str,
  size_t size)
{
  uint32_t instruction;
  int arg0, arg1, arg2;
  double kdarg;
  intptr_t kiarg;
  char ksarg[16];
  long start = 0, end = 0;
  char* buffer;
  bool new_line;
  bool new_comment;
  bool in_resq = false;
  const char* c = u8str;
  size_t len, line = 1, col = 1;
  CsPair* pair;
  CsPseudoOp pop;
  CsOpcode op;
  CsByteFunction *func = NULL, *cur_func;
  CsByteConst* konst;
  CsList* stack = cs_list_new();
  CsList* fstack = cs_list_new(); // stack of in-progress function blocks
  cs_list_push_back(stack, (void*) CS_ASM_STATE_INIT);

  if (size == 0) {
    cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
  }

  do {
    CsAsmState state = (uintptr_t) cs_list_peek_back(stack);
    new_line = false;
    new_comment = false;
    switch (*c) {

      case '#':
        new_comment = true;
        switch (state) {
          // special case: args ending in comments need to push a comment state
          case CS_ASM_STATE_ARGS_READ:
            goto args_read_break;
            break;

          default:
            cs_list_push_back(stack, (void*) CS_ASM_STATE_COMMENT);
            break;
        }
        break;

      case '\n':
        new_line = true;
        switch (state) {
          case CS_ASM_STATE_COMMENT:
            cs_list_pop_back(stack);
            break;

          case CS_ASM_STATE_ARGS_READ:
            args_read_break: cs_list_pop_back(stack);
            consts_break: state = (uintptr_t) cs_list_pop_back(stack);
            end = cs_utf8_pointer_to_offset(u8str, c);
            buffer = cs_utf8_substr(u8str, start, end);
            if (cs_unlikely(buffer == NULL))
              cs_exit(CS_REASON_NOMEM);
            switch (state) {
              case CS_ASM_STATE_ENTRY:
              case CS_ASM_STATE_FUNC:
                if (sscanf(buffer, "%d %d %d", &arg0, &arg1, &arg2) != 3) {
                  cs_error("%zu:%zu: wrong number of operands pseudo op\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                func = cs_alloc_object(CsByteFunction);
                if (cs_unlikely(func == NULL))
                  cs_exit(CS_REASON_NOMEM);
                func->nparams = arg0;
                func->nupvals = arg1;
                func->nstacks = arg2;
                func->funcs = cs_array_new();
                func->codes = cs_array_new();
                func->consts = cs_array_new();
                func->resq = NULL;
                cs_list_push_back(fstack, func);
                break;

              case CS_ASM_STATE_CONSTN:
                konst = cs_alloc_object(CsByteConst);
                if (cs_unlikely(konst == NULL))
                  cs_exit(CS_REASON_NOMEM);
                if (strchr(buffer, '.') != NULL
                    && sscanf(buffer, "%lf", &kdarg) == 1) {
                  konst->type = CS_CONST_TYPE_REAL;
                  konst->real = kdarg;
                } else if (sscanf(buffer, "%"SCNiPTR, &kiarg) == 1) {
                  konst->type = CS_CONST_TYPE_INT;
                  konst->integer = kiarg;
                } else if (sscanf(buffer, "%s", ksarg) == 1) {
                  if (strncmp(ksarg, "nil", 16) == 0) {
                    konst->type = CS_CONST_TYPE_NIL;
                  } else if (strncmp(ksarg, "true", 16) == 0) {
                    konst->type = CS_CONST_TYPE_TRUE;
                  } else if (strncmp(ksarg, "false", 16) == 0) {
                    konst->type = CS_CONST_TYPE_FALSE;
                  } else {
                    cs_error("%zu:%zu: unrecognized constant: '%s'\n",
                      line, col, ksarg);
                    cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                  }
                } else {
                  cs_error("%zu:%zu: unrecognized constant: '%s'\n",
                    line, col, buffer);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                // append const to consts list
                cur_func = cs_list_peek_back(fstack);
                cs_array_insert(cur_func->consts, -1, konst);
                break;

              case CS_ASM_STATE_CONSTS:
                konst = cs_alloc_object(CsByteConst);
                if (cs_unlikely(konst == NULL))
                  cs_exit(CS_REASON_NOMEM);
                konst->type = CS_CONST_TYPE_STRING;
                // intercept the buffer
                konst->string = buffer;
                konst->size = strlen(buffer);
                konst->length = cs_utf8_strnlen(buffer, konst->size);
                konst->hash = XXH32(buffer, konst->size, 0xdeadface);
                buffer = NULL; // this prevents freeing the buffer
                // append const to consts list
                cur_func = cs_list_peek_back(fstack);
                cs_array_insert(cur_func->consts, -1, konst);
                break;

              case CS_ASM_STATE_MOVE:
                if (sscanf(buffer, "%d %d", &arg0, &arg1) != 2) {
                  cs_error("%zu:%zu: wrong number of operands for move\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_MOVE, arg0, arg1, 0);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_LOADK:
                if (sscanf(buffer, "%d %d", &arg0, &arg1) != 2) {
                  cs_error("%zu:%zu: wrong number of operands for loadk\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_LOADK, arg0, arg1, 0);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_LOADG:
                if (sscanf(buffer, "%d %d", &arg0, &arg1) != 2) {
                  cs_error("%zu:%zu: wrong number of operands for loadg\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_LOADG, arg0, arg1, 0);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_STORG:
                if (sscanf(buffer, "%d %d", &arg0, &arg1) != 2) {
                  cs_error("%zu:%zu: wrong number of operands for storg\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_STORG, arg0, arg1, 0);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_LOADI:
                if (sscanf(buffer, "%d %d %d", &arg0, &arg1, &arg2) != 3) {
                  cs_error("%zu:%zu: wrong number of operands for loadi\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_LOADI, arg0, arg1, arg2);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_STORI:
                if (sscanf(buffer, "%d %d %d", &arg0, &arg1, &arg2) != 3) {
                  cs_error("%zu:%zu: wrong number of operands for stori\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_STORI, arg0, arg1, arg2);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_LODUP:
                if (sscanf(buffer, "%d %d", &arg0, &arg1) != 2) {
                  cs_error("%zu:%zu: wrong number of operands for lodup\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_LODUP, arg0, arg1, 0);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_STRUP:
                if (sscanf(buffer, "%d %d", &arg0, &arg1) != 2) {
                  cs_error("%zu:%zu: wrong number of operands for strup\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_STRUP, arg0, arg1, 0);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_PUTS:
                if (sscanf(buffer, "%d", &arg0) != 1) {
                  cs_error("%zu:%zu: wrong number of operands for puts\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_PUTS, arg0, 0, 0);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_NEW:
                if (sscanf(buffer, "%d %d", &arg0, &arg1) != 2) {
                  cs_error("%zu:%zu: wrong number of operands for new\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_NEW, arg0, arg1, 0);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_ADD:
                if (sscanf(buffer, "%d %d %d", &arg0, &arg1, &arg2) != 3) {
                  cs_error("%zu:%zu: wrong number of operands for add\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_ADD, arg0, arg1, arg2);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_SUB:
                if (sscanf(buffer, "%d %d %d", &arg0, &arg1, &arg2) != 3) {
                  cs_error("%zu:%zu: wrong number of operands for sub\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_SUB, arg0, arg1, arg2);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_MUL:
                if (sscanf(buffer, "%d %d %d", &arg0, &arg1, &arg2) != 3) {
                  cs_error("%zu:%zu: wrong number of operands for mul\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_MUL, arg0, arg1, arg2);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_DIV:
                if (sscanf(buffer, "%d %d %d", &arg0, &arg1, &arg2) != 3) {
                  cs_error("%zu:%zu: wrong number of operands for div\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_DIV, arg0, arg1, arg2);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_MOD:
                if (sscanf(buffer, "%d %d %d", &arg0, &arg1, &arg2) != 3) {
                  cs_error("%zu:%zu: wrong number of operands for mod\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_MOD, arg0, arg1, arg2);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_POW:
                if (sscanf(buffer, "%d %d %d", &arg0, &arg1, &arg2) != 3) {
                  cs_error("%zu:%zu: wrong number of operands for pow\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_POW, arg0, arg1, arg2);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_NEG:
                if (sscanf(buffer, "%d %d", &arg0, &arg1) != 2) {
                  cs_error("%zu:%zu: wrong number of operands for neg\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_NEG, arg0, arg1, 0);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_AND:
                if (sscanf(buffer, "%d %d %d", &arg0, &arg1, &arg2) != 3) {
                  cs_error("%zu:%zu: wrong number of operands for and\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_AND, arg0, arg1, arg2);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_OR:
                if (sscanf(buffer, "%d %d %d", &arg0, &arg1, &arg2) != 3) {
                  cs_error("%zu:%zu: wrong number of operands for or\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_OR, arg0, arg1, arg2);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_XOR:
                if (sscanf(buffer, "%d %d %d", &arg0, &arg1, &arg2) != 3) {
                  cs_error("%zu:%zu: wrong number of operands for xor\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_XOR, arg0, arg1, arg2);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_NOT:
                if (sscanf(buffer, "%d %d", &arg0, &arg1) != 2) {
                  cs_error("%zu:%zu: wrong number of operands for and\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_NOT, arg0, arg1, 0);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_SHL:
                if (sscanf(buffer, "%d %d %d", &arg0, &arg1, &arg2) != 3) {
                  cs_error("%zu:%zu: wrong number of operands for shl\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_SHL, arg0, arg1, arg2);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_SHR:
                if (sscanf(buffer, "%d %d %d", &arg0, &arg1, &arg2) != 3) {
                  cs_error("%zu:%zu: wrong number of operands for shr\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_SHR, arg0, arg1, arg2);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_JMP:
                if (sscanf(buffer, "%d", &arg0) != 1) {
                  cs_error("%zu:%zu: wrong number of operands for jmp\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type3(CS_OPCODE_JMP, 0, arg0);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_IF:
                if (sscanf(buffer, "%d %d", &arg0, &arg1) != 2) {
                  cs_error("%zu:%zu: wrong number of operands for if\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type3(CS_OPCODE_IF, arg0, arg1);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_LT:
                if (sscanf(buffer, "%d %d %d", &arg0, &arg1, &arg2) != 3) {
                  cs_error("%zu:%zu: wrong number of operands for lt\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_LT, arg0, arg1, arg2);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_LE:
                if (sscanf(buffer, "%d %d %d", &arg0, &arg1, &arg2) != 3) {
                  cs_error("%zu:%zu: wrong number of operands for le\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_LE, arg0, arg1, arg2);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_EQ:
                if (sscanf(buffer, "%d %d %d", &arg0, &arg1, &arg2) != 3) {
                  cs_error("%zu:%zu: wrong number of operands for eq\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_EQ, arg0, arg1, arg2);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_CLOS:
                if (sscanf(buffer, "%d %d", &arg0, &arg1) != 2) {
                  cs_error("%zu:%zu: wrong number of operands for clos\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type2(CS_OPCODE_CLOS, arg0, arg1);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_CPYUP:
                if (sscanf(buffer, "%d %d %d", &arg0, &arg1, &arg2) != 3) {
                  cs_error("%zu:%zu: wrong number of operands for cpyup\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_CPYUP, arg0, arg1, arg2);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_MOVUP:
                if (sscanf(buffer, "%d %d %d", &arg0, &arg1, &arg2) != 3) {
                  cs_error("%zu:%zu: wrong number of operands for movup\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_MOVUP, arg0, arg1, arg2);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_CALL:
                if (sscanf(buffer, "%d %d %d", &arg0, &arg1, &arg2) != 3) {
                  cs_error("%zu:%zu: wrong number of operands for call\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_CALL, arg0, arg1, arg2);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_RET:
                if (sscanf(buffer, "%d %d", &arg0, &arg1) != 2) {
                  cs_error("%zu:%zu: wrong number of operands for ret\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_RET, arg0, arg1, 0);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_RAISE:
                if (sscanf(buffer, "%d", &arg0) != 1) {
                  cs_error("%zu:%zu: wrong number of operands for raise\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_RAISE, arg0, 0, 0);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_CATCH:
                if (sscanf(buffer, "%d", &arg0) != 1) {
                  cs_error("%zu:%zu: wrong number of operands for catch\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_CATCH, arg0, 0, 0);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_SPWN:
                if (sscanf(buffer, "%d %d", &arg0, &arg1) != 2) {
                  cs_error("%zu:%zu: wrong number of operands for spwn\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type2(CS_OPCODE_SPWN, arg0, arg1);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_SEND:
                if (sscanf(buffer, "%d %d", &arg0, &arg1) != 2) {
                  cs_error("%zu:%zu: wrong number of operands for send\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_SEND, arg0, arg1, 0);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              case CS_ASM_STATE_RECV:
                if (sscanf(buffer, "%d %d", &arg0, &arg1) != 2) {
                  cs_error("%zu:%zu: wrong number of operands for recv\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                instruction =
                  cs_bytecode_make_type1(CS_OPCODE_RECV, arg0, arg1, 0);
                cur_func = cs_list_peek_back(fstack);
                if (in_resq)
                  cs_array_insert(cur_func->resq, -1,
                    (void*) (uintptr_t) instruction);
                else
                  cs_array_insert(cur_func->codes, -1,
                    (void*) (uintptr_t) instruction);
                break;

              default:
                cs_error("%zu:%zu: unexpected argument list...\n", line, col);
                cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                break;
            }
            cs_free_object(buffer);
            if (new_comment == true) {
              cs_list_push_back(stack, (void*) CS_ASM_STATE_COMMENT);
            }
            break;

          case CS_ASM_STATE_PSEUDO:
            goto pseudo_break;
            break;

          case CS_ASM_STATE_OP:
            goto op_break;
            break;

          default:
            break;
        }
        break;

      case ' ':
      case '\t':
      case '\r':
        switch (state) {
          case CS_ASM_STATE_PSEUDO:
            pseudo_break: cs_list_pop_back(stack);
            end = cs_utf8_pointer_to_offset(u8str, c);
            buffer = cs_utf8_substr(u8str, start + 1, end);
            if (cs_unlikely(buffer == NULL))
              cs_exit(CS_REASON_NOMEM);
            len = strlen(buffer);
            if ((pair = cs_hash_find(assembler_pseudo_ops, buffer, len))) {
              pop = (uintptr_t) pair->value;
              switch (pop) {
                case CS_PSEUDO_ENTRY:
                  if ((uintptr_t) cs_list_peek_back(stack)
                    != CS_ASM_STATE_INIT)
                  {
                    cs_error("%zu:%zu: .entry op unexpected...\n", line, col);
                    cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                  }
                  // push next expected states onto stack
                  cs_list_push_back(stack, (void*) CS_ASM_STATE_BLOCK);
                  cs_list_push_back(stack, (void*) CS_ASM_STATE_ENTRY);
                  cs_list_push_back(stack, (void*) CS_ASM_STATE_ARGS);
                  break;

                case CS_PSEUDO_FUNC:
                  cs_list_push_back(stack, (void*) CS_ASM_STATE_BLOCK);
                  cs_list_push_back(stack, (void*) CS_ASM_STATE_FUNC);
                  cs_list_push_back(stack, (void*) CS_ASM_STATE_ARGS);
                  break;

                case CS_PSEUDO_RESQ:
                  cur_func = cs_list_peek_back(fstack);
                  if (cur_func->resq != NULL) {
                    cs_error("%zu:%zu: Blocks resq only once!\n", line, col);
                    cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                  }
                  cur_func->resq = cs_array_new();
                  in_resq = true;
                  break;

                case CS_PSEUDO_END:
                  if ((uintptr_t) cs_list_peek_back(stack)
                    != CS_ASM_STATE_BLOCK)
                  {
                    cs_error("%zu:%zu: .end op unexpected...\n", line, col);
                    cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                  }
                  if (in_resq) {
                    in_resq = false;
                    goto close_resq;
                  }
                  // pop the block state off the stack
                  cs_list_pop_back(stack);
                  // get the func, and append
                  func = cs_list_pop_back(fstack);
                  // unless... this is the entry function
                  if (fstack->length == 0) {
                    // make sure to free the buffer :l
                    cs_free_object(buffer);
                    goto return_entry;
                  }
                  cur_func = cs_list_peek_back(fstack);
                  cs_array_insert(cur_func->funcs, -1, func);
                  close_resq: break;

                case CS_PSEUDO_CONST:
                  if ((uintptr_t) cs_list_peek_back(stack)
                    != CS_ASM_STATE_BLOCK)
                  {
                    cs_error("%zu:%zu: .const op unexpected...\n", line, col);
                    cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                  }
                  // Assume the next arg is a const number
                  cs_list_push_back(stack, (void*) CS_ASM_STATE_CONSTN);
                  cs_list_push_back(stack, (void*) CS_ASM_STATE_ARGS);
                  break;

                default:
                  cs_error("Um... this isn't right. %zu:%zu: '%s' \n",
                    line, col, buffer);
                  cs_exit(CS_REASON_UNIMPLEMENTED);
                  break;
              }
            } else {
              cs_error("%zu:%zu: '%s' is not a pseudo operation!\n",
                line, col, buffer);
              cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
            }
            cs_free_object(buffer);
            break;

          case CS_ASM_STATE_OP:
            op_break: cs_list_pop_back(stack);
            end = cs_utf8_pointer_to_offset(u8str, c);
            buffer = cs_utf8_substr(u8str, start, end);
            if (cs_unlikely(buffer == NULL))
              cs_exit(CS_REASON_NOMEM);
            len = strlen(buffer);
            if ((pair = cs_hash_find(assembler_ops, buffer, len))) {
              op = (uintptr_t) pair->value;
              switch (op) {

                // THIS MIGHT WORK!!! :0
                case CS_OPCODE_MOVE ... CS_OPCODE_RECV:
                  cs_list_push_back(stack, (void*) CS_ASM_STATE_MOVE + op);
                  cs_list_push_back(stack, (void*) CS_ASM_STATE_ARGS);
                  break;

                default:
                  cs_error("Um... this isn't right. %zu:%zu: '%s' \n",
                    line, col, buffer);
                  cs_exit(CS_REASON_UNIMPLEMENTED);
                  break;
              }
            } else {
              cs_error("%zu:%zu: '%s' is not an operation!\n",
                line, col, buffer);
              cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
            }
            cs_free_object(buffer);
            break;

          default:
            break;
        }
        break;

      case '.':
        switch (state) {
          case CS_ASM_STATE_COMMENT:
          case CS_ASM_STATE_CONSTS:
          case CS_ASM_STATE_CONSTN:
          case CS_ASM_STATE_ARGS:
          case CS_ASM_STATE_ARGS_READ:
            break;

          case CS_ASM_STATE_INIT:
          case CS_ASM_STATE_BLOCK:
            cs_list_push_back(stack, (void*) CS_ASM_STATE_PSEUDO);
            start = cs_utf8_pointer_to_offset(u8str, c);
            break;

          default:
            cs_error("%zu:%zu: pseudo op unexpected...\n", line, col);
            cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
            break;
        }
        break;

      case '=':
        switch (state) {
          case CS_ASM_STATE_COMMENT:
            break;
            
          case CS_ASM_STATE_BLOCK:
            cs_list_push_back(stack, (void*) CS_ASM_STATE_LABEL);
            start = cs_utf8_pointer_to_offset(u8str, c);
            break;

          case CS_ASM_STATE_LABEL:
            cs_list_pop_back(stack);
            end = cs_utf8_pointer_to_offset(u8str, c);
            buffer = cs_utf8_substr(u8str, start + 1, end);
            if (cs_unlikely(buffer == NULL))
              cs_exit(CS_REASON_NOMEM);
            cs_debug("label: %s\n", buffer);
            cs_free_object(buffer);
            break;

          default:
            cs_error("%zu:%zu: label unexpected...\n", line, col);
            cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
            break;
        }
        break;

      case '\'':
        switch (state) {
          case CS_ASM_STATE_ARGS_READ:
          case CS_ASM_STATE_ARGS:
          cs_list_pop_back(stack);
          // Remove CONSTN
          if (CS_ASM_STATE_CONSTN != (uintptr_t) cs_list_pop_back(stack)) {
            cs_error("%zu:%zu: unexpected string constant...\n", line, col);
            cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
          }
          cs_list_push_back(stack, (void*) CS_ASM_STATE_CONSTS);
          start = cs_utf8_pointer_to_offset(u8str, c);
          start++;
          break;

          case CS_ASM_STATE_CONSTS:
            goto consts_break;
            break;
        
          default:
            break;
        }
        break;

      default:
        switch (state) {
          case CS_ASM_STATE_ARGS:
            cs_list_pop_back(stack);
            cs_list_push_back(stack, (void*) CS_ASM_STATE_ARGS_READ);
            start = cs_utf8_pointer_to_offset(u8str, c);
            break;

          case CS_ASM_STATE_COMMENT:
          case CS_ASM_STATE_CONSTN:
          case CS_ASM_STATE_CONSTS:
          case CS_ASM_STATE_ARGS_READ:
          case CS_ASM_STATE_OP:
          case CS_ASM_STATE_PSEUDO:
          case CS_ASM_STATE_LABEL:
            break;

          // This should be an operation
          case CS_ASM_STATE_BLOCK:
            cs_list_push_back(stack, (void*) CS_ASM_STATE_OP);
            start = cs_utf8_pointer_to_offset(u8str, c);
            break;

          default:
            cs_error("%zu:%zu: unexpected symbol...\n", line, col);
            cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
            break;
        }
        break;
    }

    if (new_line == true) {
      col = 0;
      line++;
    }
    col++;
  } while ((c = cs_utf8_next(c, u8str + size)));

  // This location is impossible!
  cs_error("Assembly has no entry point!\n");
  cs_error("(Are you missing a newline at the end of file?)\n");
  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);

  return_entry: cs_list_free(stack);
  cs_list_free(fstack);

  CsByteChunk* chunk = cs_alloc_object(CsByteChunk);
  if (cs_unlikely(chunk == NULL))
    cs_exit(CS_REASON_NOMEM);
  chunk->entry = func;
  return chunk;
}

void cs_assembler_free(CsAssembler* assembler) {
  cs_free_object(assembler);
}
