#include "cs_lexer.h"

CsLexer* cs_lexer_new() {
  CsLexer* lex = cs_alloc_object(CsLexer);
  if (lex == NULL)
    cs_exit(CS_REASON_NOMEM);
  return lex;
}

void cs_lexer_free(CsLexer* lex) {
  cs_free_object(lex);
}