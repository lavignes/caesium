#ifndef _CS_LEXER_H_
#define _CS_LEXER_H_

#include "cs_common.h"

typedef struct CsLexer {

} CsLexer;

CsLexer* cs_lexer_new();

void cs_lexer_free(CsLexer* lex);

#endif /* _CS_LEXER_H_ */