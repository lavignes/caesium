#include "cs_common.h"
#include "cs_assembler.h"
#include "cs_unicode.h"

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
  cs_list_push_back(assembler->statestack, (uintptr_t) CS_ASM_STATE_INIT);
  return assembler;
}

CsByteChunk* cs_assembler_assemble(CsAssembler* assembler, const char* u8str) {
  long start, end;
  char* buffer;
  assembler->filesize = cs_utf8_strnlen(u8str, -1);
  assembler->file = u8str;
  assembler->pos = u8str;
  assembler->offset = 0;
  // A simple LR(1) parser
  while (assembler->offset < assembler->filesize) {
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
          
          default:
            cs_list_push_back(
              assembler->statestack,
              (void*) (uintptr_t) CS_ASM_STATE_COMMENT);
            break;
        }
        break;
      
      case '\n':
        assembler->col = 0;
        assembler->line++;
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
            cs_debug("PSEUDO OP: %s\n", buffer);
            cs_free_object(buffer);

            break;

          case CS_ASM_STATE_OP:
            op_break: cs_list_pop_back(assembler->statestack);
            end = cs_utf8_pointer_to_offset(assembler->file, assembler->pos);
            buffer = cs_utf8_substr(assembler->file, start, end);
            if (buffer == NULL)
              cs_exit(CS_REASON_NOMEM);
            cs_debug("OP!: %s\n", buffer);
            cs_free_object(buffer);

          default:
            break;
        }
        break;

      case '.':
        switch (state) {
          case CS_ASM_STATE_INIT:
            cs_list_push_back(
              assembler->statestack,
              (void*) (uintptr_t) CS_ASM_STATE_PSEUDO);
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
          case CS_ASM_STATE_INIT:
          default:
            cs_list_push_back(
              assembler->statestack,
              (void*) (uintptr_t) CS_ASM_STATE_OP);
            start = cs_utf8_pointer_to_offset(assembler->file, assembler->pos);
            break;
        }
        break;
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
