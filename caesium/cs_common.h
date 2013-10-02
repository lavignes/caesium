#ifndef _CS_COMMON_H_
#define _CS_COMMON_H_

/*
 * Core things
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>

/*
 * Core macros for useful things
 */

/*
 * Call return if a case is true
 */
#define cs_return_if(expression, return_value) \
  if (expression) return return_value
#define cs_return_unless(expression, return_value) \
  if (!(expression)) return return_value

#define cs_assert(expr) assert(expr)

#define cs_alloc_object(type) ((type *) malloc(sizeof(type)))
#define cs_free_object(obj) (free(obj))

#define cs_error(...) fprintf(stderr, __VA_ARGS__)

#ifdef CS_DEBUG
#define cs_debug(...) fprintf(stderr, __VA_ARGS__)
#else
#define cs_debug(...) (void)
#endif

typedef enum CsReason {

  CS_REASON_NONE,
  CS_REASON_NOMEM,
  CS_REASON_BADFILE,
  CS_REASON_THRDFATAL,
  CS_REASON_UNIMPLEMENTED,

} CsReason;

extern const char* CS_REASONS[];

void cs_exit(CsReason reason);

#endif /* _CS_COMMON_H_ */
