#include "cs_common.h"
#include "cs_assembler.h"
#include "cs_unicode.h"
#include "cs_hash.h"

static CsHash* assembler_pseudo_ops = NULL;
static CsHash* assembler_ops = NULL;

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
  cs_hash_insert(assembler_ops, "loadi", 5, (void*) CS_OPCODE_LOADI);
  cs_hash_insert(assembler_ops, "stori", 5, (void*) CS_OPCODE_STORI);
  cs_hash_insert(assembler_ops, "lodup", 5, (void*) CS_OPCODE_LODUP);
  cs_hash_insert(assembler_ops, "strup", 5, (void*) CS_OPCODE_STRUP);
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
  cs_hash_insert(assembler_ops, "lt", 2, (void*) CS_OPCODE_LT);
  cs_hash_insert(assembler_ops, "le", 2, (void*) CS_OPCODE_LE);
  cs_hash_insert(assembler_ops, "eq", 2, (void*) CS_OPCODE_EQ);
  cs_hash_insert(assembler_ops, "clos", 4, (void*) CS_OPCODE_CLOS);
  cs_hash_insert(assembler_ops, "cpyup", 5, (void*) CS_OPCODE_CPYUP);
  cs_hash_insert(assembler_ops, "movup", 5, (void*) CS_OPCODE_MOVUP);
  cs_hash_insert(assembler_ops, "call", 4, (void*) CS_OPCODE_CALL);
  cs_hash_insert(assembler_ops, "ret", 3, (void*) CS_OPCODE_RET);
}

void shutdown_assembler() {
  cs_hash_free(assembler_pseudo_ops);
  cs_hash_free(assembler_ops);
}

CsAssembler* cs_assembler_new() {
  CsAssembler* assembler = cs_alloc_object(CsAssembler);
  if (assembler == NULL)
    cs_exit(CS_REASON_NOMEM);
  return assembler;
}

CsByteChunk* cs_assembler_assemble(
  CsAssembler* assembler,
  const char* u8str,
  size_t size)
{
  int arg0, arg1, arg2, arg3, arg4;
  double karg;
  long start, end;
  char* buffer;
  bool new_line;
  bool new_comment;
  const char* c = u8str;
  size_t len, line = 1, col = 1;
  CsPair* pair;
  CsPseudoOp pop;
  CsOpcode op;
  CsList* stack = cs_list_new();
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
            if (buffer == NULL)
              cs_exit(CS_REASON_NOMEM);
            switch (state) {
              case CS_ASM_STATE_ENTRY:
                if (sscanf(buffer, "%d %d %d %d %d",
                  &arg0, &arg1, &arg2, &arg3, &arg4) != 5) {
                  cs_error("%zu:%zu: wrong number of operands for .entry\n",
                    line, col);
                  cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                }
                cs_debug("FUNCS: %d\n"
                         "UPVALS: %d\n"
                         "LOCALS: %d\n"
                         "CONSTS: %d\n"
                         "STACKS: %d\n",
                         arg0, arg1, arg2, arg3, arg4);
                break;

              case CS_ASM_STATE_CONSTN:
                sscanf(buffer, "%lf", &karg);
                break;

              case CS_ASM_STATE_CONSTS:
                break;

              case CS_ASM_STATE_MOVE ... CS_ASM_STATE_RET:
                cs_debug("ARGS: %s\n", buffer);
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
            if (buffer == NULL)
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

                case CS_PSEUDO_END:
                  if ((uintptr_t) cs_list_peek_back(stack)
                    != CS_ASM_STATE_BLOCK)
                  {
                    cs_error("%zu:%zu: .end op unexpected...\n", line, col);
                    cs_exit(CS_REASON_ASSEMBLY_MALFORMED);
                  }
                  // pop the block state off the stack
                  cs_list_pop_back(stack);
                  break;

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
            if (buffer == NULL)
              cs_exit(CS_REASON_NOMEM);
            len = strlen(buffer);
            if ((pair = cs_hash_find(assembler_ops, buffer, len))) {
              op = (uintptr_t) pair->value;
              switch (op) {

                // THIS MIGHT WORK!!! :0
                case CS_OPCODE_MOVE ... CS_OPCODE_RET:
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

  cs_list_free(stack);
  return NULL;
}

void cs_assembler_free(CsAssembler* assembler) {
  cs_free_object(assembler);
}
