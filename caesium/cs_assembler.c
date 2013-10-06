#include "cs_common.h"
#include "cs_assembler.h"
#include "cs_unicode.h"
#include "cs_hash.h"

static CsHash* assembler_pseudo_ops;
static CsHash* assembler_ops;

void setup_assembler() {
  assembler_pseudo_ops = cs_hash_new();
  assembler_ops = cs_hash_new();

  cs_hash_insert(assembler_pseudo_ops, "entry", 5, (void*) CS_PSEUDO_ENTRY);
  cs_hash_insert(assembler_pseudo_ops, "const", 5, (void*) CS_PSEUDO_CONST);
  cs_hash_insert(assembler_pseudo_ops, "end", 3, (void*) CS_PSEUDO_END);

  cs_hash_insert(assembler_ops, "move", 4, (void*) CS_OPCODE_MOVE);
  cs_hash_insert(assembler_ops, "loadk", 5, (void*) CS_OPCODE_LOADK);
  cs_hash_insert(assembler_ops, "loadg", 5, (void*) CS_OPCODE_LOADG);
  cs_hash_insert(assembler_ops, "storg", 5, (void*) CS_OPCODE_STORG);
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
  cs_hash_insert(assembler_ops, "not", 3, (void*) CS_OPCODE_NOT);
  cs_hash_insert(assembler_ops, "shl", 3, (void*) CS_OPCODE_SHL);
  cs_hash_insert(assembler_ops, "shr", 3, (void*) CS_OPCODE_SHR);
  cs_hash_insert(assembler_ops, "jmp", 3, (void*) CS_OPCODE_JMP);
  cs_hash_insert(assembler_ops, "eq", 2, (void*) CS_OPCODE_EQ);
  cs_hash_insert(assembler_ops, "lt", 2, (void*) CS_OPCODE_LT);
  cs_hash_insert(assembler_ops, "le", 2, (void*) CS_OPCODE_LE);
  cs_hash_insert(assembler_ops, "return", 6, (void*) CS_OPCODE_RETURN);
}

void shutdown_assembler() {
  cs_hash_free(assembler_pseudo_ops);
  cs_hash_free(assembler_ops);
}

CsAssembler* cs_assembler_new() {
  CsAssembler* assembler = cs_alloc_object(CsAssembler);
  if (assembler == NULL)
    cs_exit(CS_REASON_NOMEM);
  assembler->filesize = 0;
  assembler->offset = 0;
  assembler->line = 1;
  assembler->col = 1;
  assembler->file = NULL;
  assembler->pos = NULL;
  assembler->statestack = cs_list_new();
  cs_list_push_back(assembler->statestack, (void*) CS_ASM_STATE_INIT);
  return assembler;
}

