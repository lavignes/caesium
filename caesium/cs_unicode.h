#ifndef _CS_UNICODE_H_
#define _CS_UNICODE_H_

#include "cs_common.h"

typedef uint32_t CsUnichar;

/**
 * Compute string length in characters, excluding the null terminator.
 * @param  u8str pointer to utf8 string
 * @param  max   maximum number of bytes to examine. If `max` < 0, then the
 *               string must be null terminated. If `max` > 0, `max` bytes will
 *               be examined.
 * @return       the length of the string in characters
 */
size_t cs_utf8_strnlen(const char* u8str, ssize_t max);

/**
 * Like strncpy but using utf8 characters rather than bytes.
 * @param  dest    buffer to copy `n_chars` characters into
 * @param  src     a utf8 string
 * @param  n_chars number of characters to copy
 * @return         `dest`
 */
char* cs_utf8_strncpy(char* dest, const char* src, size_t n_chars);

/**
 * Find leftmost occurance of character in a utf8 string.
 * @param  u8str a null-terminated utf8 string to search
 * @param  max   maximum length of `u8str`
 * @param  c     unicode character to search for
 * @return       NULL if the character cant be found, otherwise, a pointer to
 *               the character
 */
char* cs_utf8_strchr(const char* u8str, ssize_t max, CsUnichar c);

/**
 * Convert a unicode character to a utf8 character
 * @param  c      a unicode character to convert
 * @param  buffer a buffer to fill with utf8 data, should be at least 6 bytes.
 *                If `buffer` is NULL, then just the number of bytes that would
 *                have been written will be returned.
 * @return        number of bytes written
 */
int cs_unichar_to_utf8(CsUnichar c, char* buffer);

#endif /* _CS_UNICODE_H_ */