CsByteChunk* cs_assembler_assemble(CsAssembler* assembler, const char* u8str) {
  long start, end;
  size_t len;
  char* buffer;
  bool new_line = false;
  CsPair* pair;
  CsPseudoOp pop;
  assembler->filesize = cs_utf8_strnlen(u8str, -1);
  assembler->file = u8str;
  assembler->pos = u8str;
  assembler->offset = 0;
  // A simple LR(1) parser
  while (assembler->offset < assembler->filesize) {
    new_line = false;
    CsAsmState state = (uintptr_t) cs_list_peek_back(assembler->statestack);
    // invariables
    switch (*assembler->pos) {

      case '#':
        switch (state) {
          case CS_ASM_STATE_PSEUDO:
            // this should break an op
            goto pseudo_break;
            break;

          case CS_ASM_STATE_OP:
            goto op_break;
            break;

          case CS_ASM_STATE_NUM:
            goto num_break;
            break;
          
          default:
            cs_list_push_back(
              assembler->statestack,
              (void*) CS_ASM_STATE_COMMENT);
            break;
        }
        break;
      
      case '\n':
        new_line = true;
        switch (state) {
          case CS_ASM_STATE_COMMENT:
            // this should break an op
            cs_list_pop_back(assembler->statestack);
            break;
            
          case CS_ASM_STATE_PSEUDO:
            goto pseudo_break;
            break;

          case CS_ASM_STATE_OP:
            goto op_break;
            break;

          case CS_ASM_STATE_NUM:
            goto num_break;
            break;

          default:
            break;
        }
        break;

      case ' ':
      case '\t':
      case '\r':
        switch (state) {
          // break of pseudo token
          case CS_ASM_STATE_PSEUDO:
            pseudo_break: cs_list_pop_back(assembler->statestack);
            end = cs_utf8_pointer_to_offset(assembler->file, assembler->pos);
            buffer = cs_utf8_substr(assembler->file, start+1, end);
            if (buffer == NULL)
              cs_exit(CS_REASON_NOMEM);
            len = strlen(buffer);
            if ((pair = cs_hash_find(assembler_pseudo_ops, buffer, len))) {
              pop = (uintptr_t) pair->value;
              switch (pop) {
                case CS_PSEUDO_ENTRY:
                  cs_list_push_back(
                    assembler->statestack,
                    (void*) CS_ASM_STATE_BLOCK);
                  break;

                case CS_PSEUDO_CONST:
                  cs_list_push_back(
                    assembler->statestack,
                    (void*) CS_ASM_STATE_CONST);
                  break;

                default:
                  break;
              }

            } else {
              cs_error("%zu:%zu: '%s' is not a pseudo operation!\n",
                assembler->line, assembler->col, buffer);
              cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
            }
            cs_free_object(buffer);
            break;

          case CS_ASM_STATE_OP:
            op_break: cs_list_pop_back(assembler->statestack);
            end = cs_utf8_pointer_to_offset(assembler->file, assembler->pos);
            buffer = cs_utf8_substr(assembler->file, start, end);
            if (buffer == NULL)
              cs_exit(CS_REASON_NOMEM);
            len = strlen(buffer);
            if (cs_hash_find(assembler_ops, buffer, len)) {
              cs_debug("matched! %s\n", buffer);
            } else {
              cs_error("%zu:%zu: '%s' is not an instruction!\n",
                assembler->line, assembler->col, buffer);
              cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
            }
            cs_free_object(buffer);
            break;

          case CS_ASM_STATE_NUM:
            num_break: cs_list_pop_back(assembler->statestack);
            cs_list_pop_back(assembler->statestack);
            end = cs_utf8_pointer_to_offset(assembler->file, assembler->pos);
            buffer = cs_utf8_substr(assembler->file, start, end);
            if (buffer == NULL)
              cs_exit(CS_REASON_NOMEM);
            cs_debug("const number!!!: %s\n", buffer);
            cs_free_object(buffer);

          default:
            break;
        }
        break;

      case '\'':
        switch (state) {
          case CS_ASM_STATE_CONST:
            cs_list_push_back(
              assembler->statestack,
              (void*) CS_ASM_STATE_STRING);
            start = cs_utf8_pointer_to_offset(assembler->file, assembler->pos);
            break;

          case CS_ASM_STATE_STRING:
            cs_list_pop_back(assembler->statestack);
            cs_list_pop_back(assembler->statestack);
            end = cs_utf8_pointer_to_offset(assembler->file, assembler->pos);
            buffer = cs_utf8_substr(assembler->file, start+1, end);
            if (buffer == NULL)
              cs_exit(CS_REASON_NOMEM);
            cs_debug("read const!: %s\n", buffer);
            cs_free_object(buffer);
            break;

          default:
            break;
        }

      case '.':
        switch (state) {
          case CS_ASM_STATE_INIT:
          case CS_ASM_STATE_BLOCK:
            cs_list_push_back(
              assembler->statestack,
              (void*) CS_ASM_STATE_PSEUDO);
            start = cs_utf8_pointer_to_offset(assembler->file, assembler->pos);
            break;

          default:
            break;
        }
        break;

      case '0'...'9':
        switch (state) {
          case CS_ASM_STATE_CONST:
            cs_list_push_back(
              assembler->statestack,
              (void*) CS_ASM_STATE_NUM);
            start = cs_utf8_pointer_to_offset(assembler->file, assembler->pos);
            break;

          default:
            break;
        }
        break;

      // text?
      default:
        switch (state) {
          case CS_ASM_STATE_OP:
          case CS_ASM_STATE_PSEUDO:
          case CS_ASM_STATE_COMMENT:
            break;

          // An op?
          case CS_ASM_STATE_BLOCK:
            cs_list_push_back(
              assembler->statestack,
              (void*) CS_ASM_STATE_OP);
            start = cs_utf8_pointer_to_offset(assembler->file, assembler->pos);
            break;

          default:
            break;
        }
        break;
    }

    if (new_line == true) {
      assembler->col = 0;
      assembler->line++;
    }
    assembler->pos = cs_utf8_next(assembler->pos, NULL);
    assembler->offset++;
    assembler->col++;
  }

  return NULL;
}

void cs_assembler_free(CsAssembler* assembler) {
  cs_list_free(assembler->statestack);
  cs_free_object(assembler);
}
